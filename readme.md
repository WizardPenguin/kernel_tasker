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


# Day 2 Objective
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


# Day 3 Objective
Add **basic synchronization** to make read/write safe under concurrent access and implement `.llseek` support for flexible file offsets.

---

## Changes made
- Added a `mutex` to the kernel module to protect the shared kernel buffer and size.
- Ensured `read()` and `write()` acquire/release the mutex to avoid race conditions.
- Implemented `.llseek` using `fixed_size_llseek()` to allow user-space `lseek()` behavior bounded by `BUF_SIZE`.

---

## New Kernel Concepts Learned
- **`struct mutex` / `mutex_lock` / `mutex_unlock`**: Kernel primitive for mutual exclusion in sleeping context (suitable for user-blocking code sections).
- **Atomic buffer updates**: Ensuring reads and writes don't interleave and leave buffer in inconsistent state.
- **`fixed_size_llseek()`**: Helper to implement `.llseek` with bounds checking against buffer length.
- **Best practice**: Always protect any shared kernel data (buffers, counters) that can be accessed concurrently from multiple processes/threads.

---

## Tools / Includes Used
- `<linux/mutex.h>` → Provides `struct mutex` and related API.
- `<linux/uaccess.h>` → `copy_to_user()` / `copy_from_user()`.
- `<linux/fs.h>` → `fixed_size_llseek()` and `file_operations`.
- `pr_info()` → Kernel logging to observe lock/wait info and operations.

---

## Test
- `read_write_threaded_test.c` spawns multiple threads, each performing write + lseek + read to `/dev/sync_demo`.
- The test checks that the driver returns consistent data (i.e., what was last written).
- Run the test after inserting the module. Example:

# Day 4 – Custom Kernel Data Structure (Linked List Job Queue)

## Objective:
Implement a linked list in the kernel to store multiple jobs in a queue.

## New Kernel Concepts Learned:
- `struct list_head` and Linux list API (`list_add_tail`, `list_del`, `list_first_entry`, `list_empty`).
- Dynamic memory allocation in kernel space with `kmalloc`/`kfree`.
- Protecting linked list operations with `mutex`.

## Driver Changes:
- Replaced single buffer with a `struct job` queue.
- Each job stores: ID, priority (future), and payload.
- `.write` enqueues new jobs, `.read` dequeues jobs.

## Tools/Includes Used:
- `<linux/list.h>` → Kernel linked list API.
- `<linux/slab.h>` → Memory allocation in kernel.
- `<linux/mutex.h>` → Protect linked list access.


# Day 5 – Sorted Priority Queue
-----------------------------
## Objective:
Implement a job queue with sorted insertion to ensure O(1) retrieval of highest-priority job.

## Concepts Learned:
- list_for_each_entry() for traversal
- Insertion before a given element in kernel list
- Trade-off: O(n) insert vs O(1) read

## Includes:
<linux/list.h> – Doubly linked list API
<linux/slab.h> – kmalloc/kfree

## Test:
- Insert jobs in random priority order
- Ensure read order matches descending priority

[TODO] implement priority queue by ourselves, either using tree or fix size array
or resize array when size reached.

# Time for modularisation

kernel_tasker/
 ├── src/                 # kernel module sources
 │    ├── tasker_core.c
 │    ├── tasker_cdev.c
 │    ├── tasker_queue.c
 │    └── tasker.h
 │
 ├── tests/               # all user-space test programs
 │    ├── load_unload_test.c
 │    ├── read_write_test.c
 │    ├── read_write_threaded.c
 │    ├── test_runner.c   # NEW: main program that calls all tests
 │    └── Makefile        # builds test_runner + object files
 │
 ├── Makefile             # builds kernel module
 └── run_tests.sh         # script to build + load module + run tests

TODO: add wait queue refer notes

