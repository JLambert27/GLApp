    // draw a simple terrain height field

#include "Terrain.hpp"
#include "AppContext.hpp"
#include "ImagePPM.hpp"
#include "Noise.hpp"
#include "Vec.inl"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <stdio.h>

// enable half-edge search
#define HALF_EDGE 1

#if HALF_EDGE
////////////////////////////////////////////////////////////////////////
// for half-edge construction, map from pair of vertices to edge index
struct EdgeKey {
    int v0, v1;

    EdgeKey() : v0(-1), v1(-1) {}
    EdgeKey(int a, int b) : v0(a), v1(b) {}

    bool operator==(const EdgeKey &b) const
    {
        return v0 == b.v0 && v1 == b.v1;
    }
};

namespace std {
    template <>
    struct hash<EdgeKey> {
        std::size_t operator()(const EdgeKey &e) const
        {
            return std::hash<int>()(std::hash<int>()(e.v0) ^ e.v1);
        }
    };
}

typedef std::unordered_map<EdgeKey, int> EdgeMap;

// add pair of half edges into edge map
// assumes edge array is big enough to hold all the edges
// links them together, adds the vertices and returns index to v0 half
// face and next are left uninitialized
static int addEdge(HalfEdge *edge, EdgeMap &edgeMap, int v0, int v1)
{
    EdgeKey key0(v0, v1), key1(v1, v0);
    int edge0, edge1;

    // look for or insert first edge in map
    auto half0 = edgeMap.find(key0);
    if (half0 == edgeMap.end()) {
        edge0 = edgeMap.size();
        edgeMap[key0] = edge0;
        edge[edge0].edge = edge0;
        edge[edge0].vert = v0;
    }
    else {
        edge0 = half0->second;
    }

    // look for or insert second edge in map
    auto half1 = edgeMap.find(key1);
    if (half1 == edgeMap.end()) {
        edge1 = edgeMap.size();
        edgeMap[key1] = edge1;
        edge[edge1].edge = edge1;
        edge[edge1].vert = v1;
    }
    else {
        edge1 = half1->second;
    }

    // make sure edges connect
    edge[edge0].pair = &edge[edge1];
    edge[edge1].pair = &edge[edge0];

    return edge0;
}
#endif

//////////////////
// build terrain

static Vec3f elevation(Vec3f P, int octaves)
{
    float s = 1;
    for(int i=0; i < octaves; ++i, s *= 2) {
        P.z += Noise::Noise2(s*P.xy) / s;
    }
    return P;
}

