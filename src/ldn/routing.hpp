#pragma once
#include <netlink/route/route.h>
#include <netlink/route/addr.h>
#include <netlink/route/neighbour.h>

namespace LDN {
    class RouteManager {
        struct nl_sock* sock;
        struct rtnl_addr* addr;
        struct nl_cache* link_cache;
        
    public:
        RouteManager();
        ~RouteManager();
        
        void add_address(int ifindex, const std::string& ip, uint8_t prefixlen);
        void add_neighbor(int ifindex, const std::string& ip, const MACAddress& mac);
        void remove_neighbor(int ifindex, const std::string& ip);
    };
}