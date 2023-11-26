// License: BSD unless otherwise stated.
// https://github.com/ccxvii/asstools

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define BASE64_C
#define FREE free
#define MALLOC malloc
#include "3rd_base64.h"
//
#ifdef _MSC_VER
#define strcmpi _stricmp
#else
#define strcmpi strcasecmp
#endif

int verbose = 0;
int need_to_bake_skin = 0;
int save_all_bones = 0;
int dolowprec = 0;

int dostatic = 0; // export without skeleton
int dorigid = 0; // export rigid (non-deformed) nodes as bones too
int domesh = 1; // export mesh
int doanim = 0; // export animations
int dobone = 0; // export skeleton
int doflip = 1; // export flipped (quake-style clockwise winding) triangles
int doflipUV = 0; // export flipped UVs
int doanimlist = 0; // generate list of animations with properties

int doaxis = 0; // flip bone axis from X to Y to match blender
int dounscale = 0; // remove scaling from bind pose
int dohips = 0; // reparent thighs to pelvis (see zo_hom_marche)

char *output = NULL; // output filename
char *input = NULL; // input filename

char *only_one_node = NULL;
int list_all_meshes = 0;
int list_all_positions = 0;

#define MAX_UVMAP 4
#define FIRST_UVMAP 0
#define MAX_COL 4
#define FIRST_COL 4

// We use %.9g to print floats with 9 digits of precision which
// is enough to represent a 32-bit float accurately, while still
// shortening if possible to save space for all those 0s and 1s.

#define EPSILON 0.00001
#define NEAR_0(x) (fabs((x)) < EPSILON)
#define NEAR_1(x) (NEAR_0((x)-1))
#define KILL_0(x) (NEAR_0((x)) ? 0 : (x))
#define KILL_N(x,n) (NEAR_0((x)-(n)) ? (n) : (x))
#define KILL(x) KILL_0(KILL_N(KILL_N(x, 1), -1))

#define LOWP(x) (roundf(x*32768)/32768)

int fix_hips(int verbose);
void unfix_hips(void);

static struct aiMatrix4x4 yup_to_zup = {
    1, 0, 0, 0,
    0, 0, -1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1
};

static struct aiMatrix4x4 axis_x_to_y = {
    0, 1, 0, 0,
    -1, 0, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

double aiDeterminant(struct aiMatrix4x4 *m)
{
    return (double)
        m->a1*m->b2*m->c3*m->d4 - m->a1*m->b2*m->c4*m->d3 +
        m->a1*m->b3*m->c4*m->d2 - m->a1*m->b3*m->c2*m->d4 +
        m->a1*m->b4*m->c2*m->d3 - m->a1*m->b4*m->c3*m->d2 -
        m->a2*m->b3*m->c4*m->d1 + m->a2*m->b3*m->c1*m->d4 -
        m->a2*m->b4*m->c1*m->d3 + m->a2*m->b4*m->c3*m->d1 -
        m->a2*m->b1*m->c3*m->d4 + m->a2*m->b1*m->c4*m->d3 +
        m->a3*m->b4*m->c1*m->d2 - m->a3*m->b4*m->c2*m->d1 +
        m->a3*m->b1*m->c2*m->d4 - m->a3*m->b1*m->c4*m->d2 +
        m->a3*m->b2*m->c4*m->d1 - m->a3*m->b2*m->c1*m->d4 -
        m->a4*m->b1*m->c2*m->d3 + m->a4*m->b1*m->c3*m->d2 -
        m->a4*m->b2*m->c3*m->d1 + m->a4*m->b2*m->c1*m->d3 -
        m->a4*m->b3*m->c1*m->d2 + m->a4*m->b3*m->c2*m->d1;
}

void aiInverseMatrix(struct aiMatrix4x4 *p, struct aiMatrix4x4 *m)
{
    double det = aiDeterminant(m);
    assert(det != 0.0);
    double invdet = 1.0 / det;
    p->a1= invdet * (m->b2*(m->c3*m->d4-m->c4*m->d3) + m->b3*(m->c4*m->d2-m->c2*m->d4) + m->b4*(m->c2*m->d3-m->c3*m->d2));
    p->a2=-invdet * (m->a2*(m->c3*m->d4-m->c4*m->d3) + m->a3*(m->c4*m->d2-m->c2*m->d4) + m->a4*(m->c2*m->d3-m->c3*m->d2));
    p->a3= invdet * (m->a2*(m->b3*m->d4-m->b4*m->d3) + m->a3*(m->b4*m->d2-m->b2*m->d4) + m->a4*(m->b2*m->d3-m->b3*m->d2));
    p->a4=-invdet * (m->a2*(m->b3*m->c4-m->b4*m->c3) + m->a3*(m->b4*m->c2-m->b2*m->c4) + m->a4*(m->b2*m->c3-m->b3*m->c2));
    p->b1=-invdet * (m->b1*(m->c3*m->d4-m->c4*m->d3) + m->b3*(m->c4*m->d1-m->c1*m->d4) + m->b4*(m->c1*m->d3-m->c3*m->d1));
    p->b2= invdet * (m->a1*(m->c3*m->d4-m->c4*m->d3) + m->a3*(m->c4*m->d1-m->c1*m->d4) + m->a4*(m->c1*m->d3-m->c3*m->d1));
    p->b3=-invdet * (m->a1*(m->b3*m->d4-m->b4*m->d3) + m->a3*(m->b4*m->d1-m->b1*m->d4) + m->a4*(m->b1*m->d3-m->b3*m->d1));
    p->b4= invdet * (m->a1*(m->b3*m->c4-m->b4*m->c3) + m->a3*(m->b4*m->c1-m->b1*m->c4) + m->a4*(m->b1*m->c3-m->b3*m->c1));
    p->c1= invdet * (m->b1*(m->c2*m->d4-m->c4*m->d2) + m->b2*(m->c4*m->d1-m->c1*m->d4) + m->b4*(m->c1*m->d2-m->c2*m->d1));
    p->c2=-invdet * (m->a1*(m->c2*m->d4-m->c4*m->d2) + m->a2*(m->c4*m->d1-m->c1*m->d4) + m->a4*(m->c1*m->d2-m->c2*m->d1));
    p->c3= invdet * (m->a1*(m->b2*m->d4-m->b4*m->d2) + m->a2*(m->b4*m->d1-m->b1*m->d4) + m->a4*(m->b1*m->d2-m->b2*m->d1));
    p->c4=-invdet * (m->a1*(m->b2*m->c4-m->b4*m->c2) + m->a2*(m->b4*m->c1-m->b1*m->c4) + m->a4*(m->b1*m->c2-m->b2*m->c1));
    p->d1=-invdet * (m->b1*(m->c2*m->d3-m->c3*m->d2) + m->b2*(m->c3*m->d1-m->c1*m->d3) + m->b3*(m->c1*m->d2-m->c2*m->d1));
    p->d2= invdet * (m->a1*(m->c2*m->d3-m->c3*m->d2) + m->a2*(m->c3*m->d1-m->c1*m->d3) + m->a3*(m->c1*m->d2-m->c2*m->d1));
    p->d3=-invdet * (m->a1*(m->b2*m->d3-m->b3*m->d2) + m->a2*(m->b3*m->d1-m->b1*m->d3) + m->a3*(m->b1*m->d2-m->b2*m->d1));
    p->d4= invdet * (m->a1*(m->b2*m->c3-m->b3*m->c2) + m->a2*(m->b3*m->c1-m->b1*m->c3) + m->a3*(m->b1*m->c2-m->b2*m->c1));
}

void aiComposeMatrix(struct aiMatrix4x4 *m, struct aiVector3D *scale, struct aiQuaternion *q, struct aiVector3D *pos)
{
    struct aiMatrix4x4 smat;

    aiIdentityMatrix4(m);
    m->a1 = 1.0f - 2.0f * (q->y * q->y + q->z * q->z);
    m->a2 = 2.0f * (q->x * q->y - q->z * q->w);
    m->a3 = 2.0f * (q->x * q->z + q->y * q->w);
    m->b1 = 2.0f * (q->x * q->y + q->z * q->w);
    m->b2 = 1.0f - 2.0f * (q->x * q->x + q->z * q->z);
    m->b3 = 2.0f * (q->y * q->z - q->x * q->w);
    m->c1 = 2.0f * (q->x * q->z - q->y * q->w);
    m->c2 = 2.0f * (q->y * q->z + q->x * q->w);
    m->c3 = 1.0f - 2.0f * (q->x * q->x + q->y * q->y);

    aiIdentityMatrix4(&smat);
    smat.a1 = scale->x;
    smat.b2 = scale->y;
    smat.c3 = scale->z;

    aiMultiplyMatrix4(m, &smat);

    m->a4 = pos->x; m->b4 = pos->y; m->c4 = pos->z;
}

void aiNormalizeQuaternion(struct aiQuaternion *q)
{
    const float mag = sqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
    if (mag)
    {
        const float invMag = 1.0f/mag;
        q->x *= invMag;
        q->y *= invMag;
        q->z *= invMag;
        q->w *= invMag;
    }
}

void print_matrix(struct aiMatrix4x4 *m)
{
    printf("matrix %g %g %g %g %g %g %g %g %g (det=%g)\n",
            m->a1, m->a2, m->a3,
            m->b1, m->b2, m->b3,
            m->c1, m->c2, m->c3,
            aiDeterminant(m));
}

int is_identity_matrix(struct aiMatrix4x4 *m)
{
    return
        NEAR_1(m->a1) && NEAR_0(m->a2) && NEAR_0(m->a3) &&
        NEAR_0(m->b1) && NEAR_1(m->b2) && NEAR_0(m->b3) &&
        NEAR_0(m->c1) && NEAR_0(m->c2) && NEAR_1(m->c3) &&
        NEAR_0(m->a4) && NEAR_0(m->b4) && NEAR_0(m->c4);
}

char basename[1024];

int numtags = 0;
char **taglist = NULL;

int numuntags = 0;
char *untaglist[100];

#define MAXBLEND 12
#define MIN(a,b) ((a)<(b)?(a):(b))

struct vb {
    int b[MAXBLEND];
    float w[MAXBLEND];
    int n;
};

struct material {
    struct aiMaterial *material;
    char *name;
    char *shader;
};

struct material matlist[10000];
int nummats = 0;

struct bone {
    struct aiNode *node;
    char *name;
    char *clean_name;
    int parent;
    int number; // for iqe export
    int isbone;
    int isskin;
    int isrigid;
    char *reason; // reason for selecting
    float unscale[3]; // inverse of scaling factor in bind pose

