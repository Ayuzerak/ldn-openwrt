local m, s, o

m = Map("ldn", "LDN Settings",
    [[Nintendo Local Direct Network configuration.<br/>
    Configure wireless ad-hoc gaming networks.]])

s = m:section(TypedSection, "network", "Network Profiles")
s.addremove = true
s.anonymous = false

o = s:option(Flag, "enabled", "Enable Profile")
o.rmempty = false

o = s:option(Value, "comm_id", "Communication ID")
o.datatype = "hex(16)"

o = s:option(Value, "game_mode", "Game Mode")
o.datatype = "uinteger"

o = s:option(Value, "max_players", "Max Players")
o.datatype = "range(1,8)"

o = s:option(Value, "password", "Network Password")
o.password = true

return m