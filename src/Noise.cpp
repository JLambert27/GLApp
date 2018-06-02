#include "Noise.hpp"
#include "Vec.inl"
#include <math.h>

// Unreal's 3DPCG16 hash
static unsigned int hash(int x, int y) {
    Vec<unsigned int, 3> v = vec3<unsigned int>((unsigned int)(x), (unsigned int)(y), 0);
    v = v * 1664525u + 1013904223u;
    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;
    v.x += v.y*v.z;
    return v.x >> 16u;
}

// smooth fade from 1 to 0 as t goes from 0 to 1
static float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10); 
}

// linear interpolation between a and b
static float lerp(float a, float b, float t) { 
    return a + t * (b - a); 
}

// convert low two bits of hash code to gradient
static float grad(unsigned int hash, float x, float y) {
    return ((hash & 1) ? x : -x) + ((hash & 2) ? y : -y);
}

// 2D noise function
float Noise::Noise2(Vec2f v) {
    // split v into integer and fractional parts
    Vec2f vi = vec2<float>(floorf(v.x), floorf(v.y));
    Vec2f vf = v - vi;

    // smooth blend curve
    float fx = fade(vf.x), fy = fade(vf.y);

    // blend results from four corners of square
    return lerp(lerp(grad(hash(int(vi.x  ), int(vi.y  )), vf.x  , vf.y  ),
                     grad(hash(int(vi.x+1), int(vi.y  )), vf.x-1, vf.y  ),
                     fx),
                lerp(grad(hash(int(vi.x  ), int(vi.y+1)), vf.x  , vf.y-1),
                     grad(hash(int(vi.x+1), int(vi.y+1)), vf.x-1, vf.y-1),
                     fx),
                fy);
}
