#include"tasker.h"

static dev_t dev;
static struct class *cls = NULL;
static struct device *task_device = NULL;


static int tasker_init(void){
    int ret;

    // Allocate device number
    ret = alloc_chrdev_region(&dev_no, MINOR_NUMBER_START, MINOR_NUMBERS, DEVICE_NAME);
    if(ret < 0) {
        task_err("Failed to allocate major number\n");
        return ret;
    }
    task_info("Major number allocated : %d\n", MAJOR(dev_no));

    // creating class
    cls = class_create(CLASS_NAME);
    if(IS_ERR(cls)) {
        unregister_chrdev_region(dev_no, MINOR_NUMBERS);
        task_err("Failed to create class\n");
        return PTR_ERR(cls);
    }
    task_info("Class allocated successfully\n");

    // init and add cdev
    tasker_cdev_init(cls);
    if(ret < 0) {
        class_destroy(cls);
        unregister_chrdev_region(dev_no, MINOR_NUMBERS);
        task_err("failed to add cdev\n");
    }

    // creating device node
    task_device = device_create(cls, NULL, dev_no, NULL, DEVICE_NAME);
    if(IS_ERR(task_device)) {
        tasker_cdev_cleanup(cls);
        class_destroy(cls);
        unregister_chrdev_region(dev_no, MINOR_NUMBERS);
        task_err("Failed to create device\n");
        return PTR_ERR(task_device);
    }
    task_info("Device created successfully\n");
    return 0;
}

static void tasker_exit(void){
    // cleanup
    device_destroy(task_class, dev_no);
    tasker_cdev_cleanup(cls);
    class_destroy(task_class);
    unregister_chrdev_region(dev_no, MINOR_NUMBERS);

    // clearning list
    clear_job_list();
    task_info("Device removed successfully\n");
    return;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raman Sharma");
MODULE_DESCRIPTION("sync_demo: Day 5 - Task Driver with Sorted Priority Queue, restructuring");
MODULE_VERSION("0.5");

module_init(tasker_init);
module_exit(tasker_exit);