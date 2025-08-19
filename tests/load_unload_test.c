#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<error.h>

int run_load_unload_test(){
    int fd = open("/dev/task_driver", O_RDWR);
    if( fd < 0 ) {
        perror("load_unload_test : Failed to open device");
        return -1;
    }
    printf("load_unload_test : Device opened successfully\n");
    close(fd);
    return 0;
}