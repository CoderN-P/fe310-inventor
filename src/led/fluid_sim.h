//
// Created by Neel P on 5/17/26.
//

#ifndef FLUID_SIM_H
#define FLUID_SIM_H

#include <stdbool.h>

#define NUM_PARTICLES 30
#define G 120
#define TIME_STEP 1.0 / 30.0
#define WIDTH 10
#define HEIGHT 8
#define CELL_SIZE 1.0
#define AIR 0
#define WATER 1
#define SOLID 2
#define DENSITY 1000.0
#define PARTICLE_RADIUS 0.5f
#define OVER_RELAXATION 1.9f
#define PRESSURE_ITERATIONS 4
#define PUSH_ITERATIONS 1
#define FLIP_RATIO 0.98

#define clamp(v, lo, hi) (((v) < (lo)) ? (lo) : (((v) > (hi)) ? (hi) : (v)))

typedef struct Particle
{
    float x, y;
    float u, v;
} Particle;

extern int cellType[WIDTH][HEIGHT];
extern float particleDensity[WIDTH][HEIGHT];
extern float particleRestDensity;

void update(float pitch, float roll);
void initFluidSim(void);

#endif // FLUID_SIM_H
