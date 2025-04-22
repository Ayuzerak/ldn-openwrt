module("luci.controller.ldn", package.seeall)

local sys = require "luci.sys"
local http = require "luci.http"
local uci = require "luci.model.uci".cursor()

function index()
    entry({"admin", "network", "ldn"}, firstchild(), _("LDN Wireless"), 60).dependent = false
    entry({"admin", "network", "ldn", "settings"}, cbi("ldn/settings"), _("Configuration"), 10)
    entry({"admin", "network", "ldn", "status"}, call("action_status"), _("Status"), 20)
    entry({"admin", "network", "ldn", "join"}, post("action_join"), nil)
    entry({"admin", "network", "ldn", "create"}, post("action_create"), nil)
end

function action_status()
    local result = {
        networks = sys.exec("ubus call ldn list_networks"),
        connections = sys.exec("ubus call ldn get_connections"),
        status = sys.exec("ubus call ldn get_status")
    }
    
    http.prepare_content("application/json")
    http.write_json(result)
end

function action_join()
    local bssid = http.formvalue("bssid")
    os.execute(string.format("ubus call ldn join_network '{ \"bssid\": \"%s\" }'", bssid))
    http.redirect(luci.dispatcher.build_url("admin/network/ldn/status"))
end

function action_create()
    local channel = http.formvalue("channel") or "auto"
    os.execute(string.format("ubus call ldn create_network '{ \"channel\": %s }'", channel))
    http.redirect(luci.dispatcher.build_url("admin/network/ldn/status"))
end

function _get_networks()
    local iw = luci.sys.wifi.getiwinfo("ldn0")
    return iw and iw.networks or {}
end