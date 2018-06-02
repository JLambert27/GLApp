// terrain data and drawing
#ifndef Terrain_hpp
#define Terrain_hpp

#include "Vec.hpp"
#include "HalfEdge.hpp"
#include "Shader.hpp"

// terrain data and rendering methods
class Terrain {
// private data
private:
    Vec3f gridSize;             // elevation grid size
    Vec3f mapSize;              // size of terrain in world space

    unsigned int numvert;       // total vertices
    Vec3f *vert;                // per-vertex position
    Vec3f *norm;                // per-vertex normal
	Vec3f *normMap;				// per-vertex normal map
    Vec2f *texcoord;            // per-vertex texture coordinate

    unsigned int numtri;        // total triangles
    unsigned int (*indices)[3]; // 3 vertex indices per triangle
    HalfEdge **triEdge;         // first edge for each triangle

    unsigned int numedge;       // total number of half edges
    HalfEdge *edge;             // array of edges


	bool normalMap; //true if we're using the normal map, updated in Input
	bool reliefMap; //true if we're using the relief map, updated in Input

    // GL vertex array object IDs
    unsigned int varrayID;

    // GL texture IDs
    enum {COLOR_TEXTURE, NORMAL_MAP_TEXTURE, NUM_TEXTURES};
    unsigned int textureIDs[NUM_TEXTURES];

    // GL buffer object IDs
    enum {POSITION_BUFFER, NORMAL_BUFFER, UV_BUFFER, INDEX_BUFFER, NORMAL_MAP_BUFFER, NUM_BUFFERS};
    unsigned int bufferIDs[NUM_BUFFERS];

    // GL shaders
    unsigned int shaderID;      // ID for shader program
    ShaderInfo shaderParts[2];  // vertex & fragment shader info

// public methods
public:
    // load terrain, given triangle size and surface texture
    Terrain(int level, int octaves);

    // clean up allocated memory
    ~Terrain();

    // load/reload shaders
    void updateShaders();

    // draw this terrain object
    void draw() const;

    // set normal and position.z at given position.xy position
    // returns true if over navigation mesh
    bool setHeight(Vec3f &position, Vec3f &normal) const;

	//toggles the use or lack of the normal map
	void toggleNormal() { normalMap = !normalMap; }

	//toggles the use or lack of the relief map
	void toggleRelief() { reliefMap = !reliefMap; }
};

#endif
