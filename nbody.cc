/* 
Ian A. Cosden (icosden@princeton.edu)
Bei Wang (beiwang@princeton.edu)

Modified with Permission from Colfax International for the sole 
use of the attendees of the PICSciE mini-course "Performance and
Vectorization" 
*/

/* Copyright (c) 2013-2015, Colfax International. All Right Reserved.
This file, labs/4/4.01-overview-nbody/nbody.cc,
is a part of Supplementary Code for Practical Exercises for the handbook
"Parallel Programming and Optimization with Intel Xeon Phi Coprocessors",
2nd Edition -- 2015, Colfax International,
ISBN 9780988523401 (paper), 9780988523425 (PDF), 9780988523432 (Kindle).
Redistribution or commercial usage without written permission
from Colfax International is prohibited.
Contact information can be found at http://colfax-intl.com/    */

#include <cmath>
//#include <math.h>
#include <cstdio>
#include <omp.h>
#include <stdlib.h>

#ifdef SoA
struct ParticleArrays { 
  float *x, *y, *z;
  float *vx, *vy, *vz; 
};
#else
struct Particle { 
  float x, y, z;
  float vx, vy, vz; 
};
#endif

#ifdef SoA
float MoveParticles(const int nParticles, ParticleArrays &particle, const float dt) {
#else
float MoveParticles(const int nParticles, Particle* const particle, const float dt) {
#endif
  // Loop over particles that experience force
  float energy = 0.0f;
  const float G=1.0e-5f;
  const float softening = 1.0e-3f;
#ifdef SoA
  float *xp = particle.x;
  float *yp = particle.y;
  float *zp = particle.z;
  float *vxp = particle.vx;
  float *vyp = particle.vy;
  float *vzp = particle.vz;
#endif

  for (int i = 0; i < nParticles; i++) { 
    // Components of the gravity force on particle i
    float Fx = 0, Fy = 0, Fz = 0; 
#ifdef SoA
    const float xi = xp[i];
    const float yi = yp[i];
    const float zi = zp[i];
#else
    const float xi = particle[i].x;
    const float yi = particle[i].y;
    const float zi = particle[i].z;
#endif
      
    // Loop over particles that exert force: vectorization expected here
#ifdef OMP_SIMD
#if defined Aligned && defined SoA
#pragma omp simd aligned(xp, yp, zp: 64) reduction(+:Fx,Fy,Fz) 
#else
#pragma omp simd reduction(+:Fx,Fy,Fz)
#endif
#endif
    for (int j = 0; j < nParticles; j++) { 
      // Newton's law of universal gravity
#ifdef SoA
      const float dx = xp[j] - xi; // 1flop
      const float dy = yp[j] - yi; // 1flop
      const float dz = zp[j] - zi; // 1flop
#else
      const float dx = particle[j].x - xi;
      const float dy = particle[j].y - yi;
      const float dz = particle[j].z - zi;
#endif
      const float drSquared  = dx*dx + dy*dy + dz*dz + softening; // 6flops
#ifdef No_FP_Conv
      const float drPower32  = powf(drSquared, 3.0f/2.0f); // 1pow
#else
      const float drPower32  = pow(drSquared, 3.0/2.0);
#endif
      const float drPower32Inv = 1.0f / drPower32; // 1divident
      // Calculate the net force
      Fx += dx * G * drPower32Inv; // 3flops
      Fy += dy * G * drPower32Inv; // 3flops
      Fz += dz * G * drPower32Inv; // 3flops
    }

    // Accelerate particles in response to the gravitational force
#ifdef SoA
    vxp[i] += dt*Fx; //2flops
    vyp[i] += dt*Fy; //2flops
    vzp[i] += dt*Fz; //2flops
#else
    particle[i].vx += dt*Fx; 
    particle[i].vy += dt*Fy; 
    particle[i].vz += dt*Fz;
#endif
  }

  // Move particles according to their velocities
  // O(N) work, so using a serial loop

#ifdef OMP_SIMD
#if defined SoA & Aligned
#pragma omp simd aligned(xp,yp,zp,vxp,vyp,vzp: 64) reduction(+:energy) 
#else
#pragma omp simd reduction(+:energy)
#endif
#endif
  for (int i = 0 ; i < nParticles; i++) { 
#ifdef SoA
    xp[i]  += vxp[i]*dt; //2flops
    yp[i]  += vyp[i]*dt; //2flops
    zp[i]  += vzp[i]*dt; //2flops
    energy += vxp[i]*vxp[i] + vyp[i]*vyp[i] + vzp[i]*vzp[i]; //6flops
#else
    particle[i].x  += particle[i].vx*dt;
    particle[i].y  += particle[i].vy*dt;
    particle[i].z  += particle[i].vz*dt;
    energy += particle[i].vx*particle[i].vx + particle[i].vy*particle[i].vy + particle[i].vz*particle[i].vz;
#endif
  }

  return energy;
}

int main(const int argc, const char** argv) {

  // Problem size and other parameters
  const int nParticles = (argc > 1 ? atoi(argv[1]) : 16384);
  const int nSteps = 10;  // Duration of test
  const float dt = 0.01f; // Particle propagation time step

#ifdef SoA
  // Particle data stored now stored as Structure of Arrays (SoA)
  // This is makes vectorization more efficient by improving strided access
  ParticleArrays particle;
#ifdef Aligned 
  posix_memalign((void **)&particle.x, 64, nParticles*sizeof(float));
  posix_memalign((void **)&particle.y, 64, nParticles*sizeof(float));
  posix_memalign((void **)&particle.z, 64, nParticles*sizeof(float));
  posix_memalign((void **)&particle.vx, 64, nParticles*sizeof(float));
  posix_memalign((void **)&particle.vy, 64, nParticles*sizeof(float));
  posix_memalign((void **)&particle.vz, 64, nParticles*sizeof(float));
#else
  particle.x  = new float[nParticles];
  particle.y  = new float[nParticles];
  particle.z  = new float[nParticles];
  particle.vx = new float[nParticles];
  particle.vy = new float[nParticles];
  particle.vz = new float[nParticles];
#endif

  // Initialize random number generator and particles
  for (int i=0; i<nParticles; i++) {
    particle.x[i]  = (float) rand() / (float) RAND_MAX;
    particle.y[i]  = (float) rand() / (float) RAND_MAX;
    particle.z[i]  = (float) rand() / (float) RAND_MAX;
    particle.vx[i] = (float) rand() / (float) RAND_MAX * 1.0e-3f;
    particle.vy[i] = (float) rand() / (float) RAND_MAX * 1.0e-3f;
    particle.vz[i] = (float) rand() / (float) RAND_MAX * 1.0e-3f;   
  }
#else
  // Particle data stored as an Array of Structures (AoS)
  // this is good object-oriented programming style,
  // but inefficient for the purposes of vectorization
  Particle* particle = new Particle[nParticles];

  // Initialize random number generator and particles
  for (int i=0; i<nParticles; i++) {
    particle[i].x = (float) rand() / (float) RAND_MAX;
    particle[i].y = (float) rand() / (float) RAND_MAX;
    particle[i].z = (float) rand() / (float) RAND_MAX;
    particle[i].vx = (float) rand() / (float) RAND_MAX * 1.0e-3f;
    particle[i].vy = (float) rand() / (float) RAND_MAX * 1.0e-3f;
    particle[i].vz = (float) rand() / (float) RAND_MAX * 1.0e-3f;
  }
#endif

  // Perform benchmark
  printf("\nPropagating %d particles using 1 thread on CPU...\n\n", 
	 nParticles);
#ifdef SoA
  printf("\nParticle data layout: Structure of Arrays (SoA)\n");
#else
  printf("\nParticle data layout: Array of Structures (AoS)\n");
#endif

  double rate = 0, dRate = 0; // Benchmarking data
  const int skipSteps = 3; // Skip first iteration is warm-up on Xeon Phi coprocessor
  printf("\033[1m%5s %10s %10s %8s %6s\033[0m\n", "Step", "Time, s", "Interact/s", "GFLOP/s", "Energy"); fflush(stdout);
  for (int step = 1; step <= nSteps; step++) {

    const double tStart = omp_get_wtime(); // Start timing
    float energy = MoveParticles(nParticles, particle, dt);
    const double tEnd = omp_get_wtime(); // End timing

    const float HztoInts   = float(nParticles)*float(nParticles-1);
    const float HztoGFLOPs= 1.0e-9*((11.0+9.0)*float(nParticles)*float(nParticles) + (6.0+12.0)*float(nParticles));

    if (step > skipSteps) { // Collect statistics
      rate  += HztoGFLOPs/(tEnd - tStart); 
      dRate += HztoGFLOPs*HztoGFLOPs/((tEnd - tStart)*(tEnd-tStart)); 
    }

    printf("%5d %10.3e %10.3e %8.1f %s %8.1f\n", 
	   step, (tEnd-tStart), HztoInts/(tEnd-tStart), HztoGFLOPs/(tEnd-tStart), (step<=skipSteps?"*":""), energy);
    fflush(stdout);
  }
  rate/=(double)(nSteps-skipSteps); 
  dRate=sqrt(dRate/(double)(nSteps-skipSteps)-rate*rate);
  printf("-----------------------------------------------------\n");
  printf("\033[1m%s %4s \033[42m%10.1f +- %.1f GFLOP/s\033[0m\n",
	 "Average performance:", "", rate, dRate);
  printf("-----------------------------------------------------\n");
  printf("* - warm-up, not included in average\n\n");
#ifdef SoA
#ifdef Aligned
  free(particle.x);
  free(particle.y);
  free(particle.z);
  free(particle.vx);
  free(particle.vy);
  free(particle.vz);
#else
  delete particle.x;
  delete particle.y;
  delete particle.z;
  delete particle.vx;
  delete particle.vy;
  delete particle.vz;
#endif
#else
  delete particle;
#endif
}


