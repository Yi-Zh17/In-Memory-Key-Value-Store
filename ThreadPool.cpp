#include <unistd.h>

#include "ThreadPool.h"
#include "Logger.h"


void ThreadPool::thread_tasks() {
    while (true) {
        // Only wake up if stopping or there is a task
        std::unique_lock<std::mutex> lock(queue_mutex);
        condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });

        // Exit
        if (this->stop && this->tasks.empty()) return;

        // Do tasks
        Task t = std::move(this->tasks.front()); // Get a task
        this->tasks.pop(); // Remove the task
        lock.unlock(); // Unlock the queue

        // Check tokens
        if (t.command.empty()) {
            Logger::log(ERROR, "Empty message");
            close(t.client_fd);
            continue; // Jump back to top
        }

        // Perform actions
        if (t.command[0] == "SET" && t.command.size() == 3) { // SET
            if (!table->insert(t.command[1], t.command[2])) {
                Logger::log(WARNING, "Data not inserted");
                char msg[] = "Insertion error\n";
                write(t.client_fd, msg, std::string_view(msg).size());
            }
            else {
                Logger::log(INFO, "Inserted 1 entry");
                write(t.client_fd, "OK\n", 3);
            }
        }
        else if (t.command[0] == "GET" && t.command.size() == 2) { // GET
            auto result = table->get(t.command[1]);
            if (result.has_value()) {
                Logger::log(INFO, "Got 1 entry");
                write(t.client_fd, result->data(), result->size());
                write(t.client_fd, "\n", 1);
            }
            else {
                Logger::log(WARNING, "Get query failed");
                char msg[] = "Not found\n";
                write(t.client_fd, msg, std::string_view(msg).size());
            }
        }
        else if (t.command[0] == "DEL" && t.command.size() == 2) { // DEL
            if (!table->remove(t.command[1])) {
                Logger::log(ERROR, "Entry not deleted");
                char msg[] = "Deletion error\n";
                write(t.client_fd, msg, std::string_view(msg).size());
            }
            else {
                Logger::log(INFO, "Deleted 1 entry");
                write(t.client_fd, "OK\n", 3);
            }
        }
        else {
            Logger::log(ERROR, "Invalid request received");
            char msg[] = "Invalid request\n";
            write(t.client_fd, msg, std::string_view(msg).size());
            close(t.client_fd);
            continue;
        }
    }
}

ThreadPool::ThreadPool(HashTable* table) {
    this->table = table;

    // Maximum threads to use
    unsigned int num_threads = std::thread::hardware_concurrency();

    for (int i = 0; i < num_threads; i++) {
        workers.emplace_back(&ThreadPool::thread_tasks, this);
    }
}

ThreadPool::~ThreadPool() {
    queue_mutex.lock();
    this->stop = true;
    queue_mutex.unlock();

    condition.notify_all();

    for (int i = 0; i < workers.size(); i++) {
        workers[i].join();
    }
}

void ThreadPool::enqueue(Task t) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks.push(std::move(t)); // Avoid copying the strings inside tasks
    condition.notify_one();
}
