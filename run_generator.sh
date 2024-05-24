#! /bin/sh

program_task="generator"
program_task_conf="generator.json"

cd build/src/


./MultiResourceProjectScheduler  --program_task ${program_task} \
                                 --program_task_conf ${program_task_conf} \
                                 --verbosity "debug"