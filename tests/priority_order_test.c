#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<error.h>

int run_priority_order_test() {
    int ret = 0;
    int fd = open("/dev/task_driver", O_RDWR);
    if (fd < 0) {
        perror("priority_order_test : Failed to open device");
        return -1;
    }
    printf("priority_order_test : Device opened successfully\n");
    
    char buf[1024];
    memset(buf, 0, sizeof(buf));

    // Writing tasks with different priorities
    for (int i = 0; i < 10; i++) {
        int prio = rand() % 5;   // random priority [0-4]
        ret = dprintf(fd, "%d %d payload%d\n", i, prio, i);
        if(ret < 0) {
            perror("priority_order_test : Failed to write to device");
            close(fd);
            return -1;
        }
        printf("priority_order_test : Written task %d with priority %d\n", i, prio);
    }

    for (int i = 0; i < 10; i++) {
        ret = read(fd, buf, sizeof(buf));
        if(ret < 0) {
            perror("priority_order_test : Failed to read from device");
            close(fd);
            return -1;
        }
        buf[ret] = '\0'; // Null-terminate the string
        printf("priority_order_test : Read %d bytes from device: %s\n", ret, buf);
    }
}