    // scratch matrices for inverse bind pose and absolute bind pose
    struct aiMatrix4x4 invpose; // inv(parent * pose)
    struct aiMatrix4x4 abspose; // (parent * pose)

    // current pose in matrix and decomposed form
    struct aiMatrix4x4 pose;
    struct aiVector3D translate;
    struct aiQuaternion rotate;
    struct aiVector3D scale;
};

struct bone bonelist[10000];
int numbones = 0;

int find_bone(char *name)
{
    int i;
    for (i = 0; i < numbones; i++)
        if (!strcmp(name, bonelist[i].name))
            return i;
    return -1;
}

char *get_base_name(char *s)
{
    char *p = strrchr(s, '/');
    if (!p) p = strrchr(s, '\\');
    if (!p) return s;
    return p + 1;
}

char *clean_node_name(char *p)
{
    static char buf[200];
    if (strstr(p, "node-") == p)
        p += 5;
    strcpy(buf, p);
    for (p = buf; *p; p++) {
        *p = tolower(*p);
        if (*p == ' ') *p = '_';
    }
    return strdup(buf); // leak like a sieve
}

char *clean_material_name(char *p)
{
    static char buf[200];
    strcpy(buf, p);
    p = strstr(buf, "-material");
    if (p) *p = 0;
    for (p = buf; *p; p++) {
        *p = tolower(*p);
        if (*p == ' ') *p = '_';
        if (*p == '#') *p = '_';
    }
    return strdup(buf); // leak like a sieve
}

char *find_material(struct aiMaterial *material)
{
    struct aiString str;
    char shader[500], *p;
    char *name;
    int i;

    for (i = 0; i < nummats; i++)
        if (matlist[i].material == material)
            return matlist[i].shader;

    aiGetMaterialString(material, AI_MATKEY_NAME, &str);
    name = str.data;

    strcpy(shader, clean_material_name(name));
    strcat(shader, "+");

    if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_DIFFUSE(0), &str))
        strcat(shader, get_base_name(str.data));
    else if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_SPECULAR(0), &str))
        strcat(shader, get_base_name(str.data));
    else if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_AMBIENT(0), &str))
        strcat(shader, get_base_name(str.data));
    else if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_EMISSIVE(0), &str))
        strcat(shader, get_base_name(str.data));
    else if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_NORMALS(0), &str))
        strcat(shader, get_base_name(str.data));
    else if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_HEIGHT(0), &str))
        strcat(shader, get_base_name(str.data));
    else if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_SHININESS(0), &str))
        strcat(shader, get_base_name(str.data));
    else if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_OPACITY(0), &str))
        strcat(shader, get_base_name(str.data));
    else if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_DISPLACEMENT(0), &str))
        strcat(shader, get_base_name(str.data));
    else if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_LIGHTMAP(0), &str))
        strcat(shader, get_base_name(str.data));
    else if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_REFLECTION(0), &str))
        strcat(shader, get_base_name(str.data));
    else
        strcat(shader, "unknown");
    p = strrchr(shader, '.');
    if (p) *p = 0;

    p = shader; while (*p) { *p = tolower(*p); p++; }

    matlist[nummats].name = name;
    matlist[nummats].material = material;
    matlist[nummats].shader = strdup(shader);
    return matlist[nummats++].shader;
}

// --- figure out which bones are part of armature ---

void build_bone_list_from_nodes(struct aiNode *node, int parent, char *clean_name)
{
    int i;

    // inherit clean names for auto-inserted nodes
    if (!strstr(node->mName.data, "$ColladaAutoName$"))
        clean_name = clean_node_name((char*)node->mName.data);

    bonelist[numbones].name = node->mName.data;
    bonelist[numbones].clean_name = clean_name;
    bonelist[numbones].parent = parent;
    bonelist[numbones].reason = "<none>";
    bonelist[numbones].isbone = 0;
    bonelist[numbones].isskin = 0;
    bonelist[numbones].isrigid = 0;
    bonelist[numbones].node = node;

    // these are set in calc_bind_pose and/or apply_initial_frame
    aiIdentityMatrix4(&bonelist[numbones].pose);
    aiIdentityMatrix4(&bonelist[numbones].abspose);
    aiIdentityMatrix4(&bonelist[numbones].invpose);

    parent = numbones++;
    for (i = 0; i < node->mNumChildren; i++)
        build_bone_list_from_nodes(node->mChildren[i], parent, clean_name);
}

void apply_initial_frame(void)
{
    int i;
    for (i = 0; i < numbones; i++) {
        // restore original transformation
        bonelist[i].pose = bonelist[i].node->mTransformation;
        // ... and update rotate/translate/scale
        aiDecomposeMatrix(&bonelist[i].pose, &bonelist[i].scale, &bonelist[i].rotate, &bonelist[i].translate);
    }
}

