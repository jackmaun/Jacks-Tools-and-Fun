```
       ___         ___           ___           ___           ___                    ___           ___           ___           ___       ___                    ___           ___           ___                    ___           ___           ___     
      /\  \       /\  \         /\  \         /\__\         /\  \                  /\  \         /\  \         /\  \         /\__\     /\  \                  /\  \         /\__\         /\  \                  /\  \         /\__\         /\__\    
      \:\  \     /::\  \       /::\  \       /:/  /        /::\  \                 \:\  \       /::\  \       /::\  \       /:/  /    /::\  \                /::\  \       /::|  |       /::\  \                /::\  \       /:/  /        /::|  |   
  ___ /::\__\   /:/\:\  \     /:/\:\  \     /:/__/        /:/\ \  \                 \:\  \     /:/\:\  \     /:/\:\  \     /:/  /    /:/\ \  \              /:/\:\  \     /:|:|  |      /:/\:\  \              /:/\:\  \     /:/  /        /:|:|  |   
 /\  /:/\/__/  /::\~\:\  \   /:/  \:\  \   /::\__\____   _\:\~\ \  \                /::\  \   /:/  \:\  \   /:/  \:\  \   /:/  /    _\:\~\ \  \            /::\~\:\  \   /:/|:|  |__   /:/  \:\__\            /::\~\:\  \   /:/  /  ___   /:/|:|  |__ 
 \:\/:/  /    /:/\:\ \:\__\ /:/__/ \:\__\ /:/\:::::\__\ /\ \:\ \ \__\              /:/\:\__\ /:/__/ \:\__\ /:/__/ \:\__\ /:/__/    /\ \:\ \ \__\          /:/\:\ \:\__\ /:/ |:| /\__\ /:/__/ \:|__|          /:/\:\ \:\__\ /:/__/  /\__\ /:/ |:| /\__\
  \::/  /     \/__\:\/:/  / \:\  \  \/__/ \/_|:|~~|~    \:\ \:\ \/__/             /:/  \/__/ \:\  \ /:/  / \:\  \ /:/  / \:\  \    \:\ \:\ \/__/          \/__\:\/:/  / \/__|:|/:/  / \:\  \ /:/  /          \/__\:\ \/__/ \:\  \ /:/  / \/__|:|/:/  /
   \/__/           \::/  /   \:\  \          |:|  |      \:\ \:\__\              /:/  /       \:\  /:/  /   \:\  /:/  /   \:\  \    \:\ \:\__\                 \::/  /      |:/:/  /   \:\  /:/  /                \:\__\    \:\  /:/  /      |:/:/  / 
                   /:/  /     \:\  \         |:|  |       \:\/:/  /              \/__/         \:\/:/  /     \:\/:/  /     \:\  \    \:\/:/  /                 /:/  /       |::/  /     \:\/:/  /                  \/__/     \:\/:/  /       |::/  /  
                  /:/  /       \:\__\        |:|  |        \::/  /                              \::/  /       \::/  /       \:\__\    \::/  /                 /:/  /        /:/  /       \::/__/                              \::/  /        /:/  /   
                  \/__/         \/__/         \|__|         \/__/                                \/__/         \/__/         \/__/     \/__/                  \/__/         \/__/         ~~                                   \/__/         \/__/    
```

## Favorite Projects

- [hook_getdents64](https://github.com/jackmaun/Jacks-Tools-and-Fun/tree/main/hook_getdents64)  
  A Linux kernel module that hooks the `getdents64` syscall to hide files from user space tools like `ls` and `readdir()`. Implements syscall table discovery by scanning kernel memory for a known function pointer match (`sys_close`), then overwrites `__NR_getdents64` to        redirect directory reads to a custom handler. Filters out any file containing the substring `"hidden_file"` directly in kernel space. Features live syscall table patching with CR0 write-protect manipulation. Built to learn more about the Linux kernel and system calls.
  [Full write-up and explination](https://puzzled-den-b1b.notion.site/Hooking-getdents64-in-the-Linux-Kernel-14924f1021d780a482e6fd23520ba411?pvs=74)


- [metamorphic-hellspawn](https://github.com/jackmaun/Jacks-Tools-and-Fun/tree/main/metamorphic-hellspawn)  
  A full-blown metamorphic malware PoC written in C. Features polymorphic encryption routines with inline assembly, self-replication, SSH-based lateral movement, anti-debugging, in-memory payload execution, basic credential harvesting, and C2 exfil via XOR’d HTTP. This        binary mutates itself on each run, rewrites its own payload, and infects everything in its path. Built to learn.
 *Write-up in progress*

- [irc](https://github.com/jackmaun/Jacks-Tools-and-Fun/tree/main/irc)  
  Custom IRC server and client stack built in C, RFC 1459-compliant with OpenSSL/TLS support. Built natively for Windows, including a `poll()` replacement and low-level socket handling with WinSock2. Handles user connections, message parsing, nickname         management, and secure communication over port 6697. Features custom certificate generation, and raw SSL context config. Built for fun, reverse engineering practice, and protocol learning.


