// simple function to read a PPM and return the size and date
// ppm because they're so easy to read and write without external libraries

// ppm files start with header lines, separated by white space (space,
// tab, return, etc). The header is "P6 <width> <height> <maxvalue>"
// after the header, there is one more white-space character, then
// raw uncompressed byte data in [height][width][rgb] order

// it would be cleaner to throw/catch errors, but they just print & exit

#include "ImagePPM.hpp"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>

// OpenGL, just for loadTexture
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
// don't complain if we use standard IO functions instead of windows-only
#pragma warning( disable: 4996 )
#endif

//
// create from file
//
ImagePPM::ImagePPM(const char *name)
{
    // open file in project data directory
    std::string path = std::string(PROJECT_DATA_DIR) + name;
    FILE *fp = fopen(path.c_str(), "rb");
    assert(fp);

    // check that "magic number" at beginning of file is P6
    if (fgetc(fp) != 'P' || fgetc(fp) != '6') {
        fprintf(stderr, "unknown image format %s\n", path.c_str());
        assert(false);
    }

    // the following really needs some more error checking...
    // read image size, maximum value, and blank following header
    fscanf(fp, " #%*[^\n]");    // skip comment (if there)
    fscanf(fp, "%d %d", &width, &height); // read image size
    fscanf(fp, " #%*[^\n]");    // skip comment (if there)
    fscanf(fp, "%*d");          // skip max value
    fgetc(fp);                  // skip final \n before data
    assert(width > 0 && height > 0);

    // check remaining file size matches image size
    // if this fails, you may have checked a ppm file out
    // as text rather than binary
    long headerEnd = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long fileEnd = ftell(fp);
    fseek(fp, headerEnd, SEEK_SET);
    assert(fileEnd - headerEnd == width*height*3);

    // allocate image and read array
    image = new color_type[width * height];
    fread(image, sizeof(color_type), width * height, fp);

    // done!
    fclose(fp);
}

//
// create empty image given size
//
ImagePPM::ImagePPM(unsigned int w, unsigned int h)
    : width(w), height(h), image(new color_type[w*h])
{}

//
// write image as PPM
//
void ImagePPM::write(const char *name) const
{
    // open file
    std::string path = std::string(PROJECT_BASE_DIR) + name;
    FILE *fp = fopen(path.c_str(), "wb");
    assert(fp);

    // write header then data
    fprintf(fp, "P6\n%d %d\n255\n", width, height);
    fwrite(image, sizeof(color_type), width * height, fp);

    // close file
    fclose(fp);
}

void ImagePPM::loadTexture(unsigned int bufferID) const
{
    glBindTexture(GL_TEXTURE_2D, bufferID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
            GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
}
