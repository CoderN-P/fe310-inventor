#include "fluid_sim.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>

int cellType[WIDTH][HEIGHT] = {0};
float particleDensity[WIDTH][HEIGHT] = {0};
float particleRestDensity = 0.0f;

static float uGrid[WIDTH + 1][HEIGHT];
static float vGrid[WIDTH][HEIGHT + 1];
static float prevU[WIDTH + 1][HEIGHT];
static float prevV[WIDTH][HEIGHT + 1];
static float du[WIDTH + 1][HEIGHT];
static float dv[WIDTH][HEIGHT + 1];
static float pressure[WIDTH][HEIGHT];
static int solid[WIDTH][HEIGHT];

static Particle particles[NUM_PARTICLES];

static void updateParticles(float pitch, float roll);
static void transferVelocities(bool toGrid);
static void solveIncompressibility(bool compensateDrift);
static void updateParticleDensity(void);
static void pushParticlesApart(void);

void update(float pitch, float roll){
    updateParticles(pitch, roll);
    pushParticlesApart();
    transferVelocities(true);
    updateParticleDensity();
    solveIncompressibility(true);
    transferVelocities(false);
}

void initFluidSim(){
    for (int i = 0; i < NUM_PARTICLES; i++){
        particles[i].x = (WIDTH / 2 + (i % 5) - 2) * CELL_SIZE;
        particles[i].y = (HEIGHT / 2 + (i / 5) - 2) * CELL_SIZE;
        particles[i].u = 0;
        particles[i].v = 0;
    }

    for (int i = 0; i < WIDTH; i++){
        cellType[i][0] = SOLID;
        cellType[i][HEIGHT - 1] = SOLID;
    }

    for (int j = 0; j < HEIGHT; j++){
        cellType[0][j] = SOLID;
        cellType[WIDTH - 1][j] = SOLID;
    }

    for (int i = 0; i < WIDTH; i++){
        for (int j = 0; j < HEIGHT; j++){
            solid[i][j] = (cellType[i][j] == SOLID) ? 0 : 1;
        }
    }
}

static void updateParticles(float pitch, float roll){
    float ax = G * sinf(roll);
    float ay = G * sinf(pitch);

    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].u += ax * TIME_STEP;
        particles[i].v += ay * TIME_STEP;
        particles[i].x += particles[i].u * TIME_STEP;
        particles[i].y += particles[i].v * TIME_STEP;

        if (particles[i].x < PARTICLE_RADIUS) {
            particles[i].x = PARTICLE_RADIUS;
            particles[i].u = 0;
        } else if (particles[i].x > (WIDTH - 1) * CELL_SIZE - PARTICLE_RADIUS) {
            particles[i].x = (WIDTH - 1) * CELL_SIZE - PARTICLE_RADIUS;
            particles[i].u = 0;
        }

        if (particles[i].y < PARTICLE_RADIUS + CELL_SIZE) {
            particles[i].y = PARTICLE_RADIUS + CELL_SIZE;
            particles[i].v = 0;
        } else if (particles[i].y > (HEIGHT - 1) * CELL_SIZE - PARTICLE_RADIUS) {
            particles[i].y = (HEIGHT - 1) * CELL_SIZE - PARTICLE_RADIUS;
            particles[i].v = 0;
        }
    }
}