//
// load the terrain data
//
Terrain::Terrain(int level, int octaves)
	: normalMap(false), reliefMap(false)
{
    // buffer objects to be used later
    glGenTextures(NUM_TEXTURES, textureIDs);
    glGenBuffers(NUM_BUFFERS, bufferIDs);
    glGenVertexArrays(1, &varrayID);

    // load color image into a named texture
	ImagePPM textureImage("pebbles.ppm");
	textureImage.loadTexture(textureIDs[COLOR_TEXTURE]);
	ImagePPM normalTextureImage("pebbles-norm.ppm");
	normalTextureImage.loadTexture(textureIDs[NORMAL_MAP_TEXTURE]);

    // convenient size of coordinates, and size the whole world should appear
    gridSize = vec3<float>(level+1, level+1, 1);
    mapSize = vec3<float>(300, 300, 100);

    // number of vertices: 1, 1+6, 1+6+12: 1 + 6*sum(i)
    numvert = 1 + 3*(level+1)*(level+2);
    vert = new Vec3f[numvert];
    norm = new Vec3f[numvert];
	normMap = new Vec3f[numvert];
    texcoord = new Vec2f[numvert];

    int idx = 0;
    // increasing number of vertices from top row to middle
    for(int y=0;  y <= level+1;  ++y) {
        float rowY = sqrtf(0.75) * (y - float(level+1));
        for(int x=-y-level-1; x<=y+level+1; x += 2, ++idx) {
            vert[idx] = elevation(vec3<float>(0.5 * x, rowY, 0) / gridSize, octaves) * mapSize;
        }
    }

    // decreasing number of vertices from middle to bottom
    for(int y = level;  y >= 0;  --y) {
        float rowY = sqrtf(0.75) * (float(level+1) - y);
        for(int x=-y-level-1; x<=y+level+1; x += 2, ++idx) {
            vert[idx] = elevation(vec3<float>(0.5 * x, rowY, 0) / gridSize, octaves) * mapSize;
        }
    }

    // texture coordinate from position
    for(int i=0; i<numvert; ++i) {
        texcoord[i] = (vert[i].xy / mapSize.xy) * 0.5f + 0.5f;
        norm[i] = vec3<float>(0,0,0);

		//normal map coordinate also from position
		ImagePPM::color_type newColor = textureImage(texcoord[i].x, texcoord[i].y);
		normMap[i].x = ((float)newColor.x) / 256. * 2 - 1;
		normMap[i].y = ((float)newColor.y) / 256. * 2 - 1;
		normMap[i].z = ((float)newColor.z) / 256. * 2 - 1;

		if (i == 0)
		{
			printf("%f, %f, %f", normMap[i].x, normMap[i].y, normMap[i].z);
		}
    }

    // number of triangles: 6, 6*4, 6*9: 6*level^2
    numtri = 6 * ((level + 2)*level + 1);
    indices = new unsigned int[numtri][3];

    // increasing number of triangles from top to middle
    idx = 0;
    unsigned int toprow = 0, bottomrow = toprow + level + 2;
    for(int y=0; y<=level; ++y) {
        // upward pointing triangles
        for(int x=0; x <= y+level+1; ++x, ++idx) {
            indices[idx][0] = bottomrow + x + 1;
            indices[idx][1] = bottomrow + x;
            indices[idx][2] = toprow + x;
        }

        // downward pointing triangles
        for(int x=0; x <= y+level; ++x, ++idx) {
            indices[idx][0] = toprow + x;
            indices[idx][1] = toprow + x + 1;
            indices[idx][2] = bottomrow + x + 1;
        }
        toprow = bottomrow;
        bottomrow += level + y + 3;
    }

    // decreasing number of triangles from top to middle
    for(int y = level; y >= 0; --y) {
        // downward pointing triangles
        for(int x=0; x <= y+level+1; ++x, ++idx) {
            indices[idx][0] = toprow + x;
            indices[idx][1] = toprow + x + 1;
            indices[idx][2] = bottomrow + x;
        }

        // upward pointing triangles
        for(int x=0; x <= y+level; ++x, ++idx) {
            indices[idx][0] = toprow + x + 1;
            indices[idx][1] = bottomrow + x + 1;
            indices[idx][2] = bottomrow + x;
        }
        toprow = bottomrow;
        bottomrow += level + y + 2;
    }

    // compute face normals and sum into each vertex normal
    for(int i=0; i < numtri; ++i) {
        int i0 = indices[i][0], i1 = indices[i][1], i2 = indices[i][2];
        Vec3f v0 = vert[i0], v1 = vert[i1], v2 = vert[i2];
        Vec3f faceNorm = (v1 - v0) ^ (v2 - v0);
        faceNorm = normalize(faceNorm);

        norm[i0] += faceNorm; 
        norm[i1] += faceNorm;
        norm[i2] += faceNorm;
    }

    // renormalize normal array
    for(int i=0; i < numvert; ++i) {
        norm[i] = normalize(norm[i]);
		normMap[i] = normalize(normMap[i]);
    }

    // load vertex and index array to GPU
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[POSITION_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, numvert*sizeof(Vec3f), vert, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[NORMAL_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, numvert*sizeof(Vec3f), norm, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[NORMAL_MAP_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, numvert * sizeof(Vec3f), normMap, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[UV_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, numvert*sizeof(Vec2f), texcoord, 
            GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIDs[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
            numtri*sizeof(unsigned int[3]), indices, GL_STATIC_DRAW);

    // initial shader load
    shaderParts[0].id = glCreateShader(GL_VERTEX_SHADER);
    shaderParts[0].file = "terrain.vert";
    shaderParts[1].id = glCreateShader(GL_FRAGMENT_SHADER);
    shaderParts[1].file = "terrain.frag";
    shaderID = glCreateProgram();
    updateShaders();

    printf("level %d: %d triangle terrain, %d octaves\n", level, numtri, octaves);

#if HALF_EDGE
    ////////
    // build half-edge data

    // three half-edges for each triangle, plus one for each boundary edge
    numedge = 3 * numtri + 6 * (level + 1);
    triEdge = new HalfEdge*[numtri];
    edge = new HalfEdge[numedge];
    EdgeMap edgeMap;

    // add edges for each face
    for (int i=0; i < numtri; ++i) {
        int v0 = indices[i][0];
        int v1 = indices[i][1];
        int v2 = indices[i][2];

        int e0 = addEdge(edge, edgeMap, v0, v1);
        int e1 = addEdge(edge, edgeMap, v1, v2);
        int e2 = addEdge(edge, edgeMap, v2, v0);

        // connect edges around face
        edge[e0].next = &edge[e1];
        edge[e1].next = &edge[e2];
        edge[e2].next = &edge[e0];

        // connect to face
        edge[e0].face = edge[e1].face = edge[e2].face = i;
        triEdge[i] = &edge[e0];
    }
#endif
}

//
// Delete terrain data
//
Terrain::~Terrain()
{
    glDeleteShader(shaderParts[0].id);
    glDeleteShader(shaderParts[1].id);
    glDeleteProgram(shaderID);
    glDeleteTextures(NUM_TEXTURES, textureIDs);
    glDeleteBuffers(NUM_BUFFERS, bufferIDs);
    glDeleteVertexArrays(1, &varrayID);

    delete[] indices;
    delete[] texcoord;
    delete[] norm;
	delete[] normMap;
    delete[] vert;

#if HALF_EDGE
    delete[] triEdge;
    delete[] edge;
#endif
}

//
// load (or replace) terrain shaders
//
void Terrain::updateShaders()
{
    loadShaders(shaderID, sizeof(shaderParts)/sizeof(*shaderParts), 
            shaderParts);
    glUseProgram(shaderID);

    // (re)connect view and projection matrices
    glUniformBlockBinding(shaderID, 
            glGetUniformBlockIndex(shaderID,"SceneData"),
            AppContext::SCENE_UNIFORMS);

    // map shader name for texture to glActiveTexture number used in draw
    glUniform1i(glGetUniformLocation(shaderID, "colorTexture"), 0);

	// map shader name for normal map to glActiveTexture number used in draw
	glUniform1i(glGetUniformLocation(shaderID, "normalTexture"), 0);

    // re-connect attribute arrays
    glBindVertexArray(varrayID);

    GLint positionAttrib = glGetAttribLocation(shaderID, "vPosition");
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[POSITION_BUFFER]);
    glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionAttrib);

    GLint normalAttrib = glGetAttribLocation(shaderID, "vNormal");
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[NORMAL_BUFFER]);
    glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(normalAttrib);

	GLint normalMapAttrib = glGetAttribLocation(shaderID, "vNormalMap");
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[NORMAL_MAP_BUFFER]);
	glVertexAttribPointer(normalMapAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(normalMapAttrib);

    GLint uvAttrib = glGetAttribLocation(shaderID, "vUV");
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[UV_BUFFER]);
    glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(uvAttrib);
}

