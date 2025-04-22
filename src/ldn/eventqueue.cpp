#include "eventqueue.hpp"

namespace LDN {
    void EventQueue::push(const NetworkEvent& event) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(event);
        cv.notify_one();
    }

    NetworkEvent EventQueue::pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !queue.empty(); });
        
        auto event = queue.front();
        queue.pop();
        return event;
    }
}