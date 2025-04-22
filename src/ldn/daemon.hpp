#pragma once
#include "networkmanager.hpp"
#include "scanner.hpp"
#include "ap.hpp"
#include "station.hpp"
#include "eventqueue.hpp"
#include <unordered_map>
#include <atomic>
#include <memory>
#include <thread>
#include <mutex>

namespace LDN {

class LDNdaemon {
public:
    explicit LDNdaemon();
    ~LDNdaemon();

    // Configuration management
    void load_config(const std::string& config_path = "/etc/config/ldn");
    void reload_config();
    void save_config() const;

    // Daemon control
    void start();
    void stop();
    bool is_running() const { return running_; }

    // Network operations
    void create_network(uint8_t channel);
    void join_network(const MACAddress& host);
    void disconnect();

    // Status monitoring
    struct DaemonStatus {
        bool is_hosting;
        bool is_connected;
        uint8_t current_channel;
        std::vector<ParticipantInfo> participants;
    };
    
    DaemonStatus get_status() const;

private:
    // Core components
    NetworkManager netmgr_;
    RouteManager routemgr_;
    EventQueue events_;
    NetworkScanner scanner_;
    
    // Network instances
    std::unique_ptr<APNetwork> ap_network_;
    std::unique_ptr<StationNetwork> station_network_;
    
    // Configuration
    mutable std::mutex config_mutex_;
    std::unordered_map<std::string, std::string> config_;
    
    // Thread control
    std::atomic<bool> running_{false};
    std::thread worker_thread_;
    std::thread scanner_thread_;
    
    // Internal methods
    void worker_loop();
    void scanner_loop();
    void apply_config();
    void cleanup_networks();
    void setup_ubus();
    void handle_events();

    // Default parameters
    static constexpr uint8_t DEFAULT_CHANNEL = 6;
    static constexpr auto SCAN_INTERVAL = std::chrono::seconds(300);
};

// UBUS integration
struct UBusContext {
    struct ubus_context* ctx;
    struct ubus_object obj;
    
    UBusContext(LDNdaemon& daemon);
    ~UBusContext();
};

} // namespace LDN