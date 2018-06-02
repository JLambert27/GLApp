// data shared across entire scene
// expected to change up to once per frame
// primarily view information

#include "Scene.hpp"
#include "Terrain.hpp"
#include "Xform.inl"
#include "AppContext.hpp"

// using core modern OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// for offsetof
#include <cstddef>

#ifndef F_PI
#define F_PI 3.1415926f
#endif

//
// create and initialize view
//
Scene::Scene(GLFWwindow &win, const Terrain &terrain)
    : pan(0), tilt(-1.4f)
{
    // create uniform buffer objects
    glGenBuffers(NUM_BUFFERS, bufferIDs);
    glBindBuffer(GL_UNIFORM_BUFFER, bufferIDs[MATRIX_BUFFER]);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderData), 0, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, AppContext::SCENE_UNIFORMS,
            bufferIDs[MATRIX_BUFFER]);

    // initialize scene data
    position = vec3<float>(0,0,0);
    terrain.setHeight(position, up);
    viewport(win);
    view();
    sdata.fog = vec4<float>(1,1,1,0); // white fog, off

	sdata.normRelief.x = 0; // do not use 
	sdata.normRelief.y = 0; // do not use
}

//
// New view at position, with view defined by pan and tilt
//
void Scene::view()
{
    // update view matrix
    sdata.viewmat =
        xrotate4fp(tilt) *
        zrotate4fp(pan) *
        translate4fp(-position);

}

//
// New view at position, with view defined by pan, tilt, and up
//
void Scene::alignedView()
{
    // many ways to do this, this constructs a look-at matrix
    Vec3f viewz = vec3<float>(sinf(pan)*sinf(tilt), cosf(pan)*sin(tilt), cos(tilt));
    Vec3f viewx = normalize(up ^ viewz);
    Vec3f viewy = viewz ^ viewx;
    
    sdata.viewmat.matrix = mat4<float>(
        viewx.x, viewy.x, viewz.x, 0,
        viewx.y, viewy.y, viewz.y, 0,
        viewx.z, viewy.z, viewz.z, 0,
        0, 0, 0, 1
    );
    sdata.viewmat.inverse = mat4<float>(
        viewx.x, viewx.y, viewx.z, 0,
        viewy.x, viewy.y, viewy.z, 0,
        viewz.x, viewz.y, viewz.z, 0,
        0, 0, 0, 1
    );
    
    // combine that look-at with the translation
    sdata.viewmat = sdata.viewmat * translate4fp(-position);
}

//
// This is called when window is created or resized
// Adjust projection accordingly.
//
void Scene::viewport(GLFWwindow &win)
{
    // get window dimensions
    glfwGetFramebufferSize(&win, &width, &height);

    // this viewport makes a 1 to 1 mapping of physical pixels to GL
    // "logical" pixels
    glViewport(0, 0, width, height);

    // adjust 3D projection into this window
    sdata.projection = perspective4fp(F_PI/4, (float)width/height, 1, 10000);
}

//
// call before drawing each frame to update per-frame scene state
//
void Scene::update() const
{
    // update uniform block
    glBindBuffer(GL_UNIFORM_BUFFER, bufferIDs[MATRIX_BUFFER]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderData), &sdata);
}
