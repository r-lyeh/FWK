/* Progressive Mesh type Polygon Reduction Algorithm
 *
 *   1998: Original version by Stan Melax (c) 1998
 *   Permission to use any of this code wherever you want is granted..
 *   Although, please do acknowledge authorship if appropriate.
 *
 *   2014: Code style upgraded to be more consistent with graphics/gamedev conventions. Relicensed as MIT/PD.
 *   Stan Melax: "Yes, this code can be licensed with the same license as the original. That should be fine."
 *
 *   2020: C version by Cloud Wu (c) 2020. Licensed as MIT/PD.
 */

static inline void array_find_and_remove(array(int) arr, int v) {
    for( int i = 0, end = array_count(arr); i < end; i++ )
        if( arr[i] == v ) { array_erase_fast(arr, i); --end; break; }
}

#include <assert.h>
#include <math.h>
#include <stdlib.h>

struct triangle_n {
    int vertex[3];      // the 3 points (id) that make this tri
    vec3 normal;        // unit vector othogonal to this face
};

struct vertex {
    vec3 position;          // location of point in euclidean space
    array(int) neighbor;    // adjacent vertices
    array(int) face;        // adjacent triangles
    int id;                 // place of vertex in original Array
    int collapse;           // candidate vertex (id) for collapse
    float objdist;          // cached cost of collapsing edge
};

struct mesh {
    struct vertex *v;
    struct triangle_n *t;
    int n_face;
    int n_vertex;
};

// array

static inline struct vertex *Vertex(struct mesh *M, int id) { return M->v + id; }
static inline struct triangle_n *Triangle(struct mesh *M, int id) { return M->t + id; }
static inline struct triangle_n *Face(struct mesh *M, struct vertex *v, int idx) { return M->t + v->face[idx]; }

static void AddVertex(struct mesh *M, const float *v) {
    int id = M->n_vertex++;
    struct vertex * tmp = Vertex(M, id);
    tmp->position = ptr3(v);
    tmp->neighbor = NULL;
    tmp->face = NULL;
    tmp->id = id;
    tmp->collapse = -1;
    tmp->objdist = 0;
}

static void RemoveVertex(struct mesh *M, int id) {
    struct vertex * v = Vertex(M, id);
    ASSERT(v->id == id);
    ASSERT(array_count(v->face) == 0);
    for (int i=0;i<array_count(v->face);i++) {
        struct vertex * nv = Vertex(M, v->face[i]);
        array_find_and_remove(nv->neighbor, id);
    }
    v->id = -1; // invalid vertex id
    array_free(v->neighbor);
    array_free(v->face);
}

static void ComputeNormal(struct mesh *M, struct triangle_n *t) {
    struct vertex * v0 = Vertex(M, t->vertex[0]);
    struct vertex * v1 = Vertex(M, t->vertex[1]);
    struct vertex * v2 = Vertex(M, t->vertex[2]);
    vec3 a = sub3(v1->position, v0->position);
    vec3 b = sub3(v2->position, v1->position);
    t->normal = norm3(cross3(a,b));
}

static void AddNeighbor(struct mesh *M, int vid, int id) {
    struct vertex *v = Vertex(M, vid);
    for (int i=0;i<array_count(v->neighbor);i++) {
        if (v->neighbor[i] == id)
            return;
    }
    array_push(v->neighbor, id);
}

static void AddTriangle(struct mesh *M, const int v[3]) {
    if (v[0] == v[1] || v[0] == v[2] || v[1] == v[2])
        return;
    ASSERT(v[0] < M->n_vertex);
    ASSERT(v[1] < M->n_vertex);
    ASSERT(v[2] < M->n_vertex);
    int id = M->n_face++;
    struct triangle_n * tmp = Triangle(M, id);
    tmp->vertex[0] = v[0];
    tmp->vertex[1] = v[1];
    tmp->vertex[2] = v[2];
    ComputeNormal(M, tmp);

    for(int i=0;i<3;i++) {
        struct vertex *obj = Vertex(M, v[i]);
        array_push(obj->face, id);
    }

    AddNeighbor(M, v[0], v[1]);
    AddNeighbor(M, v[0], v[2]);
    AddNeighbor(M, v[1], v[0]);
    AddNeighbor(M, v[1], v[2]);
    AddNeighbor(M, v[2], v[0]);
    AddNeighbor(M, v[2], v[1]);
}

static int HasVertex(struct triangle_n * t, int vid) {
    return (t->vertex[0] == vid || t->vertex[1] == vid || t->vertex[2] == vid);
}

static void RemoveIfNonNeighbor_(struct mesh *M, struct vertex *v, int id) {
    for (int i=0;i<array_count(v->neighbor);i++) {
        if (v->neighbor[i] == id) {
            for (int j=0;j<array_count(v->face);j++) {
                if (HasVertex(Face(M, v, j), id))
                    return;
            }
            // remove from neighbors
            array_erase_fast(v->neighbor, i);
            return;
        }
    }
}

