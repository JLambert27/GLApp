// handle changes due to mouse motion, or keys
#ifndef Input_hpp
#define Input_hpp

class AppContext;
class Scene;
struct GLFWwindow;

class Input {
// private data
private:
    int button, oldButton;      // which mouse button was pressed?
    double oldX, oldY;          // location of mouse at last event

    double updateTime;          // time (in seconds) of last update
    float moveRate, strafeRate; // movement rates in units/sec

    bool wireframe;             // toggle wireframe drawing
    bool alignview;             // toggle aligning view with normal
    
// directly accessable public data
public:
    bool redraw;                // true if we need to redraw
    int level;                  // terrain levels
    int octaves;                  // terrain levels

// public methods
public:
    // initialize
    Input() : button(-1), oldButton(-1), oldX(0), oldY(0), 
        moveRate(0), strafeRate(0),
        wireframe(false), alignview(false), redraw(true),
        level(30), octaves(4) {}

    // handle mouse press / release
    void mousePress(GLFWwindow &win, int button, int action);

    // handle mouse motion
    void mouseMove(const AppContext &ctx, double x, double y);

    // handle key press
    void keyPress(GLFWwindow &win, AppContext &ctx, int key);

    // handle key release
    void keyRelease(int key);

    // update view (if necessary) based on key input
    void keyUpdate(const AppContext &ctx);
};

#endif