static void transferVelocities(bool toGrid){
    if (toGrid) {
        memcpy(prevU, uGrid, sizeof(uGrid));
        memcpy(prevV, vGrid, sizeof(vGrid));

        memset(uGrid, 0, sizeof(uGrid));
        memset(vGrid, 0, sizeof(vGrid));
        memset(du, 0, sizeof(du));
        memset(dv, 0, sizeof(dv));

        for (int i = 0; i < WIDTH; i++){
            for (int j = 0; j < HEIGHT; j++){
                cellType[i][j] = (solid[i][j] == 0) ? SOLID : AIR;
            }
        }

        for (int i = 0; i < NUM_PARTICLES; i++){
            int cellX = (int)(particles[i].x / CELL_SIZE);
            int cellY = (int)(particles[i].y / CELL_SIZE);

            if (cellType[cellX][cellY] == AIR) {
                cellType[cellX][cellY] = WATER;
            }
        }
    }

    for (int component = 0; component <= 1; component++){
        float dx = (component == 0) ? 0 : CELL_SIZE / 2;
        float dy = (component == 0) ? CELL_SIZE / 2 : 0;

        for (int i = 0; i < NUM_PARTICLES; i++){
            float x = clamp(particles[i].x, CELL_SIZE, (WIDTH - 1) * CELL_SIZE);
            float y = clamp(particles[i].y, CELL_SIZE, (HEIGHT - 1) * CELL_SIZE);

            int x0 = fmin((int)(x - dx) / CELL_SIZE, WIDTH - 2);
            float tx = ((x - dx) - x0 * CELL_SIZE) / CELL_SIZE;
            int x1 = fmin(x0 + 1, WIDTH - 2);

            int y0 = fmin((int)(y - dy) / CELL_SIZE, HEIGHT - 2);
            float ty = ((y - dy) - y0 * CELL_SIZE) / CELL_SIZE;
            int y1 = fmin(y0 + 1, HEIGHT - 2);

            float sx = 1.0 - tx;
            float sy = 1.0 - ty;

            float d0 = sx * sy;
            float d1 = tx * sy;
            float d3 = tx * ty;
            float d2 = sx * ty;

            if (toGrid) {
                if (component == 0){
                    uGrid[x0][y0] += particles[i].u * d0;
                    uGrid[x1][y0] += particles[i].u * d1;
                    uGrid[x0][y1] += particles[i].u * d2;
                    uGrid[x1][y1] += particles[i].u * d3;
                    du[x0][y0] += d0;
                    du[x1][y0] += d1;
                    du[x0][y1] += d2;
                    du[x1][y1] += d3;
                } else {
                    vGrid[x0][y0] += particles[i].v * d0;
                    vGrid[x1][y0] += particles[i].v * d1;
                    vGrid[x0][y1] += particles[i].v * d2;
                    vGrid[x1][y1] += particles[i].v * d3;
                    dv[x0][y0] += d0;
                    dv[x1][y0] += d1;
                    dv[x0][y1] += d2;
                    dv[x1][y1] += d3;
                }
            } else {
                float valid0, valid1, valid2, valid3;
                if (component == 0) {
                    valid0 = (cellType[x0][y0] != AIR || cellType[x0 - 1][y0] != AIR) ? 1.0 : 0.0;
                    valid1 = (cellType[x1][y0] != AIR || cellType[x1 - 1][y0] != AIR) ? 1.0 : 0.0;
                    valid2 = (cellType[x0][y1] != AIR || cellType[x0 - 1][y1] != AIR) ? 1.0 : 0.0;
                    valid3 = (cellType[x1][y1] != AIR || cellType[x1 - 1][y1] != AIR) ? 1.0 : 0.0;
                } else {
                    valid0 = (cellType[x0][y0] != AIR || cellType[x0][y0 - 1] != AIR) ? 1.0 : 0.0;
                    valid1 = (cellType[x1][y0] != AIR || cellType[x1][y0 - 1] != AIR) ? 1.0 : 0.0;
                    valid2 = (cellType[x0][y1] != AIR || cellType[x0][y1 - 1] != AIR) ? 1.0 : 0.0;
                    valid3 = (cellType[x1][y1] != AIR || cellType[x1][y1 - 1] != AIR) ? 1.0 : 0.0;
                }

                float d = valid0 * d0 + valid1 * d1 + valid2 * d2 + valid3 * d3;

                if (d > 0.0){
                    if (component == 0){
                        float picU = (valid0 * d0 * uGrid[x0][y0] + valid1 * d1 * uGrid[x1][y0] + valid2 * d2 * uGrid[x0][y1] + valid3 * d3 * uGrid[x1][y1]) / d;
                        float corr = (valid0 * d0 * (uGrid[x0][y0] - prevU[x0][y0]) + valid1 * d1 * (uGrid[x1][y0] - prevU[x1][y0]) + valid2 * d2 * (uGrid[x0][y1] - prevU[x0][y1]) + valid3 * d3 * (uGrid[x1][y1] - prevU[x1][y1])) / d;
                        particles[i].u = (1 - FLIP_RATIO) * picU + FLIP_RATIO * (particles[i].u + corr);
                    } else {
                        float picV = (valid0 * d0 * vGrid[x0][y0] + valid1 * d1 * vGrid[x1][y0] + valid2 * d2 * vGrid[x0][y1] + valid3 * d3 * vGrid[x1][y1]) / d;
                        float corr = (valid0 * d0 * (vGrid[x0][y0] - prevV[x0][y0]) + valid1 * d1 * (vGrid[x1][y0] - prevV[x1][y0]) + valid2 * d2 * (vGrid[x0][y1] - prevV[x0][y1]) + valid3 * d3 * (vGrid[x1][y1] - prevV[x1][y1])) / d;
                        particles[i].v = (1 - FLIP_RATIO) * picV + FLIP_RATIO * (particles[i].v + corr);
                    }
                }
            }
        }

        if (toGrid) {
            if (component == 0){
                for (int i = 0; i < WIDTH + 1; i++){
                    for (int j = 0; j < HEIGHT; j++){
                        if (du[i][j] > 0.0){
                            uGrid[i][j] /= du[i][j];
                        }
                    }
                }
            } else {
                for (int i = 0; i < WIDTH; i++) {
                    for (int j = 0; j < HEIGHT + 1; j++) {
                        if (dv[i][j] > 0.0) {
                            vGrid[i][j] /= dv[i][j];
                        }
                    }
                }
            }

            for (int i = 0; i < WIDTH; i++) {
                for (int j = 0; j < HEIGHT; j++){
                    if (cellType[i][j] == SOLID || (i > 0 && cellType[i - 1][j] == SOLID)){
                        uGrid[i][j] = prevU[i][j];
                    }
                    if (cellType[i][j] == SOLID || (j > 0 && cellType[i][j - 1] == SOLID)){
                        vGrid[i][j] = prevV[i][j];
                    }
                }
            }
        }
    }
}

