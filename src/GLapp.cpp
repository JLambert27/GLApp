//
// Simple GL example
//


#include "AppContext.hpp"
#include "Input.hpp"
#include "Scene.hpp"
#include "Terrain.hpp"

// using core modern OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <assert.h>

///////
// Clean up any context data
AppContext::~AppContext()
{
    // if any are NULL, deleting a NULL pointer is OK
    delete scene;
    delete input;
    delete terrain;
}

///////
// GLFW callbacks must use extern "C"
extern "C" {

    //
    // called for GLFW error
    //
    void winError(int error, const char *description)
    {
        fprintf(stderr, "GLFW error: %s\n", description);
    }

    //
    // called whenever the window size changes
    //
    void reshape(GLFWwindow *win, int width, int height)
    {
        AppContext *appctx = (AppContext*)glfwGetWindowUserPointer(win);
        assert(appctx);

        appctx->scene->viewport(*win);
        appctx->input->redraw = true;
    }

    //
    // called when mouse button is pressed
    //
    void mousePress(GLFWwindow *win, int button, int action, int mods)
    {
        AppContext *appctx = (AppContext*)glfwGetWindowUserPointer(win);
        assert(appctx);

        appctx->input->mousePress(*win, button, action);
    }

    //
    // called when mouse is moved
    //
    void mouseMove(GLFWwindow *win, double x, double y)
    {
        AppContext *appctx = (AppContext*)glfwGetWindowUserPointer(win);
        assert(appctx);

        appctx->input->mouseMove(*appctx, x,y);
    }

    // 
    // called on any keypress
    //
    void keyPress(GLFWwindow *win, int key, int scancode, int action, int mods)
    {
        AppContext *appctx = (AppContext*)glfwGetWindowUserPointer(win);
        assert(appctx);

        if (action == GLFW_PRESS)
            appctx->input->keyPress(*win, *appctx, key);
        else if (action == GLFW_RELEASE)
            appctx->input->keyRelease(key);
    }
}

// initialize GLFW - windows and interaction
GLFWwindow *initGLFW(AppContext &appctx)
{
    // set error callback before init
    glfwSetErrorCallback(winError);
    int ok = glfwInit();
    assert(ok);

    // OpenGL version: YOU MAY NEED TO ADJUST VERSION OR OPTIONS!
    // When figuring out the settings that will work for you, make
    // sure you can see error messages on console output.
    //
    // My driver needs FORWARD_COMPAT, but others will need to comment that out.
    // Likely changes for other versions:
    //       OpenGL 4.0 (2010): configured for this already
    //       OpenGL 3.3 (2010): change VERSION_MAJOR to 3, MINOR to 3
    //         change "400 core" to "330 core" in the .vert and .frag files
    //       OpenGL 3.2 (2009): change VERSION_MAJOR to 3, MINOR to 2
    //         change "400 core" to "150 core" in the .vert and .frag files
    //       OpenGL 3.1 (2009): change VERSION_MAJOR to 3, MINOR to 1
    //         comment out PROFILE line
    //         change "400 core" to "140" in the .vert and .frag files
    //       OpenGL 3.0 (2008): does not support features we need
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // ask for a window with dimensions 843 x 480 (HD 480p)
    GLFWwindow *win = glfwCreateWindow(1200, 800, "Simple OpenGL Application", 0, 0);
    assert(win);

    glfwMakeContextCurrent(win);

    // use GLEW on windows to access modern OpenGL functions
    glewExperimental = true;
    glewInit();

    // store context pointer to access application data
    glfwSetWindowUserPointer(win, &appctx);

    // set callback functions to be called by GLFW
    glfwSetFramebufferSizeCallback(win, reshape);
    glfwSetKeyCallback(win, keyPress);
    glfwSetMouseButtonCallback(win, mousePress);
    glfwSetCursorPosCallback(win, mouseMove);

    // set OpenGL state
    glEnable(GL_DEPTH_TEST);      // tell OpenGL to handle overlapping surfaces

    return win;
}

int main(int argc, char *argv[])
{
    // collected data about application for use in callbacks
    AppContext appctx;

    // set up GLFW and OpenGL
    GLFWwindow *win = initGLFW(appctx);
    assert(win);

    // initialize context (after GLFW)
    appctx.input = new Input;
    appctx.terrain = new Terrain(appctx.input->level, appctx.input->octaves);
    appctx.scene = new Scene(*win, *appctx.terrain);

    // loop until GLFW says it's time to quit
    while (!glfwWindowShouldClose(win)) {
        // check for continuous key updates to view
        appctx.input->keyUpdate(appctx);

        if (appctx.input->redraw) {
            // we're handing the redraw now
            appctx.input->redraw = false;

            // clear old screen contents
            glClearColor(
                    appctx.scene->sdata.fog.r,
                    appctx.scene->sdata.fog.g,
                    appctx.scene->sdata.fog.b,
                    1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // draw something
            appctx.scene->update();
            appctx.terrain->draw();

            // show what we drew
            glfwSwapBuffers(win);
        }

        // wait for user input
        glfwPollEvents();
    }

    glfwDestroyWindow(win);
    glfwTerminate();

    return 0;
}