static void RemoveIfNonNeighbor(struct mesh *M, struct vertex *v0, struct vertex *v1) {
    if (v0 == NULL || v1 == NULL)
        return;
    RemoveIfNonNeighbor_(M, v0, v1->id);
    RemoveIfNonNeighbor_(M, v1, v0->id);
}

static void RemoveTriangle(struct mesh *M, int id) {
    struct triangle_n * face = Triangle(M, id);
    struct vertex * v[3];
    for (int i=0;i<3;i++) {
        v[i] = Vertex(M, face->vertex[i]);
        if (v[i]->id < 0)
            v[i] = NULL;
        else {
            array_find_and_remove(v[i]->face, id);
        }
    }
    RemoveIfNonNeighbor(M, v[0], v[1]);
    RemoveIfNonNeighbor(M, v[1], v[2]);
    RemoveIfNonNeighbor(M, v[2], v[0]);
}

static void ReplaceVertex(struct mesh *M, int faceid, int oldid, int newid) {
    struct triangle_n * face = Triangle(M, faceid);
    ASSERT(oldid >=0 && newid >= 0);
    ASSERT(HasVertex(face, oldid));
    ASSERT(!HasVertex(face, newid));
    if(oldid==face->vertex[0]){
        face->vertex[0]=newid;
    } else if(oldid==face->vertex[1]){
        face->vertex[1]=newid;
    } else {
        face->vertex[2]=newid;
    }
    struct vertex *vold = Vertex(M, oldid);
    struct vertex *vnew = Vertex(M, newid);

    array_find_and_remove(vold->face, faceid);
    array_push(vnew->face, faceid);

    RemoveIfNonNeighbor(M, vold, Vertex(M, face->vertex[0]));
    RemoveIfNonNeighbor(M, vold, Vertex(M, face->vertex[1]));
    RemoveIfNonNeighbor(M, vold, Vertex(M, face->vertex[2]));

    AddNeighbor(M, face->vertex[0], face->vertex[1]);
    AddNeighbor(M, face->vertex[0], face->vertex[2]);
    AddNeighbor(M, face->vertex[1], face->vertex[0]);
    AddNeighbor(M, face->vertex[1], face->vertex[2]);
    AddNeighbor(M, face->vertex[2], face->vertex[0]);
    AddNeighbor(M, face->vertex[2], face->vertex[1]);

    ComputeNormal(M, face);
}

static void MeshInit(struct mesh *M, int vert_n, int tri_n) {
    M->n_face = 0;
    M->n_vertex = 0;
    M->v = (struct vertex *)MALLOC(vert_n * sizeof(struct vertex));
    M->t = (struct triangle_n *)MALLOC(tri_n * sizeof(struct triangle));
}

static void MeshFree(struct mesh *M) {
    FREE(M->v);
    FREE(M->t);
}

static float ComputeEdgeCollapseCost(struct mesh *M, struct vertex *u, int vid) {
    // if we collapse edge uv by moving u to v then how
    // much different will the model change, i.e. how much "error".
    // Texture, vertex normal, and border vertex code was removed
    // to keep this demo as simple as possible.
    // The method of determining cost was designed in order
    // to exploit small and coplanar regions for
    // effective polygon reduction.
    // Is is possible to add some checks here to see if "folds"
    // would be generated.  i.e. normal of a remaining face gets
    // flipped.  I never seemed to run into this problem and
    // therefore never added code to detect this case.
    struct vertex *v = Vertex(M, vid);
    vec3 tmp = sub3(v->position, u->position);
    float edgelength = len3(tmp);
    float curvature=0;

    // find the "sides" triangles that are on the edge uv
    array(int) sides = 0;
    for (int i = 0; i<array_count(u->face); i++) {
        if (HasVertex(Face(M, u, i), vid)) {
            array_push(sides, u->face[i]);
        }
    }
    // use the triangle facing most away from the sides
    // to determine our curvature term
    for (int i = 0; i<array_count(u->face); i++) {
        float mincurv=1; // curve for face i and closer side to it
        for (int j = 0; j<array_count(sides); j++) {
            float dotprod = dot3(Triangle(M, u->face[i])->normal,
                Triangle(M, sides[j])->normal);      // use dot product of face normals.
            float t = (1-dotprod)/2.0f;
            if (t < mincurv) {
                mincurv = t;
            }
        }
        if (mincurv > curvature)
            curvature = mincurv;
    }
    array_free(sides);
    // the more coplanar the lower the curvature term
    return edgelength * curvature;
}

