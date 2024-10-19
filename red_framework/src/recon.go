package recon

import (
	"fmt"
	"net"
	"time"
)

func obfuscateIP(ip string) string {
	key := byte(0xAB)
	obfuscated := []byte(ip)
	for i := range obfuscated {
		obfuscated[i] ^= key
	}
	return string(obfuscated)
}

func deobfuscateIP(ip string) string {
	return obfuscateIP(ip)
}

func scanNetwork(targets []string) []string {
	var openIPs []string
	for _, target := range targets {
		deobfuscatedIP := deobfuscateIP(target)
		conn, err := net.DialTimeout("tcp", deobfuscatedIP+":22", 2*time.Second)
		if err == nil {
			fmt.Printf("[+] Found open SSH port on: %s\n", deobfuscatedIP)
			openIPs = append(openIPs, deobfuscatedIP)
			conn.Close()
		} else {
			fmt.Printf("[-] No open SSH on: %s\n", deobfuscatedIP)
		}
	}
	return openIPs
}

func main() {

	obfuscatedIPs := []string{
		obfuscateIP("192.168.1.100"), //example
		obfuscateIP("192.168.1.101"), //example
	}
	openSSH := scanNetwork(obfuscatedIPs)
	fmt.Println("Open SSH IPs:", openSSH)
}
