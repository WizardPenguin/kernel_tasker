#include "tasker.h"

LIST_HEAD(job_list);
DEFINE_MUTEX(job_mutex);

void insert_job_sorted(struct job_data *job){
    struct job_data *job_data, *tmp;

    mutex_lock(&job_mutex); // Wait for the lock to be available
    // since list is already sorted, we can insert at the right position
    list_for_each_entry_safe(job_data, tmp, &job_list, list) {
        if(job->priority > job_data->priority) {
            // insert before this job
            list_add(&job->list, &job_data->list);
            mutex_unlock(&job_mutex);
            task_info("Job with ID %d inserted with priority %d\n", job->id, job->priority);
            return;
        }
    }
    list_add_tail(&job->list, &job_list); // add to end of list
    mutex_unlock(&job_mutex);
    return;
}

struct job_data* get_next_job(void){
    // return head, hightest priority job
    struct job_data *job;

    mutex_lock(&job_mutex); 
    if(list_empty(&job_list)) {
        task_err("No jobs available to read\n");
        mutex_unlock(&job_mutex);
        return NULL; // No jobs available.
    }
    job = list_first_entry(&job_list, struct job_data, list);
    list_del(&job->list); // remove it from list
    mutex_unlock(&job_mutex);
    return job;
}

void clear_job_list(void){
    struct job_data *job_data, *tmp;
    mutex_lock(&job_mutex); // Wait for the lock to be available
    list_for_each_entry_safe(job_data, tmp, &job_list, list) {
        list_del(&job_data->list);
        kfree(job_data);
    }
    mutex_unlock(&job_mutex);
    task_info("Cleared job list\n");
}