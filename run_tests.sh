#!/bin/bash
set -e

MODULE_NAME="tasker"
DEVICE_PATH="/dev/task_driver"

echo "Building kernel module..."
cd src
make 
cd ..

echo "Building test programs..."
cd tests
make
cd ..

echo "Loading kernel module..."
sudo insmod src/$MODULE_NAME.ko || { echo "Failed to load module"; exit 1; }

sleep 1
sudo dmesg | tail -n 5

echo "Running tests..."
sudo ./tests/tasker_test

echo "Removing kernel module..."
sudo rmmod $MODULE_NAME || { echo "Failed to remove module"; exit 1; }

sleep 1
sudo dmesg | tail -n 5
echo "✅ DONE"
