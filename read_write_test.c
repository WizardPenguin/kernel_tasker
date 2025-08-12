#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<error.h>

char buffer[1024];

int read_write(int fd, const char *data, size_t len) {
    // writting data to device
    ssize_t bytes_written = write(fd, data, len);
    if(bytes_written < 0) {
        perror("read_write_test : Failed to write to device");
        return -1;
    }
    printf("read_write_test : Written %zd bytes to device\n", bytes_written);

    // reading data back
    if(lseek(fd, 0, SEEK_SET) < 0) {
        perror("read_write_test : failed to seek in driver");
        return -1;
    }
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if(bytes_read < 0) {
        perror("read_write_test : failed to read from device");
        return -1;
    }
    buffer[bytes_read] = '\0'; // Null-terminate the string
    printf("read_write_test : Read %zd bytes from device : %s\n", bytes_read, buffer);

    if(lseek(fd, len, SEEK_SET) < 0){
        perror("read_write_test : failed to seek in driver");
        return -1;
    }
    printf("read_write_test : Seek to position %ld\n", len);
    return 0;
}

int main(){
    int fd = open("/dev/task_driver", O_RDWR);
    if( fd < 0 ) {
        perror("read_write_test : failed to open device");
        return -1;
    }
    printf("read_write_test : Device opened successfully\n");
    
    memset(buffer, 0, sizeof(buffer));
    char *data = "Hello, kernel tasker!\n";
    size_t len = strlen(data);
    if(read_write(fd, data, len) < 0) {
        close(fd);
        return -1;
    }

    data = "another messages to kernel tasker";
    len = strlen(data);
    read_write(fd, data, len);

    close(fd);
    return 0;
}