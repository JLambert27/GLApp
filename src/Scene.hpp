// data shared across entire scene
// expected to change up to once per frame
#ifndef Scene_hpp
#define Scene_hpp

#include "Xform.hpp"
#include "Vec.hpp"

struct GLFWwindow;
class Terrain;

class Scene {
// private data
private:
    // GL uniform buffer IDs
    enum {MATRIX_BUFFER, NUM_BUFFERS};
    unsigned int bufferIDs[NUM_BUFFERS];

// drirectly accessable public data
public:
    struct ShaderData {
        Xform4f viewmat, projection; // viewing matrices -- TBN is Tangent/Bitangent/Normal
        Vec4f fog;                   // fog.rgb = color, fog.a = fog enabled
		Vec4f normRelief;	// whether or not to use normal map or relief map, stored in a vector because the uniform structure is weird
    } sdata;

    int width, height;         // current window dimensions
    float pan, tilt;           // horizontal and vertical Euler angles
    Vec3f position;            // character position
    Vec3f up;                  // up vector from normal

// public methods
public:
    // create with initial window size and orbit location
    Scene(GLFWwindow &win, const Terrain &terrain);

    // set up new window viewport and projection
    void viewport(GLFWwindow &win);

    // set view using pan and tilt angles
    void view();
    
    // set view using pan, tilt, and up vector
    void alignedView();

    // update shader uniform state each frame
    void update() const;
};

#endif
