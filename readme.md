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