// recalculate abspose from local pose matrix
void calc_abs_pose(void)
{
    int i;
    for (i = 0; i < numbones; i++) {
        bonelist[i].abspose = bonelist[i].pose;
        if (bonelist[i].parent >= 0) {
            bonelist[i].abspose = bonelist[bonelist[i].parent].abspose;
            aiMultiplyMatrix4(&bonelist[i].abspose, &bonelist[i].pose);
        }
    }
}

void calc_bind_pose(void)
{
    // we now (in the single mesh / non-baking case) have our bind pose
    // our invpose is set to the inv_bind_pose matrix
    // compute forward abspose and pose matrices here
    int i;
    for (i = 0; i < numbones; i++) {
        if (bonelist[i].isskin) {
            // skinned and boned, invpose is our reference
            aiInverseMatrix(&bonelist[i].abspose, &bonelist[i].invpose);
            bonelist[i].pose = bonelist[i].abspose;
            if (bonelist[i].parent >= 0) {
                struct aiMatrix4x4 m = bonelist[bonelist[i].parent].invpose;
                aiMultiplyMatrix4(&m, &bonelist[i].pose);
                bonelist[i].pose = m;
            }
        } else {
            // not skinned, so no invpose. pose is our reference
            bonelist[i].pose = bonelist[i].node->mTransformation;
            bonelist[i].abspose = bonelist[i].pose;
            if (bonelist[i].parent >= 0) {
                bonelist[i].abspose = bonelist[bonelist[i].parent].abspose;
                aiMultiplyMatrix4(&bonelist[i].abspose, &bonelist[i].pose);
            }
            aiInverseMatrix(&bonelist[i].invpose, &bonelist[i].abspose);
        }
    }

    // compute translate/rotate/scale
    for (i = 0; i < numbones; i++)
        if (bonelist[i].isbone)
            aiDecomposeMatrix(&bonelist[i].pose, &bonelist[i].scale, &bonelist[i].rotate, &bonelist[i].translate);
}

void mark_bone_parents(int i)
{
    while (i >= 0) {
        if (!bonelist[i].isbone) {
            bonelist[i].reason = "parent";
            bonelist[i].isbone = 1;
        }
        i = bonelist[i].parent;
    }
}

void mark_tags(void)
{
    int i, k;
    for (k = 0; k < numtags; k++) {
        for (i = 0; i < numbones; i++) {
            if (!strcmp(taglist[k], bonelist[i].clean_name)) {
                fprintf(stderr, "marking tag %s\n", taglist[k]);
                bonelist[i].reason = "tagged";
                bonelist[i].isbone = 1;
                break;
            }
        }
    }
}

void unmark_tags(void)
{
    int i, k;
    for (k = 0; k < numuntags; k++) {
        for (i = 0; i < numbones; i++) {
            if (!strcmp(untaglist[k], bonelist[i].clean_name)) {
                fprintf(stderr, "unmarking tag %s\n", untaglist[k]);
                bonelist[i].reason = "untagged";
                bonelist[i].isbone = 0;
                break;
            }
        }
    }
}

void mark_skinned_bones(const struct aiScene *scene)
{
    int i, k, a, b;

    for (i = 0; i < numbones; i++) {
        struct aiNode *node = bonelist[i].node;

        if (only_one_node && strcmp(bonelist[i].clean_name, only_one_node))
            continue;

        for (k = 0; k < node->mNumMeshes; k++) {
            struct aiMesh *mesh = scene->mMeshes[node->mMeshes[k]];
            for (a = 0; a < mesh->mNumBones; a++) {
                b = find_bone(mesh->mBones[a]->mName.data);
                if (!bonelist[b].isbone) {
                    bonelist[b].reason = "skinned";
                    bonelist[b].invpose = mesh->mBones[a]->mOffsetMatrix;
                    bonelist[b].isbone = 1;
                    bonelist[b].isskin = 1;
                } else if (!need_to_bake_skin) {
                    if (memcmp(&bonelist[b].invpose, &mesh->mBones[a]->mOffsetMatrix, sizeof bonelist[b].invpose))
                        need_to_bake_skin = 1;
                }
            }
        }
    }
}

void mark_animated_bones(const struct aiScene *scene)
{
    int i, k, b;
    for (i = 0; i < scene->mNumAnimations; i++) {
        const struct aiAnimation *anim = scene->mAnimations[i];
        for (k = 0; k < anim->mNumChannels; k++) {
            b = find_bone(anim->mChannels[k]->mNodeName.data);
            bonelist[b].reason = "animated";
            bonelist[b].isbone = 1;
        }
    }
}

void mark_rigid_bones(const struct aiScene *scene)
{
    int i, k;
    for (i = 0; i < numbones; i++) {
        struct aiNode *node = bonelist[i].node;
        for (k = 0; k < node->mNumMeshes; k++) {
            struct aiMesh *mesh = scene->mMeshes[node->mMeshes[k]];
            if (mesh->mNumBones == 0 && !is_identity_matrix(&node->mTransformation)) {
                bonelist[i].isrigid = 1;
            }
        }
        if (bonelist[i].isrigid) {
            bonelist[i].reason = "rigid";
            bonelist[i].isbone = 1;
        }
    }
}

int build_bone_list(const struct aiScene *scene)
{
    int number;
    int i;

    build_bone_list_from_nodes(scene->mRootNode, -1, "SCENE");

    if (dohips) fix_hips(0);

    // we always need the bind pose
    if (doanim || domesh || dorigid)
        mark_skinned_bones(scene);

    if (doanim || save_all_bones)
        mark_animated_bones(scene);

    if (dorigid)
        mark_rigid_bones(scene);

    mark_tags(); // mark special bones named on command line as "tags" to attach stuff
    unmark_tags(); // remove named bones from list

    // select all parents of selected bones
    for (i = 0; i < numbones; i++) {
        if (bonelist[i].isbone)
            mark_bone_parents(i);
    }

    // select all otherwise 'dead' children of selected bones
    if (save_all_bones) {
        for (i = 0; i < numbones; i++) {
            if (!bonelist[i].isbone)
                if (bonelist[i].parent >= 0 && bonelist[bonelist[i].parent].isbone)
                    bonelist[i].isbone = 1;
        }
    }

    if (save_all_bones > 1) {
        for (i = 0; i < numbones; i++) {
            bonelist[i].reason = "useless";
            bonelist[i].isbone = 1;
        }
    }

    // skip root node if it has 1 child and identity transform
    int count = 0;
    for (i = 0; i < numbones; i++)
        if (bonelist[i].isbone && bonelist[i].parent == 0)
            count++;
    if (count == 1 && is_identity_matrix(&bonelist[0].node->mTransformation)) {
        bonelist[0].reason = "useless root node";
        bonelist[0].isbone = 0;
        bonelist[0].number = -1;
    }

    if (verbose)
        for (i = 0; i < numbones; i++)
            if (bonelist[i].isbone)
                fprintf(stderr, "selecting %s bone %s\n", bonelist[i].reason, bonelist[i].clean_name);

    // assign IQE numbers to bones
    number = 0;
    for (i = 0; i < numbones; i++)
        if (bonelist[i].isbone)
            bonelist[i].number = number++;

    if (dohips) unfix_hips();

    calc_bind_pose();

    return number;
}

// --- export poses and animation frames ---

void export_pm(FILE *out, int i)
{
    struct aiMatrix4x4 m = bonelist[i].pose;
    fprintf(out, "pm %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",
        KILL(m.a4), KILL(m.b4), KILL(m.c4),
        (m.a1), (m.a2), (m.a3),
        (m.b1), (m.b2), (m.b3),
        (m.c1), (m.c2), (m.c3));
}

