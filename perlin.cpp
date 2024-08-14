#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include<perlin.hpp>
#include<iostream>

// Randomly generate a gradient per vector
glm::vec2 randomGradient(int ix, int iy) {
    // No precomputed gradients mean this works for any number of grid coordinates
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a = ix, b = iy;
    a *= 3284157443;

    b ^= a << s | a >> w - s;
    b *= 1911520717;

    a ^= b << s | b >> w - s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

    // Create the vector from the angle
    glm::vec2 v;
    v.x = sin(random);
    v.y = cos(random);

    return v;
}

float interpolate(float s, float e, float w)
{
    return (e - s) * (3.0 - w * 2.0) * w * w + s; // Cubic interploation
}

float dotGridGradient(int ix, int iy, float x, float y)
{
    glm::vec2 gradient = randomGradient(ix, iy);

    float dx = x - (float)ix;
    float dy = y - (float)iy;

    return (dx * gradient.x + dy * gradient.y);
}

float perlin2D(float x, float y)
{

    // Retrieve current grid start
    int xi = (int)x;
    int yi = (int)y;

    // Retrieve current grid end
    int xe = xi + 1;
    int ye = yi + 1;

    float weightX = x - (float)xi;
    float weightY = y - (float)yi;

    // Top two corners
    float tx = dotGridGradient(xi, yi, x, y);
    float ty = dotGridGradient(xe, yi, x, y);
    float int0 = interpolate(tx, ty, weightX);

    // Bottom two corners
    float bx = dotGridGradient(xi, ye, x, y);
    float by = dotGridGradient(xe, ye, x, y);
    float int1 = interpolate(bx, by, weightX);

    return interpolate(int0, int1, weightY);
}


