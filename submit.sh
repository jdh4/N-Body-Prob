#!/bin/bash
#SBATCH --job-name=nbody         # create a short name for your job
#SBATCH --nodes=1                # node count
#SBATCH --ntasks=1               # total number of tasks across all nodes
#SBATCH --cpus-per-task=1        # cpu-cores per task (>1 if multi-threaded tasks)
#SBATCH --mem=4G                 # memory per node
#SBATCH --time=00:05:00          # total run time limit (HH:MM:SS)
#SBATCH --reservation=bootcamp1  # REMOVE THIS WHEN BOOTCAMP IS NOT IN SESSION

# set the software environment
module purge
module load intel/19.1
module load intel-advisor/oneapi

# run advisor in command-line mode on the executable app-ICC
advisor --collect=roofline -enable-cache-simulation --project-dir=./nbody-advisor -- ./app-ICC
