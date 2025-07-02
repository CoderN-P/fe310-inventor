#include "compass.h"
#include <stdint.h>


// Helper function to get number of points for a specific arrow
uint8_t get_arrow_point_count(direction_t direction) {
    return arrow_point_counts[direction];
}

// Helper function to get a specific point from an arrow
point_t get_arrow_point(direction_t direction, uint8_t point_index) {
    if (point_index < arrow_point_counts[direction]) {
        return arrows[direction][point_index];
    }
    // Return invalid point if index out of bounds
    point_t invalid = {0xFF, 0xFF};
    return invalid;
}