void export_pq(FILE *out, int i)
{
    struct aiQuaternion rotate = bonelist[i].rotate;
    struct aiVector3D scale = bonelist[i].scale;
    struct aiVector3D translate = bonelist[i].translate;

    if (dolowprec) {
        if (KILL(scale.x) == 1 && KILL(scale.y) == 1 && KILL(scale.z) == 1)
            fprintf(out, "pq %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",
                    LOWP(translate.x), LOWP(translate.y), LOWP(translate.z),
                    LOWP(rotate.x), LOWP(rotate.y), LOWP(rotate.z), LOWP(rotate.w));
        else
            fprintf(out, "pq %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",
                    LOWP(translate.x), LOWP(translate.y), LOWP(translate.z),
                    LOWP(rotate.x), LOWP(rotate.y), LOWP(rotate.z), LOWP(rotate.w),
                    LOWP(scale.x), LOWP(scale.y), LOWP(scale.z));
    } else {
        if (KILL(scale.x) == 1 && KILL(scale.y) == 1 && KILL(scale.z) == 1)
            fprintf(out, "pq %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",
                    KILL(translate.x), KILL(translate.y), KILL(translate.z),
                    (rotate.x), (rotate.y), (rotate.z), (rotate.w));
        else
            fprintf(out, "pq %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",
                    KILL(translate.x), KILL(translate.y), KILL(translate.z),
                    (rotate.x), (rotate.y), (rotate.z), (rotate.w),
                    KILL(scale.x), KILL(scale.y), KILL(scale.z));
    }
}

int saved_parents[1000];

struct {
    char *name; char *parent; int parent_id;
} hiplist[] = {
    { "bip01_l_thigh", "bip01_pelvis", -1 },
    { "bip01_r_thigh", "bip01_pelvis", -1 },
//  { "bip01_l_foot", "bip01_l_calf", -1 },
//  { "bip01_r_foot", "bip01_r_calf", -1 },
    { NULL, NULL, 0 }
};

int fix_hips(int verbose)
{
    int i, k, p, fixed = 0;

    for (k = 0; hiplist[k].parent; k++)
        hiplist[k].parent_id = -1;

    for (i = 0; i < numbones; i++) {
        saved_parents[i] = bonelist[i].parent;

        for (k = 0; hiplist[k].parent; k++)
            if (!strcmp(bonelist[i].clean_name, hiplist[k].parent))
                hiplist[k].parent_id = i;

        p = bonelist[i].parent;
        for (k = 0; hiplist[k].parent; k++) {
            if (!strcmp(bonelist[i].clean_name, hiplist[k].name)) {
                if (p >= 0 && strcmp(bonelist[p].clean_name, hiplist[k].parent)) {
                    if (verbose)
                        fprintf(stderr, "fixing %s -> %s (was connected to %s)\n",
                            bonelist[i].clean_name,
                            bonelist[hiplist[k].parent_id].clean_name,
                            bonelist[p].clean_name);
                    fixed = 1;
                    bonelist[i].parent = hiplist[k].parent_id;
                }
            }
        }
    }

    return fixed;
}

void unfix_hips(void)
{
    int i;
    for (i = 0; i < numbones; i++)
        bonelist[i].parent = saved_parents[i];
}

void fix_pose(void)
{
    int i;

    calc_abs_pose();

    if (dohips) fix_hips(0);

    for (i = 0; i < numbones; i++) {
        if (bonelist[i].isbone) {
            // remove scaling factor in absolute pose
            if (dounscale < 0) {
                struct aiVector3D apos, ascale;
                struct aiQuaternion arot;
                aiDecomposeMatrix(&bonelist[i].abspose, &ascale, &arot, &apos);
                bonelist[i].unscale[0] = ascale.x;
                bonelist[i].unscale[1] = ascale.y;
                bonelist[i].unscale[2] = ascale.z;
                if (KILL(ascale.x) != 1 || KILL(ascale.y) != 1 || KILL(ascale.z) != 1)
                    fprintf(stderr, "unscaling %s: %g %g %g\n", bonelist[i].name, ascale.x, ascale.y, ascale.z);
            }
            if (dounscale) {
                float x = bonelist[i].unscale[0];
                float y = bonelist[i].unscale[1];
                float z = bonelist[i].unscale[2];
                if (KILL(x) != 1 || KILL(y) != 1 || KILL(z) != 1) {
                    bonelist[i].abspose.a1 /= x; bonelist[i].abspose.b1 /= x; bonelist[i].abspose.c1 /= x;
                    bonelist[i].abspose.a2 /= y; bonelist[i].abspose.b2 /= y; bonelist[i].abspose.c2 /= y;
                    bonelist[i].abspose.a3 /= z; bonelist[i].abspose.b3 /= z; bonelist[i].abspose.c3 /= z;
                }
            }

            // flip axis in absolute pose
            if (doaxis)
                aiMultiplyMatrix4(&bonelist[i].abspose, &axis_x_to_y);

            // ...and invert so we can recalculate the local poses
            aiInverseMatrix(&bonelist[i].invpose, &bonelist[i].abspose);

            // ...and recalculate the local pose
            bonelist[i].pose = bonelist[i].abspose;
            if (bonelist[i].parent >= 0) {
                struct aiMatrix4x4 m = bonelist[bonelist[i].parent].invpose;
                aiMultiplyMatrix4(&m, &bonelist[i].pose);
                bonelist[i].pose = m;
            }

            // ...and make sure we have it in decomposed form
            aiDecomposeMatrix(&bonelist[i].pose, &bonelist[i].scale, &bonelist[i].rotate, &bonelist[i].translate);
        }
    }

    if (dohips) unfix_hips();
}

void export_pose(FILE *out)
{
    int i;

    if (doaxis || dounscale || dohips)
        fix_pose();

    for (i = 0; i < numbones; i++)
        if (bonelist[i].isbone)
            //export_pm(out, i);
            export_pq(out, i);
}

void export_bone_list(FILE *out)
{
    int i, n;

    for (n = i = 0; i < numbones; i++) if (bonelist[i].isbone) n++;

    if (dounscale) fprintf(stderr, "removing scaling factors from bind pose\n");
    if (doaxis) fprintf(stderr, "flipping bone axis from x to y\n");

    if (dohips) {
        fprintf(stderr, "patching skeleton hierarchy\n");
        dohips = fix_hips(1);
    }

    fprintf(stderr, "exporting skeleton: %d bones\n", n);

    fprintf(out, "\n");
    for (i = 0; i < numbones; i++) {
        if (bonelist[i].isbone) {
            if (bonelist[i].parent >= 0)
                fprintf(out, "joint \"%s\" %d\n",
                    bonelist[i].clean_name,
                    bonelist[bonelist[i].parent].number);
            else
                fprintf(out, "joint \"%s\" -1\n", bonelist[i].clean_name);
        }
    }

    if (dohips)
        unfix_hips();

    fprintf(out, "\n");
    if (dounscale) dounscale = -1;
    export_pose(out);
    if (dounscale) dounscale = 1;
}

void export_static_animation(FILE *out, const struct aiScene *scene)
{
    fprintf(stderr, "exporting animation: static rest pose\n");
    fprintf(out, "\n");
    fprintf(out, "\nanimation \"%s\"\n", basename);
    fprintf(out, "framerate 30\n");
    fprintf(out, "frame\n");
    apply_initial_frame();
    export_pose(out);
}

