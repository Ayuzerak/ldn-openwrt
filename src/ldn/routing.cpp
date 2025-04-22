#include "routing.hpp"

namespace LDN {
    RouteManager::RouteManager() {
        sock = nl_socket_alloc();
        nl_connect(sock, NETLINK_ROUTE);
        rtnl_link_alloc_cache(sock, AF_UNSPEC, &link_cache);
    }

    RouteManager::~RouteManager() {
        nl_close(sock);
        nl_socket_free(sock);
        nl_cache_free(link_cache);
    }

    void RouteManager::add_address(int ifindex, const std::string& ip, uint8_t prefixlen) {
        struct rtnl_addr* addr = rtnl_addr_alloc();
        struct nl_addr* local = nl_addr_build(AF_INET, ip.c_str(), ip.size());
        
        rtnl_addr_set_local(addr, local);
        rtnl_addr_set_prefixlen(addr, prefixlen);
        rtnl_addr_set_ifindex(addr, ifindex);
        rtnl_addr_set_scope(addr, RT_SCOPE_UNIVERSE);
        
        rtnl_addr_add(sock, addr, 0);
        nl_addr_put(local);
        rtnl_addr_put(addr);
    }

    void RouteManager::add_neighbor(int ifindex, const std::string& ip, const MACAddress& mac) {
        struct rtnl_neigh* neigh = rtnl_neigh_alloc();
        struct nl_addr* dst = nl_addr_build(AF_INET, ip.c_str(), ip.size());
        
        rtnl_neigh_set_dst(neigh, dst);
        rtnl_neigh_set_lladdr(neigh, mac.encode().data(), 6);
        rtnl_neigh_set_ifindex(neigh, ifindex);
        rtnl_neigh_set_state(neigh, NUD_PERMANENT);
        
        rtnl_neigh_add(sock, neigh, 0);
        nl_addr_put(dst);
        rtnl_neigh_put(neigh);
    }
}