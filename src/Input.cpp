// set up and maintain view as window sizes change

#include "Input.hpp"
#include "AppContext.hpp"
#include "Scene.hpp"
#include "Terrain.hpp"
#include "Vec.inl"

// using core modern OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <math.h>

#ifndef F_PI
#define F_PI 3.1415926f
#endif

//
// set view, respecting alignview setting
//
static void setView(Scene &scene, bool alignview)
{
    if (alignview)
        scene.alignedView();
    else
        scene.view();
}


//
// called when a mouse button is pressed. 
// Remember where we were, and what mouse button it was.
//
void Input::mousePress(GLFWwindow &win, int b, int action)
{
    if (action == GLFW_PRESS) {
        // hide cursor, record button
        glfwSetInputMode(&win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        button = b;
    }
	/*
    else {
        // display cursor, update button state
        glfwSetInputMode(&win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        button = -1;       // no button
    }
	*/
}

//
// called when the mouse moves
// use difference between oldX,oldY and x,y to define a rotation
//
void Input::mouseMove(const AppContext &ctx, double x, double y)
{
    // only update view after at least one old position is stored
    if (button == GLFW_MOUSE_BUTTON_LEFT && button == oldButton) {
        // record differences & update last position
        float dx = float(x - oldX);
        float dy = float(y - oldY);

        // rotation angle, scaled so across the window = one rotation
        Scene *scene = ctx.scene;
        scene->pan += float(F_PI * dx / scene->width);
        scene->tilt += float(0.5f*F_PI * dy / scene->height);
        setView(*scene, alignview);
        redraw = true;
    }

    // update prior mouse state
    oldButton = button;
    oldX = x;
    oldY = y;
}

//
// called when any key is pressed
//
void Input::keyPress(GLFWwindow &win, AppContext &ctx, int key)
{
    switch (key) {
        case 'A':                   // move left
            strafeRate = -100.f;
            updateTime = glfwGetTime();
            redraw = true;          // need to redraw
            break;

        case 'D':                   // move right
            strafeRate = 100.f;
            updateTime = glfwGetTime();
            redraw = true;          // need to redraw
            break;

        case 'W':                   // move forward
            moveRate = 100.f;
            updateTime = glfwGetTime();
            redraw = true;          // need to redraw
            break;

        case 'S':                   // move backwards
            moveRate = -100.f;
            updateTime = glfwGetTime();
            redraw = true;          // need to redraw
            break;

        case 'R':                   // reload shaders
            ctx.terrain->updateShaders();
            redraw = true;          // need to redraw
            break;

        case 'F':                   // toggle fog on or off
            ctx.scene->sdata.fog.a = 1 - ctx.scene->sdata.fog.a;
            redraw = true;          // need to redraw
            break;

        case 'L':                   // toggle line drawing on or off
            wireframe = !wireframe;
            if (wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            redraw = true;          // need to redraw
            break;

        case 'O':                   // toggle view orientation
            alignview = !alignview;
            setView(*ctx.scene, alignview);
            redraw = true;
            break;

		case 'H':					// turn on or off Relief Map
			ctx.terrain->toggleRelief();
			ctx.scene->sdata.normRelief.y = 1 - ctx.scene->sdata.normRelief.y;
			redraw = true;
			break;

		case 'N':					// turn on or off the Normal Map
			ctx.terrain->toggleNormal();
			ctx.scene->sdata.normRelief.x = 1 - ctx.scene->sdata.normRelief.x; 
			redraw = true;
			break;
            
        case '+': case '=':         // increase number of triangles
            ++level;
            delete ctx.terrain;
            ctx.terrain = new Terrain(level, octaves);
            redraw = true;
            break;
            
        case '-': case '_':         // decrease number of triangles
            if (level > 0) --level;
            delete ctx.terrain;
            ctx.terrain = new Terrain(level, octaves);
            redraw = true;
            break;

        case '>': case '.':         // increase noise octaves
            ++octaves;
            delete ctx.terrain;
            ctx.terrain = new Terrain(level, octaves);
            redraw = true;
            break;
            
        case '<': case ',':         // decrease number of triangles
            if (octaves > 0) --octaves;
            delete ctx.terrain;
            ctx.terrain = new Terrain(level, octaves);
            redraw = true;
            break;
             
        case GLFW_KEY_ESCAPE:                    // Escape: exit
            glfwSetWindowShouldClose(&win, true);
            break;
    }
}

//
// called when any key is released
//
void Input::keyRelease(int key)
{
    switch (key) {
        case 'A': case 'D':         // stop moving left/right
            strafeRate = 0;
            break;
        case 'W': case 'S':         // stop moving forward/back
            moveRate = 0;
            break;
    }
}

//
// update view if necessary based on a/d keys
//
void Input::keyUpdate(const AppContext &ctx)
{
    if (moveRate != 0 || strafeRate != 0) {
        Scene *scene = ctx.scene;
        Terrain *terrain = ctx.terrain;

        double now = glfwGetTime();
        double dt = (now - updateTime);

        // heading based on current pan angle
        float c = cosf(scene->pan), s = sinf(scene->pan);
 
        // update based on time elapsed since last update
        // ensures uniform rate of change
        Vec3f P = scene->position, N;
        P += float(  moveRate * dt) * vec3<float>(s, c, 0);
        P += float(strafeRate * dt) * vec3<float>(c,-s, 0);
        if (terrain->setHeight(P, N)) {
            scene->position = P;
            scene->up = N;
            setView(*scene, alignview);
            redraw = true;
        }

        // remember time for next update
        updateTime = now;
    }
}
