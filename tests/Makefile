CC=gcc
CFLAGS=-Wall -pthread -O2
TESTS=read_write_test.o read_write_threaded.o load_unload_test.o priority_order_test.o
TARGET=tasker_test

all: $(TARGET)

$(TARGET): $(TESTS) test_runner.o
	$(CC) $(CFLAGS) -o $@ $^ 

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(TESTS) test_runner.o