static void solveIncompressibility(bool compensateDrift){
    memcpy(prevU, uGrid, sizeof(uGrid));
    memcpy(prevV, vGrid, sizeof(vGrid));

    memset(pressure, 0, sizeof(pressure));

    float cp = DENSITY * CELL_SIZE / TIME_STEP;

    for (int iter = 0; iter < PRESSURE_ITERATIONS; iter++){
        for (int i = 1; i < WIDTH - 1; i++){
            for (int j = 1; j < HEIGHT - 1; j++){
                if (cellType[i][j] != WATER) continue;

                int sx0 = solid[i - 1][j];
                int sx1 = solid[i + 1][j];
                int sy0 = solid[i][j - 1];
                int sy1 = solid[i][j + 1];
                int s = sx0 + sx1 + sy0 + sy1;

                if (s == 0) continue;

                float div = (uGrid[i + 1][j] - uGrid[i][j]) + (vGrid[i][j + 1] - vGrid[i][j]);

                if (particleRestDensity > 0 && compensateDrift){
                    float compression = particleDensity[i][j] / particleRestDensity;

                    if (compression > 1.0f){
                        div -= compression;
                    }
                }

                float p = (-div / s) * OVER_RELAXATION;
                pressure[i][j] += p * cp;

                uGrid[i][j] -= p * sx0;
                uGrid[i + 1][j] += p * sx1;
                vGrid[i][j] -= p * sy0;
                vGrid[i][j + 1] += p * sy1;
            }
        }
    }
}

static void updateParticleDensity(void) {
    memset(particleDensity, 0, sizeof(particleDensity));

    for (int i = 0; i < NUM_PARTICLES; i++){
        float x = clamp(particles[i].x, CELL_SIZE, (WIDTH - 1) * CELL_SIZE);
        float y = clamp(particles[i].y, CELL_SIZE, (HEIGHT - 1) * CELL_SIZE);

        int x0 = (int)((x - CELL_SIZE / 2) / CELL_SIZE);
        float tx = ((x - CELL_SIZE / 2) - x0 * CELL_SIZE) / CELL_SIZE;
        int x1 = fmin(x0 + 1, WIDTH - 2);

        int y0 = (int)((y - CELL_SIZE / 2) / CELL_SIZE);
        float ty = ((y - CELL_SIZE / 2) - y0 * CELL_SIZE) / CELL_SIZE;
        int y1 = fmin(y0 + 1, HEIGHT - 2);

        float sx = 1.0 - tx;
        float sy = 1.0 - ty;

        if (x0 < WIDTH && y0 < HEIGHT) particleDensity[x0][y0] += sx * sy;
        if (x1 < WIDTH && y0 < HEIGHT) particleDensity[x1][y0] += tx * sy;
        if (x1 < WIDTH && y1 < HEIGHT) particleDensity[x1][y1] += tx * ty;
        if (x0 < WIDTH && y1 < HEIGHT) particleDensity[x0][y1] += sx * ty;
    }

    if (particleRestDensity == 0){
        float sum = 0;
        int numFluidCells = 0;

        for (int i = 0; i < WIDTH; i++){
            for (int j = 0; j < HEIGHT; j++){
                if (cellType[i][j] == WATER){
                    sum += particleDensity[i][j];
                    numFluidCells++;
                }
            }
        }

        if (numFluidCells > 0){
            particleRestDensity = sum / numFluidCells;
        }
    }
}

static void pushParticlesApart(void){
    const float minDist = 2.0 * PARTICLE_RADIUS;
    const float minDist2 = minDist * minDist;

    for (int iter = 0; iter < PUSH_ITERATIONS; iter++){
        for (int i = 0; i < NUM_PARTICLES; i++){
            for (int j = i + 1; j < NUM_PARTICLES; j++){
                float dx = particles[j].x - particles[i].x;
                float dy = particles[j].y - particles[i].y;
                float dist2 = dx * dx + dy * dy;

                if (dist2 < minDist2 && dist2 > 0){
                    float dist = sqrtf(dist2);
                    float overlap = 0.5f * (minDist - dist) / dist;
                    float offsetX = overlap * dx;
                    float offsetY = overlap * dy;

                    particles[i].x -= offsetX;
                    particles[i].y -= offsetY;
                    particles[j].x += offsetX;
                    particles[j].y += offsetY;
                }
            }
        }
    }
}
