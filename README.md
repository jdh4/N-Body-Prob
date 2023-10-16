# Performance and Vectorization

Run the commands below to obtain the code:

```bash
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

The source code features lines such as `#ifdef ... #else ... #endif`. These are directives to the preprocessor that allow us to quickly include or exclude blocks of code. The preprocessor is the tool that runs before the compiler.

For this exercise, you will change the compilation settings to investigate the effect of vectorization.

### Baseline (no vectorization, no optimization)

Let's first compile and run the code with vectorization and optimization turned off. Open `Makefile` with a text editor and make sure `ICXXFLAGS` has the following setting:

```
$ vim Makefile
...
ICXXFLAGS= -O0
...
```

Set the software environment by loading the Intel compiler module:

```bash
$ module purge
$ module load intel/19.1
```

Compile the C++ code by running the `make` on the `app-ICC` target:

```
$ make

Compiling a ICC object file:
icpc -c -O0 -qopenmp -g -qopt-report=5 -qopt-report-phase=vec -inline-level=0 -qopt-report-filter="nbody.cc,56-111" -qopt-report-file=nbody.oicc.optrpt -o "nbody.oicc" "nbody.cc"

Linking the ICC executable:
icpc -O0 -qopenmp -o app-ICC nbody.oicc
rm nbody.oicc
```

Inspect the vectorization report which is uninteresting for this case since vectorization is turned off:

```
$ cat vec.report
Intel(R) Advisor can now assist with vectorization and show optimization
  report messages with your source code.
See "https://software.intel.com/en-us/intel-advisor-xe" for details.

Intel(R) C++ Intel(R) 64 Compiler for applications running on Intel(R) 64, Version 19.1.1.217 Build 20200306

Compiler options: -c -O0 -qopenmp -g -qopt-report=5 -qopt-report-phase=vec -inline-level=0 -qopt-report-filter=nbody.cc,56-111 -qopt-report-file=vec.report -o nbody.oicc
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

We see that our baseline performance is 0.7 GFLOP/s.

For this exercise, the run times are short so it is okay to use the login node. In general, one should submit batch jobs [using Slurm](https://researchcomputing.princeton.edu/support/knowledge-base/slurm).

In preparation for the next run, remove the executable and other files:

```
$ make clean
```

### 128-bit Vectorization

Let's repeat the procedure with 128-bit vectorization:

```
$ make clean
$ vim Makefile
...
ICXXFLAGS = -O2
...
```

Compile the code, inspect the report, and then run the code:

```
$ make
$ cat vec.report
...
$ ./app-ICC
```

What stands out from the vectorization report? Do the variables in nbody.cc use single or double precision? Does the code run faster when compiled with `-O2`?

### 256-bit Vectorization

Now let's try with 256-bit vectorization:

```
$ make clean
$ vim Makefile
...
ICXXFLAGS = -O2 -xCORE-AVX2
...
```

Compile the code, inspect the report, and then run the code:

```
$ make
$ cat vec.report
...
$ ./app-ICC
```

In the vectorization report, what is the vector length? Does the value make sense? Did the "estimated potential speedup" increase over 128-bit vectorization?

### AVX-512

Now let's try with 512-bit vectorization:

```
$ make clean
$ vim Makefile
...
ICXXFLAGS = -O2 -xCORE-AVX512 -qopt-zmm-usage=high
...
```

Compile the code, inspect the report, and then run the code:

```
$ make
$ cat vec.report
...
$ ./app-ICC
```

In the vectorization report, what is the vector length? Does the value make sense? Did the "estimated potential speedup" increase over 128-bit vectorization?

To see all of the compiler options:

```
$ man icpc
```

### Preventing data types conversions

We see from vec.report the following:

```
remark #15417: vectorization support: number of FP up converts: single precision to double precision 2   [ nbody.cc(94,32) ]
remark #15418: vectorization support: number of FP down converts: double precision to single precision 1   [ nbody.cc(94,32) ]
```

We can turn off the casting by add the switch `–DNo_FP_Conv`. Modify `Makefile` as follows:

```
ICXXFLAGS = -O2 -xCORE-AVX512 -qopt-zmm-usage=high -DNo_FP_Conv
```

Then run `make clean`, `make` and run the code. Does the vectorization report change? Do you see a speed-up?

### Structure of Arrays

Let's address the issue with non-unit stride by adding the switch `-DSoA`. Modify `Makefile` as follows:

```
ICXXFLAGS = -O2 -xCORE-AVX512 -qopt-zmm-usage=high -DNo_FP_Conv -DSoA
```

Then run `make clean`, `make` and run the code. Does the vectorization report change? Do you see a speed-up?

### Memory alignment

Let's address the issue with memory alignment by adding the switch`-DOMP_SIMD -DAligned`.

```
ICXXFLAGS = -O2 -xCORE-AVX512 -qopt-zmm-usage=high -DNo_FP_Conv -DSoA -DOMP_SIMD -DAligned
```

Then run `make clean`, `make` and run the code. Does the vectorization report change? Do you see a speed-up?

## Roofline Analysis

Watch [this video](https://mediacentral.princeton.edu/media/t/1_5nhl128acd) to see how to conduct a roofline analysis on Adroit. After starting a graphical desktop on `adroit-vis`, run these commands in a terminal:

```bash
$ cd /scratch/network/$USER/N-Body-Prob
$ module load intel/19.1 intel-advisor/oneapi
$ make
$ advisor --collect=roofline -enable-cache-simulation --project-dir=./nbody-advisor -- ./app-ICC
$ advixe-gui nbody-advisor
```

## Troubleshooting

If you see the error below then the horizontal line character before 'xCORE' is not a hyphen. The solution is to replace it with a hyphen.

```
icpc: error #10236: File not found:  '–xCORE-AVX2'
```
