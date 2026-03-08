#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <vector>
#include <string_view>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "HashTable.h"


struct Task
{
    int client_fd;
    std::vector<std::string> command;
};

class ThreadPool {
private:
    std::queue<Task> tasks; // All the tasks
    std::mutex queue_mutex; // Lock
    std::condition_variable condition; // Condition variable to wake a thread up or put it to sleep
    std::vector<std::thread> workers; // Worker threads
    HashTable* table;
    bool stop = false; // Shutdown pool when server exits

    /**
     * Common tasks performed by a thread. Pops tasks from the queue and executes them.
     */
    void thread_tasks();

public: 
    ThreadPool(HashTable* table);
    ~ThreadPool();

    /**
     * Push a client request to queue.
     */
    void enqueue(Task t);

};

#endif