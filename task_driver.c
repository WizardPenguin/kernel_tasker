#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/mutex.h>

#define DEVICE_NAME "task_driver"
#define CLASS_NAME "task_class"
#define MINOR_NUMBERS 1
#define MINOR_NUMBER_START 0
#define BUFFER_SIZE 1024
struct mutex task_mutex; // Mutex for synchronisation

static int major_number;
static struct class *task_class = NULL;
static struct device *task_device = NULL;
static struct cdev task_cdev;
static char task_buffer[BUFFER_SIZE+1]; // Buffer for read/write operations
static int task_buffer_size = 0; // size of the buffer filled

/***** file operations ****** */
static int task_open(struct inode *inode, struct file *file) {
    pr_info("Task Driver: Device opened\n");
    return 0;
}

static int task_release(struct inode *node, struct file *file) {
    pr_info("Task Driver: Device closed\n");
    return 0;
}

static ssize_t task_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    int max_send_bytes = 0;

    if(!mutex_trylock(&task_mutex)) {
        pr_info("Task Driver: Device is busy, lock not acquired\n");
        mutex_lock(&task_mutex);
    }

    if(*offset >= task_buffer_size) {
        mutex_unlock(&task_mutex);
        return 0; // No more data to read
    }

    max_send_bytes = min((int)len, task_buffer_size - (int)(*offset));
    if(copy_to_user(buffer, task_buffer + *offset, max_send_bytes)) {
        pr_err("Task Driver: Failed to copy data to user space\n");
        mutex_unlock(&task_mutex);
        return -EFAULT;
    }
    *offset += max_send_bytes;
    pr_info("Task Driver: Read %d bytes from buffer\n", max_send_bytes);
    mutex_unlock(&task_mutex);
    return max_send_bytes;
}

// wirte as much as you can, and return how much you wrote
static ssize_t task_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset) {
    int max_receive_bytes = 0;

    if(!mutex_trylock(&task_mutex)) {
        pr_info("Task Driver: Device is busy, lock not acquired\n");
        mutex_lock(&task_mutex); // Wait for the lock to be available
    }

    if(*offset >= BUFFER_SIZE) {
        mutex_unlock(&task_mutex);
        pr_err("Task Driver: Buffer overflow, can't write more data\n");
        return -ENOSPC; // No space left in buffer
    }

    // write as much as you can
    max_receive_bytes = min((int)len, BUFFER_SIZE - (int)(*offset));
    if(copy_from_user(task_buffer + *offset, buffer, max_receive_bytes)) {
        mutex_unlock(&task_mutex);
        pr_err("Task Driver: Failed to copy data from user space\n");
        return -EFAULT;
    }

    *offset += max_receive_bytes;

    task_buffer_size = max(task_buffer_size, (int)(*offset)); // Update buffer size
    pr_info("Task Driver: Written %d bytes to buffer\n", max_receive_bytes);
    mutex_unlock(&task_mutex);
    return max_receive_bytes;
}

static loff_t task_seek(struct file *file, loff_t offset, int whence) { 
    // no mutex as no kernel data corrouption
    loff_t new_offset = 0;

    switch(whence) {
        case SEEK_SET:
            new_offset = offset;
            break;
        case SEEK_CUR:
            new_offset = file->f_pos + offset;
            break;
        case SEEK_END:
            new_offset = BUFFER_SIZE + offset;
            break;
        default:
            return -EINVAL; // invalid whence
    }

    if(new_offset < 0 || new_offset > BUFFER_SIZE) {
        return -EINVAL; // out of bounds
    }

    file->f_pos = new_offset;
    pr_info("Task Driver: Seek to position %lld\n", new_offset);
    return new_offset;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = task_open,
    .release = task_release,
    .read = task_read,
    .write = task_write,
    .llseek = task_seek,
};

/***** module initialization and cleanup ******/
static int task_driver_init(void){
    dev_t dev_no;
    int ret;

    // Allocate a major number for the device
    ret = alloc_chrdev_region(&dev_no, MINOR_NUMBER_START, MINOR_NUMBERS, DEVICE_NAME);
    if(ret < 0) {
        printk(KERN_ALERT "Task Driver : Failed to allocate major number\n");
        return ret;
    }

    major_number = MAJOR(dev_no);
    pr_info("Task Driver : Major number allocated : %d\n", major_number);

    // Create a class for the device
    task_class = class_create(CLASS_NAME);
    if(IS_ERR(task_class)) {
        unregister_chrdev_region(dev_no, MINOR_NUMBERS);
        pr_err("Task Driver : Failed to create class\n");
        return PTR_ERR(task_class);
    }
    pr_info("Task Driver : Class allocated successfully\n");

    // init and add cdev
    cdev_init(&task_cdev, &fops);
    task_cdev.owner = THIS_MODULE;
    ret = cdev_add(&task_cdev, dev_no, MINOR_NUMBERS);
    if(ret < 0) {
        class_destroy(task_class);
        unregister_chrdev_region(dev_no, MINOR_NUMBERS);
        pr_err("Task Driver : failed to add cdev\n");
        return ret;
    }

    // creating device node
    task_device = device_create(task_class, NULL, dev_no, NULL, DEVICE_NAME);
    if(IS_ERR(task_device)) {
        cdev_del(&task_cdev);
        class_destroy(task_class);
        unregister_chrdev_region(dev_no, MINOR_NUMBERS);
        pr_err("Task Driver : Failed to create device\n");
        return PTR_ERR(task_device);
    }

    mutex_init(&task_mutex); // Initialize the mutex

    pr_info("Task Driver : Device created successfully\n");
    return 0;
}

static void task_driver_exit(void){
    dev_t dev_no = MKDEV(major_number, MINOR_NUMBER_START);

    // cleanup
    device_destroy(task_class, dev_no);
    cdev_del(&task_cdev);
    class_destroy(task_class);
    unregister_chrdev_region(dev_no, MINOR_NUMBERS);
    pr_info("Task Driver : Device removed successfully\n");
    return;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raman Sharma");
MODULE_DESCRIPTION("sync_demo: Day 3 - Basic synchronization for read/write with llseek");
MODULE_VERSION("0.3");

module_init(task_driver_init);
module_exit(task_driver_exit);