int animation_length(const struct aiAnimation *anim)
{
    int i, len = 0;
    for (i = 0; i < anim->mNumChannels; i++) {
        struct aiNodeAnim *chan = anim->mChannels[i];
        if (chan->mNumPositionKeys > len) len = chan->mNumPositionKeys;
        if (chan->mNumRotationKeys > len) len = chan->mNumRotationKeys;
        if (chan->mNumScalingKeys > len) len = chan->mNumScalingKeys;
    }
    return len;
}

void export_frame(FILE *out, const struct aiAnimation *anim, int frame)
{
    int i;

    // start with fresh matrices
    apply_initial_frame();

    for (i = 0; i < anim->mNumChannels; i++) {
        struct aiNodeAnim *chan = anim->mChannels[i];
        int a = find_bone(chan->mNodeName.data);
        int tframe = MIN(frame, chan->mNumPositionKeys - 1);
        int rframe = MIN(frame, chan->mNumRotationKeys - 1);
        int sframe = MIN(frame, chan->mNumScalingKeys - 1);
        bonelist[a].translate = chan->mPositionKeys[tframe].mValue;
        bonelist[a].rotate = chan->mRotationKeys[rframe].mValue;
        bonelist[a].scale = chan->mScalingKeys[sframe].mValue;
#ifdef HACK_MATRIX_KEY
        bonelist[a].pose = chan->mRotationKeys[rframe].mMatrixValue;
#endif
    }

#ifndef HACK_MATRIX_KEY
    // translate/rotate/scale have changed: recompute pose
    for (i = 0; i < numbones; i++) {
        if (bonelist[i].isbone) {
            // make sure we're not hit by precision issues in decomposematrix
            aiNormalizeQuaternion(&bonelist[i].rotate);
            aiComposeMatrix(&bonelist[i].pose, &bonelist[i].scale, &bonelist[i].rotate, &bonelist[i].translate);
        }
    }
#endif

    fprintf(out, "\n");
    fprintf(out, "frame %d\n", frame);
    export_pose(out);
}

void export_animations(FILE *out, const struct aiScene *scene)
{
    int i, k, len;

    for (i = 0; i < scene->mNumAnimations; i++) {
        const struct aiAnimation *anim = scene->mAnimations[i];
        if (scene->mNumAnimations > 1)
            fprintf(out, "\nanimation \"%s,%02d\"\n", basename, i);
        else
            fprintf(out, "\nanimation \"%s\"\n", basename);
        fprintf(out, "framerate 30\n");
        len = animation_length(anim);
        fprintf(stderr, "exporting animation %d: %d frames\n", i+1, len);
        for (k = 0; k < len; k++)
            export_frame(out, anim, k);
    }

    if (scene->mNumAnimations == 0)
        export_static_animation(out, scene);
}

void export_animlist(FILE *out, const struct aiScene *scene)
{
    int i, offset=0, len;

    for (i = 0; i < scene->mNumAnimations; i++) {
        const struct aiAnimation *anim = scene->mAnimations[i];
        len = animation_length(anim)-1;
        fprintf(stderr, "frame: %d-%d %s\n", offset, offset+len, anim->mName.data);
        fprintf(out, "frame: %d-%d %s\n", offset, offset+len, anim->mName.data);
        offset += len+1;
    }

    if (scene->mNumAnimations == 0)
        fprintf(out, "frame: %s\n", "0-0 Idle");
}

/*
 * For multi-mesh models, sometimes each mesh has its own inv_bind_matrix set
 * for each bone. To export to IQE we must have only one inv_bind_matrix per
 * bone. We can bake the mesh by animating it to the initial frame.
 * Once this is done, set the inv_bind_matrix to be the inverse of the forward
 * bind_matrix of this pose.
 */

void bake_mesh_skin(const struct aiMesh *mesh)
{
    int i, k, b;
    struct aiMatrix3x3 mat3;
    struct aiMatrix4x4 bonemat[1000], mat;
    struct aiVector3D *outpos, *outnorm;

    if (mesh->mNumBones == 0)
        return;

    outpos = malloc(mesh->mNumVertices * sizeof *outpos);
    outnorm = malloc(mesh->mNumVertices * sizeof *outnorm);
    memset(outpos, 0, mesh->mNumVertices * sizeof *outpos);
    memset(outnorm, 0, mesh->mNumVertices * sizeof *outpos);

    calc_abs_pose();

    for (i = 0; i < mesh->mNumBones; i++) {
        b = find_bone(mesh->mBones[i]->mName.data);
        bonemat[i] = bonelist[b].abspose;
        aiMultiplyMatrix4(&bonemat[i], &mesh->mBones[i]->mOffsetMatrix);
    }

    for (k = 0; k < mesh->mNumBones; k++) {
        struct aiBone *bone = mesh->mBones[k];
        b = find_bone(mesh->mBones[k]->mName.data);
        mat = bonemat[k];
        mat3.a1 = mat.a1; mat3.a2 = mat.a2; mat3.a3 = mat.a3;
        mat3.b1 = mat.b1; mat3.b2 = mat.b2; mat3.b3 = mat.b3;
        mat3.c1 = mat.c1; mat3.c2 = mat.c2; mat3.c3 = mat.c3;
if(bone->mWeights)
        for (i = 0; i < bone->mNumWeights; i++) {
            struct aiVertexWeight vw = bone->mWeights[i];
            int v = vw.mVertexId;
            float w = vw.mWeight;
            struct aiVector3D srcpos = mesh->mVertices[v];
            struct aiVector3D srcnorm = mesh->mNormals[v];
            aiTransformVecByMatrix4(&srcpos, &mat);
            aiTransformVecByMatrix3(&srcnorm, &mat3);
            outpos[v].x += srcpos.x * w;
            outpos[v].y += srcpos.y * w;
            outpos[v].z += srcpos.z * w;
            outnorm[v].x += srcnorm.x * w;
            outnorm[v].y += srcnorm.y * w;
            outnorm[v].z += srcnorm.z * w;
        }
    }

    memcpy(mesh->mVertices, outpos, mesh->mNumVertices * sizeof *outpos);
    memcpy(mesh->mNormals, outnorm, mesh->mNumVertices * sizeof *outnorm);

    free(outpos);
    free(outnorm);
}

void bake_scene_skin(const struct aiScene *scene)
{
    int i;
    fprintf(stderr, "baking skin to recreate base pose in multi-mesh model\n");
    for (i = 0; i < scene->mNumMeshes; i++)
        bake_mesh_skin(scene->mMeshes[i]);
}

void export_custom_vertexarrays(FILE *out, const struct aiScene *scene)
{
    int i, t, first = 1;
    int seen[10] = {0};
    for (i = 0; i < scene->mNumMeshes; i++) {
        struct aiMesh *mesh = scene->mMeshes[i];
        for (t = 1; t < MAX_UVMAP; t++) {
            int custom = FIRST_UVMAP + t - 1;
            if (mesh->mTextureCoords[t]) {
                if (!seen[custom]) {
                    if (first) { fprintf(out, "\n"); first = 0; }
                    fprintf(out, "vertexarray custom%d float 2 \"uvmap.%d\"\n", custom, t);
                    seen[custom] = 1;
                }
            }
        }
        for (t = 1; t < MAX_COL; t++) {
            int custom = FIRST_COL + t - 1;
            if (mesh->mColors[t]) {
                if (!seen[custom]) {
                    if (first) { fprintf(out, "\n"); first = 0; }
                    fprintf(out, "vertexarray custom%d ubyte 4 \"color.%d\"\n", custom, t);
                    seen[custom] = 1;
                }
            }
        }
    }
}

/*
 * Export meshes. Group them by materials. Also apply the node transform
 * to the vertices. IQE does not have a concept of per-group transforms.
 *
 * If we are exporting a rigged model, we have to skip any meshes which
 * are not deformed by the armature. If we are exporting a non-rigged model,
 * we have to pre-transform all meshes.
 *
 * TODO: turn non-rigged meshes into rigged meshes by hooking them up to
 * a synthesized bone for its node.
 */

