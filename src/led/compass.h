#ifndef COMPASS_H
#define COMPASS_H
// 8-Direction Arrow Array for 6x8 LED Grid
// Each arrow is defined as an array of {x, y} coordinates
// Grid: 8 pixels wide (x: 0-7), 6 pixels tall (y: 0-5)
// Origin (0,0) is top-left corner

#include <stdint.h>

// Structure for coordinate pairs
typedef struct {
    uint8_t x;
    uint8_t y;
} point_t;

// Arrow directions enum for clarity
typedef enum {
    NORTH = 0,
    NORTHEAST,
    EAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    WEST,
    NORTHWEST
} direction_t;

// Arrow definitions - each arrow has variable number of points
// Format: {x, y} coordinates where (0,0) is top-left

// NORTH Arrow (pointing up)
static const point_t north_arrow[] = {
        {3, 0},    // tip
        {2, 1}, {4, 1}, {1, 2}, {5, 2},   // arrowhead
        {3, 1}, {3, 2}, {3, 3}, {3, 4},   // shaft
};

// NORTHEAST Arrow (pointing up-right)
static const point_t northeast_arrow[] = {
        {5, 0},    // tip
        {3, 0}, {4, 0}, {5, 1}, {5, 2},    // arrowhead
        {4, 1}, {3, 2}, {2, 3}, {1, 4}    // shaft diagonal
};

// EAST Arrow (pointing right)
static const point_t east_arrow[] = {
        {5, 2},    // tip
        {4, 1}, {3, 0},  {4, 3}, {3, 4},  // arrowhead
        {4, 2}, {3, 2}, {2, 2}, {1, 2},    // shaft
};

// SOUTHEAST Arrow (pointing down-right)
static const point_t southeast_arrow[] = {
        {5, 4},    // tip
        {3, 4}, {4, 4}, {5, 3}, {5, 2},    // arrowhead base
        {4, 3}, {3, 2}, {2, 1}, {1, 0}    // shaft diagonal
};

// SOUTH Arrow (pointing down)
static const point_t south_arrow[] = {
        {3, 4},    // tip
        {2, 3}, {4, 3},  {1, 2}, {5, 2},  // arrowhead
        {3, 0}, {3, 1}, {3, 2}, {3, 3},  // shaft
};

// SOUTHWEST Arrow (pointing down-left)
static const point_t southwest_arrow[] = {
        {1, 4},    // tip
        {1, 3}, {1, 2}, {2, 4}, {3, 4},    // arrowhead
        {2, 3}, {3, 2}, {4, 1}, {5, 0}    // shaft diagonal
};

// WEST Arrow (pointing left)
static const point_t west_arrow[] = {
        {1, 2},    // tip
        {2, 3}, {2, 1}, {3, 0}, {3, 4},    // arrowhead
        {2, 2}, {3, 2}, {4, 2}, {5, 2},    // shaft
};

// NORTHWEST Arrow (pointing up-left)
static const point_t northwest_arrow[] = {
        {1, 0},    // tip
        {2, 0}, {3, 0}, {1, 1}, {1, 2},    // arrowhead
        {2, 1}, {3, 2}, {4, 3}, {5, 4}    // shaft diagonal
};

// Array of arrow point counts
static const uint8_t arrow_point_counts[] = {
        sizeof(north_arrow) / sizeof(point_t),        // NORTH: 7 points
        sizeof(northeast_arrow) / sizeof(point_t),    // NORTHEAST: 9 points
        sizeof(east_arrow) / sizeof(point_t),         // EAST: 12 points
        sizeof(southeast_arrow) / sizeof(point_t),    // SOUTHEAST: 9 points
        sizeof(south_arrow) / sizeof(point_t),        // SOUTH: 7 points
        sizeof(southwest_arrow) / sizeof(point_t),    // SOUTHWEST: 9 points
        sizeof(west_arrow) / sizeof(point_t),         // WEST: 12 points
        sizeof(northwest_arrow) / sizeof(point_t)     // NORTHWEST: 9 points
};

// 3D Array: arrows[direction][point_index] = {x, y}
static const point_t* arrows[] = {
        north_arrow,
        northeast_arrow,
        east_arrow,
        southeast_arrow,
        south_arrow,
        southwest_arrow,
        west_arrow,
        northwest_arrow
};

uint8_t get_arrow_point_count(direction_t direction);
point_t get_arrow_point(direction_t direction, uint8_t point_index);
#endif