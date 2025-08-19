#include"tasker.h"

static struct cdev cdev;

static int tasker_open(struct inode*, struct file*){
    task_info("Device opened\n");
    return 0;
}

static int tasker_release(struct inode*, struct file*){
    task_info("Device closed\n");
    return 0;
}

static ssize_t tasker_read(struct file *file, char __user *buffer, size_t len, loff_t *offset){
    struct job_data *job = NULL;
    char buf[BUFFER_SIZE];
    int max_send_bytes = len;

    if(*offset){
        return 0; // EOF, no more data for current job
    }

    job = get_next_job();
    if(!job) {
        task_err("No tasks available to read\n");
        return -ENODATA;
    }
    task_info("Read job with ID %d\n", job->id);
    // creating payload
    if(job->id < 0) {
        // return payload as it iss
        task_warn("Job ID is negative, using default ID\n");
        strscpy(buf, job->payload, BUFFER_SIZE);
    }
    else {
        scnprintf(buf, BUFFER_SIZE, "Job ID=%d Priority=%d Payload=%s\n",
            job->id, job->priority, job->payload);
    }

    if(max_send_bytes > strlen(buf) + 1) {
        max_send_bytes = strlen(buf) + 1; // +1 for null terminator
        pr_info("Task Driver : sending len : %zu, bytes as buffer small than payload : %zu\n",
            len,strlen(buf) + 1);
    }

    pr_info("Task Driver : sending %d bytes and len : %zu, string : %s",
        max_send_bytes, len, buf);

    if(copy_to_user(buffer, buf, max_send_bytes)) {
        task_err("Failed to copy data to user space\n");
        kfree(job); // free job memory
        return -EFAULT;
    }

    kfree(job);
    pr_info("Task Driver : Send %d bytes to user space\n", max_send_bytes);
    *offset = max_send_bytes;
    return max_send_bytes;
}

static ssize_t tasker_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset) {
    int max_receive_bytes = 0;
    int n = 0,ret = 0;
    char buf[BUFFER_SIZE];
    struct job_data *new_job = NULL;

    max_receive_bytes = len;
    if(max_receive_bytes > BUFFER_SIZE-1){
        task_info("Buffer size exceeded, reducing to %d bytes\n", BUFFER_SIZE);
        max_receive_bytes = BUFFER_SIZE-1;
    }

    new_job = kmalloc(sizeof(*new_job), GFP_KERNEL);
    if(IS_ERR(new_job)) {
        task_err("Failed to allocate memory for new job\n");
        return -ENOMEM;
    }

    if(copy_from_user(buf, buffer, max_receive_bytes)) {
        task_err("Failed to copy data from user space\n");
        return -EFAULT;
    }
    buf[max_receive_bytes] = '\0'; // Null-terminate the string so strlen works
    ret = sscanf(buf, "%d %d %n", &new_job->id, &new_job->priority, &n);
    if(ret < 3){
        // paste remaining into payload
        strscpy(new_job->payload, buf + n, BUFFER_SIZE);
    }
    else {
        // everything is payloead
        strscpy(new_job->payload, buf, BUFFER_SIZE);
        new_job->id = -1; // default id if not provide
        new_job->priority = 0; // default priority if not provided
        task_warn("invalid input format, expected: <id> <priority> <payload>\n");
    }

    insert_job_sorted(new_job);
    task_info("Written %d bytes to buffer\n", max_receive_bytes);
    return max_receive_bytes;
}

static loff_t tasker_seek(struct file *file, loff_t offset, int whence) { 
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
    task_info("Seek to position %lld\n", new_offset);
    return new_offset;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = tasker_open,
    .release = tasker_release,
    .read = tasker_read,
    .write = tasker_write,
    .llseek = tasker_seek,
};



int tasker_cdev_init(struct class *cls){
    // init and add cdev
    cdev_init(&cdev, &fops);
    cdev.owner = THIS_MODULE;
    return cdev_add(&cdev, dev_no, MINOR_NUMBERS);
}

void tasker_cdev_cleanup(struct class *cls){
    // cleanup
    cdev_del(&cdev);
}