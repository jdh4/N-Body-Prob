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

Open `Makefile` with a text editor and make sure `ICXXFLAGS` has the following setting:

```
$ vim Makefile
ICXXFLAGS= -O0
```

Set the software environment by loading the Intel compiler module:

```
$ module purge
$ module load intel/19.1
```

Compile the C++ code by running the `make` on the `app-ICC` target:

```
$ make app-ICC

Compiling a ICC object file:
icpc -c -O0 -qopenmp -g -qopt-report=5 -qopt-report-phase=vec -inline-level=0 -qopt-report-filter="nbody.cc,56-111" -qopt-report-file=nbody.oicc.optrpt -o "nbody.oicc" "nbody.cc"

Linking the ICC executable:
icpc -O0 -qopenmp -o app-ICC nbody.oicc
rm nbody.oicc
```

Finally, run the job:

```
$ ./app-ICC

Propagating 16384 particles using 1 thread on CPU...


Particle data layout: Array of Structures (AoS)
 Step    Time, s Interact/s  GFLOP/s Energy
    1  7.597e+00  3.533e+07      0.7 *      0.2
    2  7.605e+00  3.529e+07      0.7 *      0.6
    3  7.610e+00  3.527e+07      0.7 *      1.4
    4  7.607e+00  3.529e+07      0.7       2.5
    5  7.596e+00  3.534e+07      0.7       3.9
    6  7.608e+00  3.528e+07      0.7       5.7
    7  7.622e+00  3.522e+07      0.7       7.7
    8  7.604e+00  3.530e+07      0.7      10.0
    9  7.606e+00  3.529e+07      0.7      12.7
   10  7.615e+00  3.525e+07      0.7      15.7
-----------------------------------------------------
Average performance:             0.7 +- 0.0 GFLOP/s
-----------------------------------------------------
* - warm-up, not included in average
```

For this exercise, the run times are short so it is okay to use the login node. In general, one should submit batch jobs [using Slurm](https://researchcomputing.princeton.edu/support/knowledge-base/slurm).

