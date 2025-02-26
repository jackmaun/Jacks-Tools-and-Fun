-- Usage: nmap --script=custom-service-detect <target>

local shortport = require "shortport"
local stdnse = require "stdnse"

author = "Jack Maunsell"
license = "GPL"
categories = {"discovery", "safe"}

portrule = shortport.port_or_service({80, 443, 8080, 3306}, {"http", "https", "mysql"})

action = function(host, port)
    local socket = nmap.new_socket()
    socket:set_timeout(5000)
    
    local status, err = socket:connect(host.ip, port.number)
    if not status then
        return "Failed to connect: " .. err
    end

    socket:send("HELLO\r\n")
    local response = socket:receive()
    
    if response then
        return "Service detected: " .. response
    else
        return "No response from service"
    end
end
