#!/bin/bash

# This script is used to test the kernel tasker module
# It loads the module, creates a task, and checks if the task is created successfully

set -e

MODULE_NAME="task_driver"
DEVICE_PATH="/dev/task_device"
TEST_FILE="load_unload_test.c"
TEST_PROGRAM="load_unload_test"

# build module
echo "Building the kernel module..."
make 

# load module
echo "loading the kernel module..."
sudo insmod $MODULE_NAME.ko || { echo "failed to load module"; exit 1; }

sleep 1
# check dmesg 

echo "checking dmesg for module load messages..."
sudo dmesg | tail -n 5

# compile test porgram

gcc -o $TEST_PROGRAM $TEST_FILE

# run test program
./$TEST_PROGRAM

# remove module 
echo "Removing the kernel module..."
sudo rmmod $MODULE_NAME || { echo "fialed to remove module"; echo 1; }

sleep 1
# check dmesg again
echo "checking dmesg for module unload messages..."
sudo dmesg | tail -n 5

echo "DONE"



