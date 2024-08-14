#include "glm/fwd.hpp"
#include "glm/geometric.hpp"

// Randomly generate a gradient per vector
glm::vec2 randomGradient(int ix, int iy);
float interpolate(float s, float e, float w);
float dotGridGradient(int ix, int iy, float x, float y);
float perlin2D(float x, float y);


