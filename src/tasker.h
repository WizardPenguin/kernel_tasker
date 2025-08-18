#ifndef TASKER_H
#define TASKER_H

#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/mutex.h>
#include<linux/list.h>

// defines
#define DEVICE_NAME "task_driver"
#define CLASS_NAME "task_class"
#define MINOR_NUMBERS 1
#define MINOR_NUMBER_START 0
#define BUFFER_SIZE 128

// custom printk macros
#define task_info(fmt, ...) \
    pr_info("Task Driver: " fmt, ##__VA_ARGS__)

#define task_err(fmt, ...) \
    pr_err("Task Driver: " fmt, ##__VA_ARGS__)


// structures
struct job_data {
    int id;
    int priority;
    char payload[BUFFER_SIZE];
    struct list_head list; // for linked list
};

// global variables

extern struct list_head job_list;
extern struct mutex job_mutex;
extern dev_t dev_no;

// function prototpes

/* tasker_queue.c */
void insert_job_sorted(struct job_data *);
struct job_data* get_next_job(void);
void clear_job_list(void);

/* tasker_cdev.c */
int tasker_cdev_init(struct class *);
void tasker_cdev_cleanup(struct class *);

/* tasker_core.c */
// static int tasker_init(void);
// static void tasker_exit(void);


#endif