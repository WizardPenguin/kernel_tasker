#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>

#define DEVICE_NAME "/dev/task_driver"
#define BUFFER_SIZE 1024

struct thread_data {
    int id;
    int iterations;
};

void *worker(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int fd, ret;
    char read_buffer[BUFFER_SIZE];
    char write_buffer[BUFFER_SIZE];
    int id = data->id;
    int iterations = data->iterations;

    for(int i=0; i<iterations; i+=1){
        fd = open(DEVICE_NAME, O_RDWR);
        if(fd < 0) {
            perror("threaded : failed to open device");
            pthread_exit(NULL);
        }
        snprintf(write_buffer, BUFFER_SIZE, "Thread %d: Iteration %d", id, i);
        ret = write(fd, write_buffer, strlen(write_buffer));
        if(ret < 0) {
            perror("threaded : failed to write to device");
            close(fd);
            pthread_exit(NULL);
        }

        usleep(1000 + (rand() % 2000));

        // now read time
        lseek(fd, 0, SEEK_SET); // reset offset to start
        ret = read(fd, read_buffer, BUFFER_SIZE);
        if(ret < 0) {
            perror("threaded : failed to read from device");
            close(fd);
            pthread_exit(NULL);
        }
        read_buffer[ret] = '\0'; 
        printf("Thread %d iteration: %d Read from device : %s\n", id, i, read_buffer);
        close(fd);
        usleep(1000);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int thread = 4;
    int iterations = 10;

    if(argc >= 2) {
        thread = atoi(argv[1]);
    }
    if(argc >= 3) {
        iterations = atoi(argv[2]);
    }

    printf("starting %d threads with %d iterations each\n", thread, iterations);

    struct thread_data *data = malloc(thread * sizeof(struct thread_data));
    pthread_t *threads = malloc(thread*sizeof(pthread_t));

    for(int i=0; i<thread; i+=1) {
        data[i].id = i;
        data[i].iterations = iterations;
        if(pthread_create(&threads[i], NULL, worker, (void *)&data[i]) != 0) {
            perror("Failed to create thread");
            free(data);
            free(threads);
            return 1;
        }
    }

    for(int i=0; i<thread; i+=1) {
        pthread_join(threads[i], NULL);
    }
    free(data);
    free(threads);

    printf("All threads completed successfully\n");
    return 0;
}