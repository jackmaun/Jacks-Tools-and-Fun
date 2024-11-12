#Jack Maunsell

This is kernel module that implements a simple rootkit to hide files in directory listings i.e. 'ls'. This module modifies how the sys call getdents64 behaves. getdents64 reads contents of a directory into a buffer then gives the caller a bunch of info about the directory entries.

How it works:

Step 1:
	the function 'find_sys_call_table' finds the system call table in kernel memory. The call table is just an array of function pointers, where each entry corresponds to a system call, i.e. read, write, getdents64

	This function iterates through kernel memory starting from PAGE_OFFSET (where kernel memory starts) and looks for the address of the system call table by looking for a pointer called 'sys_close' (basically a close system call) at index __NR_close. Once it finds this it assumes this is where the system call table is.

Step 2:
	after finding the sys call table, the module saves a reference to the original getdents64 function index at sys_call_table[\__NR_getdents64]. This will allow the original getdents64 to be called even when using the hooked call.

Step 3:
	the sys call table is usually read-only. To hook the system call, the module needs to temporarily disable the write protection by modifying the CPU's CR0 register.
	"write_cr0(read_cr0() & (~0x10000))". write_cr0 and read_cr0 are used to modify the write-protect bit (bit number 16) of the CR0 register. You can clear this bit using some bitwise operations "& ~(0x10000)", this allows the system call table to be modified.

Step 4:
	time to Indiana Jones this sys call. Because there is no write protection now, the module can modify the sys call table entry for getdents64 to point to 'hooked_getdents64'. It does this by just setting sys_call_tabel[\__NR_getdents64] = (unsigned long)* hooked_getdents64. So now, the kernel will invoke hooked_getdents64 because it has no idea whats actually at that function pointer, just that something is.

Step 5:
	once the sys call table is modified you can reflip bit 16 to enable write protection on the CR0 register. 

Step 6 (hooked_getdents64):
	hooked_getdents64 is just a function that calls the original getdents64 to get the directory entries, then iterates over the entries to remove any files that you want. (Will have to match string wise so know the name of the file youd like to hide) If a match is found to your given filename, it will use mmemove to overwrite this entry with other entries, basically removing it from the list but not memory.
	Then returns the list of entries excluding the one you wanted to hide.

(proof of concept)



