# Process Synchronization Driver - Day 1

## Goal
Set up a base kernel module to ensure:
- Kernel build environment works
- Module loads/unloads correctly
- Makefile automation is ready

## Files Created
- `task_driver.c` — Minimal kernel module with init/exit
- `load_unload_test.c` — Simple user-space program to read recent kernel logs check /dev/task_driver
- `Makefile` — Handles build, install, test, and clean

## Key Includes Used
- `<linux/module.h>` — Required for all kernel modules
- `<linux/init.h>` — Macros for `module_init` and `module_exit`
- `<linux/kernel.h>` — Provides `printk` and log levels like `KERN_INFO`

## Commands
```bash
chmod +x test_script.sh
./test_script.sh
```


## Day 2 Objective
Add read/write functionality to pass data between user space and kernel space.

---

## New Kernel Concepts Learned

- **`copy_to_user()`**: Safely copies data from a kernel buffer to a user-space buffer.
- **`copy_from_user()`**: Safely copies data from a user-space buffer to a kernel buffer.
- **Kernel buffer management**: Used a fixed-size buffer (`BUF_SIZE`) inside the module for data storage.
- **`lseek()` usage in user space**: Allows resetting the file offset for reading previously written data.
- **File offsets**: Tracked by the kernel for each open file descriptor, shared by read/write calls unless manually changed with `lseek()`.

---

## Tools / Includes Used

- `<linux/uaccess.h>` → Provides safe copy functions for kernel-user data transfer.
- `pr_info()` → Kernel logging for debugging.
- `<fcntl.h>`, `<unistd.h>` → Used in user space for file operations (`open`, `read`, `write`, `lseek`).

## New test case added

- read_write_test.c