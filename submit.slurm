#!/bin/bash
#SBATCH -N 1 #one node
#SBATCH -n 1 #one task
#SBATCH -t 00:05:00 #maximum wall time of 5 minutes

#load environment modules that correspond to the build environment
#and puts VTune in path

module load intel/19.1 intel-advisor/oneapi

#run command line Advisor, with Survey and Roofline analysis
advisor --collect=roofline -enable-cache-simulation --project-dir=./nbody-advisor -- ./app-ICC