static void ComputeEdgeCostAtVertex(struct mesh *M, struct vertex *v) {
    // compute the edge collapse cost for all edges that start
    // from vertex v.  Since we are only interested in reducing
    // the object by selecting the min cost edge at each step, we
    // only cache the cost of the least cost edge at this vertex
    // (in member variable collapse) as well as the value of the
    // cost (in member variable objdist).
    if (array_count(v->neighbor) == 0) {
        // v doesn't have neighbors so it costs nothing to collapse
        v->collapse=-1;
        v->objdist=-0.01f;
        return;
    }
    v->objdist = 1000000;
    v->collapse=-1;
    // search all neighboring edges for "least cost" edge
    for (int i = 0; i<array_count(v->neighbor); i++) {
        float dist = ComputeEdgeCollapseCost(M, v, v->neighbor[i]);
        if(dist<v->objdist) {
            v->collapse=v->neighbor[i];   // candidate for edge collapse
            v->objdist=dist;                    // cost of the collapse
        }
    }
}

static void ComputeAllEdgeCollapseCosts(struct mesh *M) {
    // For all the edges, compute the difference it would make
    // to the model if it was collapsed.  The least of these
    // per vertex is cached in each vertex object.
    for (int i = 0; i<M->n_vertex; i++) {
        ComputeEdgeCostAtVertex(M, Vertex(M, i));
    }
}

static void Collapse(struct mesh *M, int uid, int vid) {
    // Collapse the edge uv by moving vertex u onto v
    // Actually remove tris on uv, then update tris that
    // have u to have v, and then remove u.
    struct vertex *u = Vertex(M, uid);
    if(vid < 0) {
        // u is a vertex all by itself so just delete it
        RemoveVertex(M, uid);
        return;
    }

    array(int) tmp = 0;

    // make tmp a Array of all the neighbors of u
    for (int i = 0; i<array_count(u->neighbor); i++) {
        array_push(tmp, u->neighbor[i]);
    }
    // delete triangles on edge uv:
    for( int i = array_count(u->face); i--; ) {
        if (HasVertex(Face(M, u, i), vid)) {
            RemoveTriangle(M, u->face[i]);
        }
    }
    // update remaining triangles to have v instead of u
    for( int i = array_count(u->face); i--; ) {
        ReplaceVertex(M, u->face[i], uid, vid);
    }
    RemoveVertex(M, uid);
    // recompute the edge collapse costs for neighboring vertices
    for (int i = 0; i<array_count(tmp); i++) {
        ComputeEdgeCostAtVertex(M, Vertex(M, tmp[i]));
    }

    array_free(tmp);
}

static struct vertex *MinimumCostEdge(struct mesh *M) {
    // Find the edge that when collapsed will affect model the least.
    // This function actually returns a Vertex, the second vertex
    // of the edge (collapse candidate) is stored in the vertex data.
    // Serious optimization opportunity here: this function currently
    // does a sequential search through an unsorted Array :-(
    // Our algorithm could be O(n*lg(n)) instead of O(n*n)
    struct vertex *mn = NULL;
    for (int i = 0; i<M->n_vertex; i++) {
        struct vertex *v = Vertex(M, i);
        if (v->id >=0) {
            if (mn == NULL || v->objdist < mn->objdist) {
                mn = v;
            }
        }
    }
    return mn;
}

/*
 *  The function ProgressiveMesh() takes a model in an "indexed face
 *  set" sort of way.  i.e. Array of vertices and Array of triangles.
 *  The function then does the polygon reduction algorithm
 *  internally and reduces the model all the way down to 0
 *  vertices and then returns the order in which the
 *  vertices are collapsed and to which neighbor each vertex
 *  is collapsed to.  More specifically the returned "permutation"
 *  indicates how to reorder your vertices so you can render
 *  an object by using the first n vertices (for the n
 *  vertex version).  After permuting your vertices, the
 *  map Array indicates to which vertex each vertex is collapsed to.
 */

API void ProgressiveMesh(int vert_n, int vert_stride, const float *v, int tri_n, const int *tri, int *map, int *permutation) {
    struct mesh M;
    MeshInit(&M, vert_n, tri_n);

    // put input data into our data structures M
    const char * tmp = (const char *)v;
    for (int i=0;i<vert_n;i++, tmp += vert_stride ) {
        AddVertex(&M, (const float *)tmp);
    }

    for (int i=0;i<tri_n;i++) {
        AddTriangle(&M, &tri[i*3]);
    }

    ComputeAllEdgeCollapseCosts(&M); // cache all edge collapse costs

    for (int i = vert_n-1; i>=0; i--) {
        // get the next vertex to collapse
        struct vertex *mn = MinimumCostEdge(&M);
        // keep track of this vertex, i.e. the collapse ordering
        permutation[mn->id] = i;
        // keep track of vertex to which we collapse to
        map[i] = mn->collapse;
        // Collapse this edge
        Collapse(&M, mn->id, mn->collapse);
    }

    // reorder the map Array based on the collapse ordering
    for (int i = 0; i<vert_n; i++) {
        map[i] = (map[i]==-1)?0:permutation[map[i]];
    }
    // The caller of this function should reorder their vertices
    // according to the returned "permutation".

    MeshFree(&M);
}

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Stan Melax
 * Copyright (c) 2020 Cloud Wu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
