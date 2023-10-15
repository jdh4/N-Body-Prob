#!/bin/bash

# submits job script 'submit.slurm' to SLURM scheduler 
# then waits for it to finish

jobid=`sbatch submit.slurm | sed 's/[^0-9]//g'`
echo "Submitted batch job $jobid"

#test if finished
while [[ $(squeue -j $jobid | grep $USER) ]]; do
    echo "running..."
    sleep 5 
done
    echo "done."


