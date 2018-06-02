: set environment variables for visual studio to find GLEW and GLFW libraries
: based on directory containing this script
:    %0 is the path and name of the script
:    %~dp0 is just the path
: add the glew and glfw directories to that
: SETX sets user environment variables to match
: Run BEFORE opening visual studio

SETX GLEWDIR "%~dp0glew-2.1.0"
SETX GLFW_LOCATION "%~dp0glfw-3.2.1"

