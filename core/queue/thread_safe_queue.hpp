#ifndef CORE_QUEUE_THREAD_SAFE_QUEUE_HPP
#define CORE_QUEUE_THREAD_SAFE_QUEUE_HPP

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

template <typename T>
class ThreadSafeQueue {
public:
    void push(T item) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push(std::move(item));
        }
        cv_.notify_one();
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });
        if (stopped_ && queue_.empty())
            return false;
        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool try_pop(T& item, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (!cv_.wait_for(lock, timeout, [this] { return !queue_.empty() || stopped_; }))
            return false;
        if (stopped_ && queue_.empty())
            return false;
        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    size_t pop_batch(std::vector<T>& out, size_t max_items) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });
        if (stopped_ && queue_.empty())
            return 0;

        size_t count = 0;
        while (!queue_.empty() && count < max_items) {
            out.push_back(std::move(queue_.front()));
            queue_.pop();
            ++count;
        }
        return count;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }

    size_t drain_to(std::vector<T>& out) {
        std::lock_guard<std::mutex> lock(mtx_);
        size_t count = queue_.size();
        while (!queue_.empty()) {
            out.push_back(std::move(queue_.front()));
            queue_.pop();
        }
        return count;
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stopped_ = true;
        }
        cv_.notify_all();
    }

    bool stopped() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return stopped_;
    }

private:
    std::queue<T>            queue_;
    mutable std::mutex       mtx_;
    std::condition_variable  cv_;
    bool                     stopped_ = false;
};

#endif