void export_node(FILE *out, const struct aiScene *scene, const struct aiNode *node,
    struct aiMatrix4x4 mat, char *clean_name)
{
    struct aiMatrix3x3 mat3;
    int i, a, k, t;

    aiMultiplyMatrix4(&mat, &node->mTransformation);
    mat3.a1 = mat.a1; mat3.a2 = mat.a2; mat3.a3 = mat.a3;
    mat3.b1 = mat.b1; mat3.b2 = mat.b2; mat3.b3 = mat.b3;
    mat3.c1 = mat.c1; mat3.c2 = mat.c2; mat3.c3 = mat.c3;

    if (!strstr(node->mName.data, "$ColladaAutoName$"))
        clean_name = clean_node_name((char*)node->mName.data);

    if (only_one_node && strcmp(clean_name, only_one_node))
        goto skip_mesh;

    for (i = 0; i < node->mNumMeshes; i++) {
        struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        if (mesh->mNumBones == 0 && dobone && !dorigid) {
            if (verbose)
                fprintf(stderr, "skipping rigid mesh %d in node %s (no bones)\n", i, clean_name);
            continue;
        }

        fprintf(stderr, "exporting mesh %s[%d]: %d vertices, %d faces\n",
                clean_name, i, mesh->mNumVertices, mesh->mNumFaces);

        fprintf(out, "\n");
        fprintf(out, "mesh \"%s\"\n", clean_name);

        // @r-lyeh
        if(0) fprintf(out, "material \"%s\"\n", find_material(material)); // original
        else {
            char buffer[4096] = {0};

            enum aiTextureType semantic = aiTextureType_UNKNOWN;

            struct aiString str;
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_DIFFUSE(0), &str))
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); semantic=aiTextureType_DIFFUSE; }
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_SPECULAR(0), &str))
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); semantic=aiTextureType_SPECULAR; }
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_AMBIENT(0), &str))
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); semantic=aiTextureType_AMBIENT; }
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_EMISSIVE(0), &str))
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); semantic=aiTextureType_EMISSIVE; }
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_NORMALS(0), &str))
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); semantic=aiTextureType_NORMALS; } //aiTextureType_NORMAL_CAMERA
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_HEIGHT(0), &str))
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); }
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_SHININESS(0), &str))
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); semantic=aiTextureType_SHININESS; }
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_OPACITY(0), &str))
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); }
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_DISPLACEMENT(0), &str))
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); }
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_LIGHTMAP(0), &str))
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); }
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_REFLECTION(0), &str))
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); }
            if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE(aiTextureType_UNKNOWN, 0), &str)) // AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE
                { strcat(buffer, "+"); strcat(buffer, get_base_name(str.data)); }

#if 1 // material colors
            char colorbuffer[32] = {0};
            struct aiColor4D color;
            struct aiColor4D translucentColor;
            enum aiReturn result = AI_FAILURE, result2 = AI_FAILURE;
            if( result == AI_FAILURE ) {
                result = aiGetMaterialColor( material, AI_MATKEY_COLOR_DIFFUSE, &color );
                result2 = aiGetMaterialColor( material, AI_MATKEY_COLOR_TRANSPARENT, &translucentColor );
                if (result2 == AI_SUCCESS)
                color.a = (1.0 - translucentColor.r);
                //printf("diffuse:%d, transp:%d, ", result == AI_SUCCESS, result2 == AI_SUCCESS);
            }
            if( result == AI_FAILURE ) { result = aiGetMaterialColor( material, AI_MATKEY_COLOR_REFLECTIVE, &color ); /*printf("reflective:%d, ", result == AI_SUCCESS);*/ }
            if( result == AI_FAILURE ) { result = aiGetMaterialColor( material, AI_MATKEY_COLOR_SPECULAR, &color );   /*printf("specular:%d, ", result == AI_SUCCESS);*/ }
            if( result == AI_FAILURE ) { result = aiGetMaterialColor( material, AI_MATKEY_COLOR_AMBIENT, &color );    /*printf("ambient:%d, ", result == AI_SUCCESS);*/ }
            if( result == AI_FAILURE ) { result = aiGetMaterialColor( material, AI_MATKEY_COLOR_EMISSIVE, &color );   /*printf("emissive:%d, ", result == AI_SUCCESS);*/ }
            if ( result == AI_SUCCESS ) {
                const char hex[] = "0123456789abcdef";
                // printf("rgba %f %f %f %f\n", color.r, color.g, color.b, color.a); // system("pause");
                unsigned char r = ((unsigned char)(color.r * 255)) >> 4;
                unsigned char g = ((unsigned char)(color.g * 255)) >> 4;
                unsigned char b = ((unsigned char)(color.b * 255)) >> 4;
                unsigned char a = ((unsigned char)(color.a * 255)) >> 4;
                sprintf(colorbuffer, "+$%c%c%c%c", hex[r], hex[g], hex[b], hex[a] );
            }
#endif

#if 1 // embedded textures
            char *embedded = 0;

            // look for embedded textures. referenced like *1, *2, *3... where N is texture ID
            // note: mHeight can be zero, in this case texture->pcData is not RGB values but
            // compressed JPEG/PNG/etc. data. Could use stb_image to decode such image in that case.

            unsigned tex_id = ~0u;

            if( buffer[0] ) {
                if( strchr(buffer, '*') ) {
                    tex_id = atoi(buffer+1);
                } else {
                    const char *fname = buffer + (buffer[0] == '+');

                    for( int j = 0; j < scene->mNumTextures; ++j ) {
                        struct aiTexture *tex = scene->mTextures[j];

                        const char *basename = tex->mFilename.data;
                        if( strrchr(basename, '/') ) basename = strrchr(basename, '/')+1;
                        if( strrchr(basename,'\\') ) basename = strrchr(basename,'\\')+1;

                        if( !strcmpi(basename, fname) ) {
                            tex_id = j;
                            break;
                        }
                    }
                }
            }

            if( tex_id < scene->mNumTextures ) {
                struct aiTexture *tex = scene->mTextures[tex_id];
                const char *hint = tex->achFormatHint; // "rgba8888" or "png", "bmp", etc.

                if( !tex->mHeight )
                {
                    embedded = base64_encode(tex->pcData, (int)tex->mWidth ); // @leak
                }
                else
                {
                    embedded = base64_encode(tex->pcData, (int)(tex->mWidth * tex->mHeight * sizeof(struct aiTexel))); // @leak
                }
            }
