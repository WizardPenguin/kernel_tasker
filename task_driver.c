#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/mutex.h>
#include<linux/list.h>

#define DEVICE_NAME "task_driver"
#define CLASS_NAME "task_class"
#define MINOR_NUMBERS 1
#define MINOR_NUMBER_START 0
#define BUFFER_SIZE 128

static int major_number;
static struct class *task_class = NULL;
static struct device *task_device = NULL;
static struct cdev task_cdev;
static DEFINE_MUTEX(task_mutex);
static LIST_HEAD(task_list);
static int id = 0; // global job id

struct job_data {
    int id;
    char payload[BUFFER_SIZE];
    struct list_head list; // for linked list
};


/***** file operations ****** */
static int task_open(struct inode *inode, struct file *file) {
    pr_info("Task Driver: Device opened\n");
    return 0;
}

static int task_release(struct inode *node, struct file *file) {
    pr_info("Task Driver: Device closed\n");
    return 0;
}
// one time read, pass job
static ssize_t task_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    
    struct job_data *job = NULL;
    int max_send_bytes = 0;

    if(*offset){
        return 0; // EOF, no more data for current job
    }

    max_send_bytes = len;
    mutex_lock(&task_mutex); // protext list

    if(list_empty(&task_list)) {
        mutex_unlock(&task_mutex);
        pr_err("Task Driver : No tasks available to read\n");
        return -ENODATA; // No data available
    }

    job = list_first_entry(&task_list, struct job_data, list);
    list_del(&job->list); // remove it from list
    pr_info("Task Driver: Read job with ID %d\n", job->id);

    if(max_send_bytes > strlen(job->payload) + 1) {
        max_send_bytes = strlen(job->payload) + 1; // +1 for null terminator
        pr_info("Task Driver : sending len : %zu, bytes as buffer small than payload : %zu\n",
            len,strlen(job->payload) + 1);
    }
    printk("sending %d bytes and len : %zu, string : %s",
        max_send_bytes, len, job->payload);

    if(copy_to_user(buffer, job->payload, max_send_bytes)) {
        pr_err("Task Driver: Failed to copy data to user space\n");
        kfree(job); // free job memory
        mutex_unlock(&task_mutex);
        return -EFAULT;
    }

    kfree(job);
    mutex_unlock(&task_mutex);

    pr_info("Task Driver : Send %d bytes to user space\n", max_send_bytes);
    *offset = max_send_bytes;
    return max_send_bytes;
}

// insert a new job, with data given by user
static ssize_t task_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset) {
    int max_receive_bytes = 0;
    struct job_data *new_job = NULL;

    new_job = kmalloc(sizeof(*new_job), GFP_KERNEL);
    if(IS_ERR(new_job)) {
        pr_err("Task Driver: Failed to allocate memory for new job\n");
        return -ENOMEM;
    }

    max_receive_bytes = len;
    if(max_receive_bytes > BUFFER_SIZE-1){
        pr_info("Task Driver: Buffer size exceeded, reducing to %d bytes\n", BUFFER_SIZE);
        max_receive_bytes = BUFFER_SIZE-1;
    }

    mutex_lock(&task_mutex); // Wait for the lock to be available

    if(copy_from_user(new_job->payload, buffer, max_receive_bytes)) {
        mutex_unlock(&task_mutex);
        pr_err("Task Driver: Failed to copy data from user space\n");
        return -EFAULT;
    }

    new_job->payload[max_receive_bytes] = '\0'; // Null-terminate the string so strlen works
    new_job->id = id++;
    list_add_tail(&new_job->list, &task_list); // add to end of list
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

    pr_info("Task Driver : Device created successfully\n");
    return 0;
}

static void task_driver_exit(void){
    dev_t dev_no = MKDEV(major_number, MINOR_NUMBER_START);
    struct job_data *job,*tmp;
    // cleanup
    device_destroy(task_class, dev_no);
    cdev_del(&task_cdev);
    class_destroy(task_class);
    unregister_chrdev_region(dev_no, MINOR_NUMBERS);

    // clearning list
    list_for_each_entry_safe(job, tmp, &task_list, list) {
        list_del(&job->list);
        kfree(job);
    }
    pr_info("Task Driver : Device removed successfully\n");
    return;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raman Sharma");
MODULE_DESCRIPTION("sync_demo: Day 3 - Basic synchronization for read/write with llseek");
MODULE_VERSION("0.3");

module_init(task_driver_init);
module_exit(task_driver_exit);