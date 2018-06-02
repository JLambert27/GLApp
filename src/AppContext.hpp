// collected state for access in callbacks
#ifndef AppContext_hpp
#define AppContext_hpp

class AppContext {
// directly acccessable public data
public:
    class Scene *scene;         // viewing data
    class Input *input;         // user interface data
    class Terrain *terrain;     // terrain geometry

    // uniform matrix block indices
    enum { SCENE_UNIFORMS, NUM_UNIFORMS };

    // initialize all pointers to NULL to allow delete in destructor
    AppContext() : scene(0), input(0), terrain(0) {}

    // clean up any context data
    ~AppContext();
};

// load set of shaders
#endif
