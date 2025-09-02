#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace TradingEngine {

template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    void push(T item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(item));
        m_cond_var.notify_one();
    }

    void wait_and_pop(T& item) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond_var.wait(lock, [this]{ return !m_queue.empty(); });
        item = std::move(m_queue.front());
        m_queue.pop();
    }

    bool try_pop(T& item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        item = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond_var;
};

}