#endif

            aiGetMaterialString(material, AI_MATKEY_NAME, &str);
            fprintf(out, "material \"%s%s%s%s%s\"\n", buffer[0] == '*' ? "" : str.data, buffer, colorbuffer, embedded ? "+b64:":"", embedded ? embedded:"");
        }

        struct vb *vb = (struct vb*) malloc(mesh->mNumVertices * sizeof(*vb));
        memset(vb, 0, mesh->mNumVertices * sizeof(*vb));

        // A rigidly animated node -- insert fake blend index/weights
        if (mesh->mNumBones == 0 && dobone) {
            a = find_bone((char*)node->mName.data);
            if (verbose)
                fprintf(stderr, "\trigid bone %d for mesh in node %s (no bones)\n", bonelist[a].number, node->mName.data);
            for (k = 0; k < mesh->mNumVertices; k++) {
                vb[k].b[0] = bonelist[a].number;
                vb[k].w[0] = 1;
                vb[k].n = 1;
            }
        }

        // Assemble blend index/weight array
        for (k = 0; k < mesh->mNumBones; k++) {
            struct aiBone *bone = mesh->mBones[k];
            a = find_bone(bone->mName.data);
            for (t = 0; t < bone->mNumWeights; t++) {
                struct aiVertexWeight *w = mesh->mBones[k]->mWeights + t;
                int idx = w->mVertexId;
                if (vb[idx].n < MAXBLEND) {
                    vb[idx].b[vb[idx].n] = bonelist[a].number;
                    vb[idx].w[vb[idx].n] = w->mWeight;
                    vb[idx].n++;
                }
            }
        }

        for (k = 0; k < mesh->mNumVertices; k++) {
            struct aiVector3D vp = mesh->mVertices[k];
            if (!dobone)
                aiTransformVecByMatrix4(&vp, &mat);
            fprintf(out, "vp %.9g %.9g %.9g\n", vp.x, vp.y, vp.z);
            if (mesh->mNormals) {
                struct aiVector3D vn = mesh->mNormals[k];
                if (!dobone)
                    aiTransformVecByMatrix3(&vn, &mat3);
                fprintf(out, "vn %.9g %.9g %.9g\n", vn.x, vn.y, vn.z);
            }

            if (mesh->mTextureCoords[0]) {
                float u = mesh->mTextureCoords[0][k].x;
                float v = 1 - mesh->mTextureCoords[0][k].y;
                fprintf(out, "vt %.9g %.9g\n", u, v);
            }
            for (t = 1; t <= MAX_UVMAP; t++) {
                if (mesh->mTextureCoords[t]) {
                    float u = mesh->mTextureCoords[t][k].x;
                    float v = 1 - mesh->mTextureCoords[t][k].y;
                    fprintf(out, "v%d %.9g %.9g\n", FIRST_UVMAP+t-1, u, v);
                }
            }

            if (mesh->mColors[0]) {
                float r = mesh->mColors[0][k].r; r = floorf(r * 255) / 255;
                float g = mesh->mColors[0][k].g; g = floorf(g * 255) / 255;
                float b = mesh->mColors[0][k].b; b = floorf(b * 255) / 255;
                float a = mesh->mColors[0][k].a; a = floorf(a * 255) / 255;
                fprintf(out, "vc %.9g %.9g %.9g %.9g\n", r, g, b, a);
            }
            for (t = 1; t <= MAX_COL; t++) {
                if (mesh->mColors[t]) {
                    float r = mesh->mColors[t][k].r; r = floorf(r * 255) / 255;
                    float g = mesh->mColors[t][k].g; g = floorf(g * 255) / 255;
                    float b = mesh->mColors[t][k].b; b = floorf(b * 255) / 255;
                    float a = mesh->mColors[t][k].a; a = floorf(a * 255) / 255;
                    fprintf(out, "v%d %.9g %.9g %.9g %.9g\n", FIRST_COL+t-1, r, g, b, a);
                }
            }

            if (dobone) {
                fprintf(out, "vb");
                for (t = 0; t < vb[k].n; t++) {
                    fprintf(out, " %d %.9g", vb[k].b[t], vb[k].w[t]);
                }
                fprintf(out, "\n");
            }
        }

        for (k = 0; k < mesh->mNumFaces; k++) {
            struct aiFace *face = mesh->mFaces + k;
            if (face->mNumIndices == 3) {
                if (doflip)
                    fprintf(out, "fm %d %d %d\n", face->mIndices[2], face->mIndices[1], face->mIndices[0]);
                else
                    fprintf(out, "fm %d %d %d\n", face->mIndices[0], face->mIndices[1], face->mIndices[2]);
            } else if (face->mNumIndices == 4) {
                if (doflip)
                    fprintf(out, "fm %d %d %d %d\n", face->mIndices[3], face->mIndices[2], face->mIndices[1], face->mIndices[0]);
                else
                    fprintf(out, "fm %d %d %d %d\n", face->mIndices[0], face->mIndices[1], face->mIndices[2], face->mIndices[3]);
            } else if (face->mNumIndices > 4) {
                fprintf(stderr, "n-gon (%d) in mesh!\n", face->mNumIndices);
                int i1 = face->mIndices[0];
                int i2 = face->mIndices[1];
                for (a = 2; a < face->mNumIndices; a++) {
                    int i3 = face->mIndices[a];
                    if (doflip)
                        fprintf(out, "fm %d %d %d\n", i3, i2, i1);
                    else
                        fprintf(out, "fm %d %d %d\n", i1, i2, i3);
                    i2 = i3;
                }
            } else {
                fprintf(stderr, "skipping point/line primitive\n");
            }
        }

        free(vb);
    }

skip_mesh:

    for (i = 0; i < node->mNumChildren; i++)
        export_node(out, scene, node->mChildren[i], mat, clean_name);
}

void export_mesh_list(const struct aiScene *scene)
{
    int i, k;

    for (i = 0; i < numbones; i++) {
        struct aiNode *node = bonelist[i].node;
        for (k = 0; k < node->mNumMeshes; k++) {
            struct aiMesh *mesh = scene->mMeshes[node->mMeshes[k]];
            if (mesh->mNumBones > 0) {
                printf("%s\n", bonelist[i].clean_name);
                break;
            }
        }
    }
}

void export_position_list(const struct aiScene *scene)
{
    int i;

    calc_abs_pose();

    for (i = 0; i < numbones; i++) {
        printf("%s %g %g %g\n", bonelist[i].clean_name,
            bonelist[i].abspose.a4,
            bonelist[i].abspose.b4,
            bonelist[i].abspose.c4);
    }
}

void usage()
{
    fprintf(stderr, "usage: assiqe [options] [-o out.iqe] input.dae [tags ...]\n");
    fprintf(stderr, "\t-AA -- export all bones (including unused ones)\n");
    fprintf(stderr, "\t-A -- export all child bones\n");
    fprintf(stderr, "\t-H -- fix hierarchy (thighs <- pelvis)\n");
    fprintf(stderr, "\t-M -- print a list of meshes in scene then quit\n");
    fprintf(stderr, "\t-P -- print the positions of all poses in scene then quit\n");
    fprintf(stderr, "\t-S -- static mesh only (no skeleton)\n");
    fprintf(stderr, "\t-U -- flip UVs\n");
    fprintf(stderr, "\t-n mesh -- export only the named mesh\n");
    fprintf(stderr, "\t-a -- only export animations\n");
    fprintf(stderr, "\t-L -- export only animation list\n");
    fprintf(stderr, "\t-m -- only export mesh\n");
    fprintf(stderr, "\t-b -- bake mesh to bind pose / initial frame\n");
    fprintf(stderr, "\t-f -- export counter-clockwise winding triangles\n");
    fprintf(stderr, "\t-r -- export rigid nodes too (experimental)\n");
    fprintf(stderr, "\t-l -- low precision mode (for smaller animation files)\n");
    fprintf(stderr, "\t-x -- flip bone orientation from x to y\n");
    fprintf(stderr, "\t-s -- remove scaling from bind pose\n");
    fprintf(stderr, "\t-u -- unmark bone (force it to be excluded)\n");
    fprintf(stderr, "\t-o filename -- save output to file\n");
    exit(1);
}

#ifndef _MSC_VER
#include <getopt.h>
#else
static __declspec(thread) char* optarg = NULL;
static __declspec(thread) int optind = 1;
int getopt(int argc, char *const argv[], const char *optstring) {
    if ((optind >= argc) || (argv[optind][0] != '-') || (argv[optind][0] == 0)) {
        return -1;
    }

    int opt = argv[optind][1];
    const char *p = strchr(optstring, opt);

    if (p == NULL) {
        return '?';
    }
    if (p[1] == ':') {
        optind++;
        if (optind >= argc) {
            return '?';
        }
        optarg = argv[optind];
        optind++;
    }
    else optind++;
    return opt;
}
#endif

