# Stage 1: builder
FROM gcc:latest AS builder
WORKDIR /app

COPY . ./
RUN make server

# Stage 2: runtime
FROM debian:trixie-slim
WORKDIR /app

COPY --from=builder /app/server .

EXPOSE 8080
CMD [ "./server" ]