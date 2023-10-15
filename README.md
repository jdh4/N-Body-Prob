# Performance and Vectorization

```
$ ssh <YourNetID>@adroit.princeton.edu
$ cd /scratch/network/$USER
$ git clone https://github.com/jdh4/N-Body-Prob.git
$ cd N-Body-Prob
$ ls -l
total 28
-rw-r--r--. 1 jdh4 cses 1222 Oct 15 13:06 Makefile
-rw-r--r--. 1 jdh4 cses 8288 Oct 15 13:14 nbody.cc
-rw-r--r--. 1 jdh4 cses   46 Oct 15 12:53 README.md
-rw-r--r--. 1 jdh4 cses  739 Oct 15 13:04 submit.slurm
-rwxr-xr-x. 1 jdh4 cses  307 Oct 15 13:06 submit_to_scheduler.sh
```

Use a text editor to inspect the source code:

```
$ vim nbody.cc  # or emacs or nano
```

The source code features blocks with `#ifdef ... #else ... #endif`. These are directives to the preprocessor that allow us to quickly include or exclude blocks of code. The preprocessor is the tool that runs before the compiler.

For this exercise, you will change the compilation settings to investigate the effect of different compiler options and code design.

Baseline

We will use the Intel compiler on Adroit.

Open `Makefile` with a text editor and m

```
$ vim Makefile
ICXXFLAGS= -O0
```

Run the next two commands to compile the code and submit the job to the scheduler:

```
$ make
$ ./submit_to_scheduler.sh
```