int main(int argc, char **argv)
{
    FILE *file;
    const struct aiScene *scene;
    char *p;
    int c;

    int onlyanim = 0;
    int onlymesh = 0;

    while ((c = getopt(argc, argv, "AHLMPSUabflmn:o:rvxsu:")) != -1) {
        switch (c) {
        case 'A': save_all_bones++; break;
        case 'H': dohips = 1; break;
        case 'M': list_all_meshes = 1; break;
        case 'P': list_all_positions = 1; break;
        case 'S': dostatic = 1; break;
case 'U': doflipUV = 1; puts("using flipUV"); break;
        case 'a': onlyanim = 1; break;
        case 'm': onlymesh = 1; break;
        case 'n': only_one_node = optarg++; break;
        case 'b': need_to_bake_skin = 1; break;
        case 'o': output = optarg++; break;
        case 'f': doflip = 0; break;
        case 'r': dorigid = 1; break;
        case 'l': dolowprec = 1; break;
        case 'L': doanimlist = 1; break;
        case 'v': verbose++; break;
        case 'x': doaxis = 1; break;
        case 's': dounscale = 1; break;
        case 'u': untaglist[numuntags++] = optarg++; break;
        default: usage(); break;
        }
    }

    if (optind == argc)
        usage();

    input = argv[optind++];

    p = strrchr(input, '/');
    if (!p) p = strrchr(input, '\\');
    if (!p) p = input; else p++;
    strcpy(basename, p);
    p = strrchr(basename, '.');
    if (p) *p = 0;

    numtags = argc - optind;
    taglist = argv + optind;

    /* Read input file and post process */
#if 0
    int flags = 0;
    flags |= aiProcess_JoinIdenticalVertices;
    flags |= aiProcess_GenSmoothNormals;
    flags |= aiProcess_GenUVCoords;
    flags |= aiProcess_TransformUVCoords;
    flags |= aiProcess_LimitBoneWeights;
    //flags |= aiProcess_FindInvalidData;
    flags |= aiProcess_ImproveCacheLocality;
    //flags |= aiProcess_RemoveRedundantMaterials;
    //flags |= aiProcess_OptimizeMeshes;
flags |= (doflipUV ? aiProcess_FlipUVs : 0);
#else
    int flags = 0
        | aiProcessPreset_TargetRealtime_MaxQuality
        | aiProcess_JoinIdenticalVertices
        | aiProcess_GenSmoothNormals
        | aiProcess_GenUVCoords
        | aiProcess_TransformUVCoords
        | aiProcess_LimitBoneWeights // #defined as AI_LMW_MAX_WEIGHTS 4
        | aiProcess_ImproveCacheLocality
        //| aiProcess_RemoveRedundantMaterials
        | aiProcess_OptimizeMeshes // aiProcess_SplitLargeMeshes
        | (doflipUV ? aiProcess_FlipUVs : 0)
        | aiProcess_OptimizeGraph
        | aiProcess_PopulateArmatureData
        //| aiProcess_FlipWindingOrder
        //| aiProcess_GenBoundingBoxes
        | aiProcess_GlobalScale // AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY
        ;

        // | aiProcess_CalcTangentSpace
        // | aiProcess_Triangulate
        // | aiProcess_SortByPType
        // | aiProcess_SplitByBoneCount // see AI_CONFIG_PP_SBBC_MAX_BONES below
#endif

    fprintf(stderr, "loading %s\n", input);
#if 0
    scene = aiImportFile(input, flags);
#else
    // flags &= ~aiProcess_PreTransformVertices;
    flags &= ~aiProcess_Debone;

    struct aiPropertyStore *aiprops = aiCreatePropertyStore();
    // build defaults
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, 1);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_READ_ALL_GEOMETRY_LAYERS, 1);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_READ_ALL_MATERIALS, 0);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_READ_MATERIALS, 1);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_READ_CAMERAS, 1);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_READ_LIGHTS, 1);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, 1);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_READ_TEXTURES, 1);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_STRICT_MODE, 0);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 1);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, 1);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_EMBEDDED_TEXTURES_LEGACY_NAMING, 0);

    // tweak: unitary bounding boxes (not working!)
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_PP_PTV_NORMALIZE, 1);

    // tweak: attempt to fix wrong number of bones (missing 10 bones or so) (not working!)
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_PP_PTV_KEEP_HIERARCHY, 1);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_PP_PTV_ADD_ROOT_TRANSFORMATION, 1);
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_PP_DB_ALL_OR_NONE, 1);
    aiSetImportPropertyFloat(aiprops, AI_CONFIG_PP_DB_THRESHOLD, 0); //

    //
    aiSetImportPropertyFloat(aiprops, AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.f);

    // tweak:
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_STRICT_MODE, 1);
    // tweak: eliminate _$AssimpFBX$ nodes
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);
    // tweak: do not remove dummies
    aiSetImportPropertyInteger(aiprops, AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, 0);
// tweak: split meshes by bone count
// aiSetImportPropertyInteger(aiprops, AI_CONFIG_PP_SBBC_MAX_BONES, 24 ); // see aiProcess_SplitByBoneCount above

    // material_chdir(input);
    scene = aiImportFileExWithProperties(input, flags, NULL, aiprops);

    aiReleasePropertyStore(aiprops);
#endif

    if (!scene) {
        fprintf(stderr, "cannot import '%s': %s\n", input, aiGetErrorString());
        exit(1);
    }

    if (scene->mNumAnimations > 0) doanim = 1;
    if (onlymesh) { domesh = 1; doanim = 0; }
    if (onlyanim) { domesh = 0; doanim = 1; }

    if (doanimlist) {
        fprintf(stderr, "exporting animation list for %s ...\n", basename);
        file = fopen(output, "w");
        if (!file) {
            fprintf(stderr, "cannot open output file: '%s'\n", output);
            exit(1);
        }
        export_animlist(file, scene);
        return 0;
    }

    if (getenv("DOANIM")) doanim = 1;

    // Convert to Z-UP coordinate system
    aiMultiplyMatrix4(&scene->mRootNode->mTransformation, &yup_to_zup);

    // Build a list of bones and compute the bind pose matrices.
    if (build_bone_list(scene) > 0)
        dobone = 1;

    if (dostatic) {
        dobone = 0;
        need_to_bake_skin = 0;
    }

    if (list_all_meshes) {
        export_mesh_list(scene);
        return 0;
    }

    if (list_all_positions) {
        export_position_list(scene);
        return 0;
    }

    // Mesh is split with incompatible bind matrices, so pick a new
    // bind pose and deform the mesh to fit.
    if (need_to_bake_skin && !onlyanim) {
        apply_initial_frame(); // ditch original bind pose
        bake_scene_skin(scene);
    }

    /*
     * Export scene as mesh/skeleton/animation
     */

    if (output) {
        fprintf(stderr, "saving %s\n", output);
        file = fopen(output, "w");
        if (!file) {
            fprintf(stderr, "cannot open output file: '%s'\n", output);
            exit(1);
        }
    } else {
        file = stdout;
    }

    fprintf(file, "# Inter-Quake Export\n");

    if (dobone) {
        export_bone_list(file);
    }

    if (domesh) {
        struct aiMatrix4x4 identity;
        aiIdentityMatrix4(&identity);
        export_custom_vertexarrays(file, scene);
        export_node(file, scene, scene->mRootNode, identity, "SCENE");
    }

    if (dobone) {
        if (doanim) {
            export_animations(file, scene);
        }
    }

    if (output)
        fclose(file);

    aiReleaseImport(scene);

    return 0;
}

