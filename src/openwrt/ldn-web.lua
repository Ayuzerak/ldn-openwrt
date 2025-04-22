local uci = require("uci").cursor()
local http = require "luci.http"
local sys = require "luci.sys"

module("luci.controller.ldn", package.seeall)

function index()
    entry({"admin", "network", "ldn"}, cbi("ldn"), _("LDN Settings"), 60)
    entry({"admin", "network", "ldn", "status"}, call("action_status"))
end

function action_status()
    local result = {
        networks = sys.exec("ubus call ldn list_networks"),
        status = sys.exec("ubus call ldn get_status")
    }
    http.prepare_content("application/json")
    http.write_json(result)
end