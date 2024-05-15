#! /bin/sh

current_directory=$(pwd)
program_task="solver"
program_task_conf="solver.json"

cd build/src/


./MultiResourceProjectScheduler  --program_task ${program_task} \
                                 --program_task_conf ${program_task_conf} \
                                 --verbosity "debug"
