// 2D Perline noise
#ifndef Noise_hpp
#define Noise_hpp

#include "Vec.hpp"

class Noise {
public:
    // 2D Modified Noise function
    // range approximately -0.5 to 0.5
    // approximately 1/2 - 1 cycle per unit change in v
    // as a static function, call as Noise::Noise2()
    // used a class rather than namespace in case I want to add data later
    static float Noise2(Vec2f v);
};

#endif
