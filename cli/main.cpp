#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <queue>
#include <thread>

#include "core/events/generator.hpp"
#include "core/store/feature_store.hpp"

struct EventQueue {
    std::queue<Event>             queue;
    std::mutex                    mtx;
    std::condition_variable       cv;
};

int main() {
    constexpr uint64_t NUM_USERS      = 50;
    constexpr uint64_t INTERVAL_MS    = 80;
    constexpr int      PRINT_INTERVAL = 3;
    constexpr int      RUN_DURATION   = 15;

    EventGenerator gen(42);
    gen.set_user_count(NUM_USERS);
    gen.set_event_interval_ms(INTERVAL_MS);

    FeatureStore store;
    EventQueue   eq;
    std::atomic<bool> done{false};

    // Producer: generate events
    std::thread producer([&] {
        while (!done) {
            Event ev = gen.next_event();
            {
                std::lock_guard<std::mutex> lock(eq.mtx);
                eq.queue.push(ev);
            }
            eq.cv.notify_one();
            std::this_thread::sleep_for(
                std::chrono::milliseconds(INTERVAL_MS));
        }
    });

    // Consumer: ingest events
    std::thread consumer([&] {
        while (!done) {
            std::unique_lock<std::mutex> lock(eq.mtx);
            eq.cv.wait_for(lock, std::chrono::milliseconds(50), [&] {
                return !eq.queue.empty() || done;
            });
            while (!eq.queue.empty()) {
                Event ev = eq.queue.front();
                eq.queue.pop();
                lock.unlock();
                store.ingest(ev);
                lock.lock();
            }
        }
    });

    // Periodic print
    for (int elapsed = 0; elapsed < RUN_DURATION; elapsed += PRINT_INTERVAL) {
        std::this_thread::sleep_for(std::chrono::seconds(PRINT_INTERVAL));
        std::cout << "\n--- Feature State [t=" << elapsed + PRINT_INTERVAL
                  << "s] ---\n";
        store.print_all();
    }

    done = true;
    eq.cv.notify_all();
    producer.join();
    consumer.join();

    std::cout << "\n--- Final Feature State ---\n";
    store.print_all();

    return 0;
}
