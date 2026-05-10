#!/bin/bash
# cleanup.sh
rm -f /dev/shm/lab5_z1_shm
rm -f /dev/shm/sem.lab5_z1_mutex
rm -f /dev/shm/sem.lab5_z1_empty
rm -f /dev/shm/sem.lab5_z1_full
echo "Environment cleaned."
