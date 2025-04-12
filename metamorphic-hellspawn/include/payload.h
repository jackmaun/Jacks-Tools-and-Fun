#ifndef PAYLOAD_H
#define PAYLOAD_H
 
static unsigned char payload_elf[] = {
    0x00 // no elf shellcode :( its a PoC
};
 
static size_t payload_size = sizeof(payload_elf);
 
#endif // PAYLOAD_H
 