//
// this is called every time the terrain needs to be redrawn 
//
void Terrain::draw() const
{
    // enable shaders
    glUseProgram(shaderID);

    // enable vertex arrays
    glBindVertexArray(varrayID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureIDs[COLOR_TEXTURE]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureIDs[NORMAL_MAP_TEXTURE]);

    // draw the triangles for each three indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIDs[INDEX_BUFFER]);
    glDrawElements(GL_TRIANGLES, 3*numtri, GL_UNSIGNED_INT, 0);
}

// return barycentric coordinates for P relative to v0/v1/v2 triangle
static Vec3f barycentric(Vec2f P, Vec2f v0, Vec2f v1, Vec2f v2) {
    // common terms in calculation
    float t0 = v1.x * v2.y - v1.y * v2.x;
    float t1 = v2.x * v0.y - v2.y * v0.x;
    float t2 = v0.x * v1.y - v0.y * v1.x;
    float d = 1. / (t0 + t1 + t2);

    return d * vec3<float>(
        P.x * (v1.y - v2.y) + P.y * (v2.x - v1.x) + t0,
        P.x * (v2.y - v0.y) + P.y * (v0.x - v2.x) + t1,
        P.x * (v0.y - v1.y) + P.y * (v1.x - v0.x) + t2
    );
}

//
// set viewer height at given xy position
// returns true if over navigation mesh
//
bool Terrain::setHeight(Vec3f &P, Vec3f &N) const {
#if HALF_EDGE
    static int prevFace = 0;

    for(int i = prevFace; i >= 0; ) {
        int i0 = indices[i][0];
        int i1 = indices[i][1];
        int i2 = indices[i][2];

        Vec3f v0 = vert[i0];
        Vec3f v1 = vert[i1];
        Vec3f v2 = vert[i2];

        Vec3f bary = barycentric(P.xy, v0.xy, v1.xy, v2.xy);

        // found our triangle
        if (bary.x >= 0 && bary.y >= 0 && bary.z >= 0) {
            prevFace = i;

            // update the z
            P.z = bary.x * v0.z + bary.y * v1.z + bary.z * v2.z;
            P.z += 10; // viewer height above terrain

            // set normal
            N = bary.x * norm[i0] + bary.y * norm[i1] + bary.z * norm[i2];

            // return success
            return true;
        }

        // find a negative edge and try to cross it
        if (bary.z < 0)
            i = triEdge[i]->pair->face;
        else if (bary.x < 0)
            i = triEdge[i]->next->pair->face;
        else if (bary.y < 0)
            i = triEdge[i]->next->next->pair->face;
    }

#else
    for(int i=0; i < numtri; ++i) {
        Vec3f v0 = vert[indices[i][0]];
        Vec3f v1 = vert[indices[i][1]];
        Vec3f v2 = vert[indices[i][2]];

        Vec3f bary = barycentric(P.xy, v0.xy, v1.xy, v2.xy);
        if (bary.x < 0 || bary.y < 0 || bary.z < 0) continue;

        // update the z
        P.z = bary.x * v0.z + bary.y * v1.z + bary.z * v2.z;
        P.z += 10; // viewer height above terrain

        // set normal
        N = bary.x * norm[indices[i][0]] + bary.y * norm[indices[i][1]] + bary.z * norm[indices[i][2]];

        // return success
        return true;
    }
#endif

    return false;
}


