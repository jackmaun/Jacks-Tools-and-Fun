-- Usage: nmap --script=ssh-brute <target>

local brute = require "brute"
local creds = require "creds"
local shortport = require "shortport"

author = "Jack Maunsell"
license = "GPL"
categories = {"auth", "intrusive"}

portrule = shortport.port_or_service(22, "ssh")

action = function(host, port)
    local users = {"root", "admin", "user"}
    local passwords = {"password", "123456", "admin", "toor"}

    for _, user in ipairs(users) do
        for _, pass in ipairs(passwords) do
            local status, err = brute.login(host, port, user, pass)
            if status then
                return "Valid credentials found: " .. user .. " / " .. pass
            end
        end
    end
    return "No valid credentials found."
end
