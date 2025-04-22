#include "daemon.hpp"
#include <uci.h>
#include <chrono>
#include <fstream>
#include <sstream>
#include <libubus.h>
#include <libubox/uloop.h>

using namespace std::chrono_literals;

namespace LDN {

// UCI Configuration parser
void LDNdaemon::load_config(const std::string& path) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    struct uci_context *ctx = uci_alloc_context();
    if (!ctx) throw std::runtime_error("UCI context allocation failed");

    struct uci_package *pkg = nullptr;
    if (uci_load(ctx, "ldn", &pkg) != UCI_OK) {
        uci_free_context(ctx);
        throw std::runtime_error("Failed to load UCI config");
    }

    uci_foreach_element(&pkg->sections, section) {
        struct uci_section *s = uci_to_section(section);
        
        if (strcmp(s->type, "network") == 0) {
            uci_foreach_option(&s->options, option) {
                config_[option->e.name] = option->v.string;
            }
        }
    }

    uci_unload(ctx, pkg);
    uci_free_context(ctx);
}

void LDNdaemon::reload_config() {
    load_config();
    apply_config();
}

void LDNdaemon::apply_config() {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    if (config_.count("auto_mode")) {
        auto mode = config_.at("auto_mode");
        if (mode != current_mode_) {
            cleanup_networks();
            current_mode_ = mode;
        }
    }
}

void LDNdaemon::start() {
    if (running_) return;

    running_ = true;
    worker_thread_ = std::thread([this] { worker_loop(); });
    scanner_thread_ = std::thread([this] { scanner_loop(); });
    
    setup_ubus();
}

void LDNdaemon::stop() {
    running_ = false;
    
    if (worker_thread_.joinable()) worker_thread_.join();
    if (scanner_thread_.joinable()) scanner_thread_.join();
    
    cleanup_networks();
    uloop_done();
}

void LDNdaemon::cleanup_networks() {
    ap_network_.reset();
    station_network_.reset();
}

void LDNdaemon::worker_loop() {
    while (running_) {
        try {
            if (!ap_network_ && !station_network_) {
                if (config_["auto_mode"] == "ap" || 
                   (config_["auto_mode"] == "hybrid" && !station_network_)) {
                    create_network(std::stoi(config_["default_channel"]));
                }
            }

            handle_events();
            std::this_thread::sleep_for(100ms);
        } catch (const std::exception& e) {
            // Log error
        }
    }
}

void LDNdaemon::scanner_loop() {
    const auto interval = config_.count("scan_interval") 
        ? std::stoi(config_["scan_interval"]) : 300;
        
    while (running_) {
        try {
            auto networks = scanner_.scan();
            
            if (config_["auto_mode"] == "hybrid") {
                bool found = false;
                for (const auto& net : networks) {
                    if (net.session.localCommunicationId == 
                        std::stoull(config_["comm_id"])) {
                        join_network(net.address);
                        found = true;
                        break;
                    }
                }
                if (!found && !ap_network_) {
                    create_network(std::stoi(config_["default_channel"]));
                }
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(interval));
        } catch (...) {
            // Handle scan errors
        }
    }
}

void LDNdaemon::create_network(uint8_t channel) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    APNetwork::CreateParams params;
    params.comm_id = std::stoull(config_["comm_id"]);
    params.game_mode = std::stoi(config_["game_mode"]);
    params.max_players = std::stoi(config_["max_players"]);
    params.password = config_["password"];
    
    // Hex string to bytes
    std::string app_data = config_["app_data"];
    params.app_data.reserve(app_data.size()/2);
    for (size_t i = 0; i < app_data.size(); i += 2) {
        params.app_data.push_back(
            static_cast<uint8_t>(std::stoi(app_data.substr(i, 2), nullptr, 16))
        );
    }

    ap_network_ = std::make_unique<APNetwork>(netmgr_, routemgr_, events_);
    ap_network_->create(params);
    ap_network_->set_channel(channel);
}

void LDNdaemon::join_network(const MACAddress& host) {
    auto networks = scanner_.scan();
    for (const auto& net : networks) {
        if (net.address == host) {
            station_network_ = std::make_unique<StationNetwork>(netmgr_, routemgr_, events_);
            
            StationNetwork::ConnectParams params;
            params.network = net;
            params.password = config_["password"];
            params.node_name = config_["node_name"];
            params.app_version = std::stoi(config_["app_version"]);
            
            station_network_->connect(params);
            return;
        }
    }
    throw std::runtime_error("Network not found");
}

LDNdaemon::DaemonStatus LDNdaemon::get_status() const {
    DaemonStatus status;
    
    if (ap_network_) {
        status.is_hosting = true;
        status.current_channel = ap_network_->get_channel();
        status.participants = ap_network_->get_participants();
    } else if (station_network_) {
        status.is_connected = true;
        status.current_channel = station_network_->get_channel();
        status.participants = station_network_->get_participants();
    }
    
    return status;
}

// UBUS integration
enum {
    CREATE_NETWORK,
    JOIN_NETWORK,
    GET_STATUS,
    RELOAD_CONFIG,
    __METHOD_MAX
};

static const struct blobmsg_policy ldn_policy[__METHOD_MAX] = {
    [CREATE_NETWORK] = { .name = "channel", .type = BLOBMSG_TYPE_INT32 },
    [JOIN_NETWORK] = { .name = "bssid", .type = BLOBMSG_TYPE_STRING },
    [GET_STATUS] = { .name = "verbose", .type = BLOBMSG_TYPE_BOOL },
    [RELOAD_CONFIG] = {},
};

static int ubus_ldn_handler(struct ubus_context *ctx, struct ubus_object *obj,
                           struct ubus_request_data *req, const char *method,
                           struct blob_attr *msg) {
    LDNdaemon* daemon = static_cast<LDNdaemon*>(container_of(obj, UBusContext, obj)->daemon);
    
    struct blob_attr *tb[__METHOD_MAX];
    blobmsg_parse(ldn_policy, __METHOD_MAX, tb, blob_data(msg), blob_len(msg));

    if (!strcmp(method, "create_network")) {
        if (!tb[CREATE_NETWORK]) return UBUS_STATUS_INVALID_ARGUMENT;
        uint8_t channel = blobmsg_get_u32(tb[CREATE_NETWORK]);
        daemon->create_network(channel);
        return UBUS_STATUS_OK;
    }
    
    // Other method handlers...
    
    return UBUS_STATUS_METHOD_NOT_FOUND;
}

void LDNdaemon::setup_ubus() {
    static struct ubus_object_type type = 
        UBUS_OBJECT_TYPE("ldn-daemon", ldn_policy);
    
    static struct ubus_object obj = {
        .name = "ldn",
        .type = &type,
        .methods = ubus_ldn_handler,
        .n_methods = __METHOD_MAX,
    };

    struct ubus_context *ctx = ubus_connect(nullptr);
    if (!ctx) throw std::runtime_error("Failed to connect to ubus");

    ubus_add_object(ctx, &obj);
    uloop_run();
}

} // namespace LDN