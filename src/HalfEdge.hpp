// half edge data structure
#ifndef HalfEdge_hpp
#define HalfEdge_hpp

// half edge as simple struct data structure
// data structure invariants:
//   pair must exist so edge->vertex and edge->pair->vertex define the edge
//   next must exist, so edge->next walks around face and edge->next->pair around vertex
//   vertex must be a valid vertex index >= 0
//   face exists inside mesh (>=0), but not at mesh borders, where face == -1
struct HalfEdge {
    int edge;       // consistency check of edge index
    int vert;       // index of vertex associated with this half edge
    int face;       // index of face for this half edge
    HalfEdge *pair; // other half of the edge
    HalfEdge *next; // next edge around the face counter-clockwise

    HalfEdge() : edge(-1), vert(-1), face(-1), pair(0), next(0) {}
};
#endif
