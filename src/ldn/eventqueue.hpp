#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <variant>
#include "frames.hpp"

namespace LDN {
    struct DisconnectEvent { uint8_t reason; };
    struct JoinEvent { size_t index; ParticipantInfo participant; };
    struct LeaveEvent { size_t index; ParticipantInfo participant; };
    
    using NetworkEvent = std::variant<
        DisconnectEvent,
        JoinEvent,
        LeaveEvent
    >;

    class EventQueue {
        std::queue<NetworkEvent> queue;
        std::mutex mtx;
        std::condition_variable cv;
        
    public:
        void push(const NetworkEvent& event);
        NetworkEvent pop();
        bool empty() const;
    };
}