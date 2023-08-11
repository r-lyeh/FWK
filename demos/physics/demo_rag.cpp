// original code by david lam. BSD3 or ZLIB licensed.

#include "fwk.h"

#define APP_TEST
#define TOKAMAK_IMPLEMENTATION
#include "3rd_tokamak.h"


const s32 WALL_NUMBER = 1;

struct DemoData
{
    neV3 pos;
    neV3 boxSize;
    neV3 colour;
};

DemoData gFloor = { {0.0f,-11.0f,0.0f}, {200.0f,2.0f,200.0f}, {0.3f,0.3f,0.6f}};

#define RAG_DUDE_BOX 0
#define RAG_DUDE_SPHERE 1
#define RAG_DUDE_CYLINDER 2

struct BoneData
{
    s32 geometryType;
    f32 zRotation;
    neV3 pos;
    neV3 size;
    neV3 colour;
};

BoneData bones[] = 
{
    {RAG_DUDE_BOX,      0.0f, {0.0f, 0.0f, 0.0f}, {0.55f, 0.7f, 0.3f}, {0.8f, 0.2f, 0.2f}}, //body      
    {RAG_DUDE_SPHERE,   0.0f, {0.0f, 0.55f, 0.0f}, {0.4f, 0.35f, 0.2f}, {0.8f, 0.8f, 0.2f}}, //head 
    
    {RAG_DUDE_CYLINDER, -NE_PI / 2.0f, {-0.45f, 0.28f, 0.0f}, {0.25f, 0.4f, 0.2f}, {0.2f, 0.2f, 0.8f}}, //right arm 
    {RAG_DUDE_CYLINDER, NE_PI / 2.0f, {0.45f, 0.28f, 0.0f}, {0.25f, 0.4f, 0.2f}, {0.2f, 0.2f, 0.8f}}, //left arm

    {RAG_DUDE_BOX, -NE_PI / 2.0f, {-0.9f, 0.28f, 0.0f}, {0.24f, 0.6f, 0.24f}, {0.2f, 0.2f, 0.8f}}, //right forearm
    {RAG_DUDE_BOX, NE_PI / 2.0f, {0.9f, 0.28f, 0.0f}, {0.24f, 0.6f, 0.24f}, {0.2f, 0.2f, 0.8f}}, //left forearm

    {RAG_DUDE_CYLINDER, 0.0f, {-0.20f, -0.6f, 0.0f}, {0.27f, 0.7f, 0.2f}, {0.3f, 0.7f, 0.2f}}, //right thigh 
    {RAG_DUDE_CYLINDER, 0.0f, {0.20f, -0.6f, 0.0f}, {0.27f, 0.7f, 0.2f}, {0.3f, 0.7f, 0.2f}}, //left thigh 
    
    {RAG_DUDE_BOX, 0.0f, {-0.20f, -1.3f, 0.0f}, {0.3f, 0.8f, 0.3f}, {0.3f, 0.7f, 0.2f}}, //right leg
    {RAG_DUDE_BOX, 0.0f, {0.20f, -1.3f, 0.0f}, {0.3f, 0.8f, 0.3f}, {0.3f, 0.7f, 0.2f}}, //left leg
};

enum
{
    BONE_BODY,
    BONE_HEAD,
    BONE_RIGHT_ARM,
    BONE_LEFT_ARM,
    BONE_RIGHT_FOREARM,
    BONE_LEFT_FOREARM,
    BONE_RIGHT_THIGH,
    BONE_LEFT_THIGH,
    BONE_RIGHT_LEG,
    BONE_LEFT_LEG,
};

struct JointData
{
    s32 bodyA;
    s32 bodyB;
    neV3 pos;
    s32 type; // 0 = ball joint, 1 = hinge joint
    f32 xAxisAngle;
    f32 lowerLimit;
    f32 upperLimit;
    neBool enableLimit;
    neBool enableTwistLimit;
    f32 twistLimit;
};

JointData joints[] = 
{
    {BONE_HEAD, BONE_BODY,              {0.0f, 0.35f, 0.0f}, 1, 0.0f, -NE_PI / 4.0f, NE_PI / 4.0f, true, false, 0.0f}, //head <-> body
    
    {BONE_RIGHT_ARM, BONE_BODY,         {-0.22f, 0.28f, 0.0f}, 0, NE_PI, 0.0f, NE_PI / 2.5f, true, true, 0.1f}, //
    {BONE_LEFT_ARM, BONE_BODY,          {0.22f, 0.28f, 0.0f}, 0, 0.0f, 0.0f, NE_PI / 2.5f, true, true, 0.1f},

    {BONE_RIGHT_FOREARM, BONE_RIGHT_ARM,{-0.65f, 0.28f, 0.0f}, 1, NE_PI, 0.0f, NE_PI / 2.0f, true, false},
    {BONE_LEFT_FOREARM, BONE_LEFT_ARM,  {0.65f, 0.28f, 0.0f}, 1, 0.0f, 0.0f, NE_PI / 2.0f, true, false},

    {BONE_RIGHT_THIGH, BONE_BODY,       {-0.20f, -0.32f, 0.0f}, 0, NE_PI * 6.0f / 8.0f, 0.0f, NE_PI / 4.0f, true, true, 0.8f},
    {BONE_LEFT_THIGH, BONE_BODY,        {0.20f, -0.32f, 0.0f}, 0, NE_PI * 2.0f / 8.0f, 0.0f, NE_PI / 4.0f, true, true, 0.8f},

    {BONE_RIGHT_LEG, BONE_RIGHT_THIGH,  {-0.20f, -0.95f, 0.0f}, 1, -NE_PI * 0.5f, -NE_PI / 2.0f, 0.0f, true, false},
    {BONE_LEFT_LEG, BONE_LEFT_THIGH,    {0.20f, -0.95f, 0.0f}, 1, -NE_PI * 0.5f, -NE_PI / 2.0f, 0.0f, true, false},
};

const s32 N_DUDE = 20;
const s32 BONES_PER_DUDE = 10;
const s32 JOINTS_PER_DUDE = 9;
const s32 N_BODY = BONES_PER_DUDE * N_DUDE;
const s32 N_STEP = 40;

uint64_t perfReport;

class CSampleRagDude
{
public:
    neSimulator * sim;
    neAllocatorDefault allocator;

    CRenderPrimitive groundRender;
    neAnimatedBody * ground;

    neRigidBody * rigidBodies[N_BODY];
    CRenderPrimitive rigidBodiesRender[N_BODY];

    neAnimatedBody * steps[N_STEP];
    CRenderPrimitive stepsRender[N_STEP];

    CSampleRagDude() : sim(NULL) {};

    void CreateSimulator();
    
    void CreateGround();

    void CreateSteps();

    void CreateRagDude(neV3 position, s32 & index);

    void Reset();

    void Cleanup();
};

CSampleRagDude sample;

void CSampleRagDude::CreateSimulator()
{
    if (sim)
    {
        neSimulator::DestroySimulator(sim);

        sim = NULL;
    }
    // creat the physics simulation

    neSimulatorSizeInfo sizeInfo;

    sizeInfo.rigidBodiesCount = N_BODY;
    sizeInfo.animatedBodiesCount = WALL_NUMBER + N_STEP;
    sizeInfo.geometriesCount = N_BODY + WALL_NUMBER + N_STEP;
    sizeInfo.constraintsCount = N_BODY * JOINTS_PER_DUDE;
    s32 totalBody = N_BODY + WALL_NUMBER;
    sizeInfo.overlappedPairsCount = totalBody * (totalBody - 1) / 2;
    neV3 gravity; gravity.Set(0.0f, -9.0f, 0.0f);
    { //dont need any of these
        sizeInfo.rigidParticleCount = 0;
        //sizeInfo.terrainNodesStartCount = 0;
    }

    sim = neSimulator::CreateSimulator(sizeInfo, &allocator, &gravity);
}

void CSampleRagDude::CreateGround()
{
    ground = sim->CreateAnimatedBody();

    neGeometry * geom = ground->AddGeometry();   

    geom->SetBoxSize(gFloor.boxSize);

    ground->UpdateBoundingInfo();

    ground->SetPos(gFloor.pos);

    groundRender.SetGraphicBox(gFloor.boxSize[0], gFloor.boxSize[1], gFloor.boxSize[2]);
}

void CSampleRagDude::CreateSteps()
{
    for (s32 i = 0; i < N_STEP; i++)
    {
        steps[i] = sim->CreateAnimatedBody();

        neGeometry * geom = steps[i]->AddGeometry();

        const f32 stepHeight = 0.5f;//1.2f;

        const f32 stepDepth = 0.5f;//1.2f;

        neV3 stepSize; stepSize.Set(30.0f, stepHeight, stepDepth);

        geom->SetBoxSize(stepSize);

        steps[i]->UpdateBoundingInfo();

        neV3 pos;

        pos.Set(0.0f, 8.0f - stepHeight * i, (-1.0f + stepDepth * i));

        steps[i]->SetPos(pos);

        stepsRender[i].SetGraphicBox(30.0f, stepHeight, stepDepth);
    }
}

void CSampleRagDude::CreateRagDude(neV3 position, s32 & index)
{
//      const f32 groundLevel = -10.0f;

    s32 cur = 0;

    f32 scale = 1.0f;

    for (s32 i = 0; i < BONES_PER_DUDE; i++)
    {
        cur = index + i;

        rigidBodies[cur] = sim->CreateRigidBody();

        rigidBodies[cur]->CollideConnected(true);

        neV3 inertiaTensor;

        f32 mass;
        
        mass = 8.0f;

        if (i == 0)
            mass = 20.0f;
        else if (i == 8 || i ==9)
            mass = 8.0f;

        neGeometry * geom = rigidBodies[cur]->AddGeometry();

        switch (bones[i].geometryType)
        {
        case RAG_DUDE_BOX:
            
            geom->SetBoxSize(bones[i].size[0] * scale, bones[i].size[1] * scale, bones[i].size[2] * scale);

            inertiaTensor = neBoxInertiaTensor(bones[i].size[0]* scale, 
                                                    bones[i].size[1]* scale, 
                                                    bones[i].size[2]* scale, mass);

            rigidBodiesRender[cur].SetGraphicBox(bones[i].size[0] * scale, bones[i].size[1] * scale, bones[i].size[2] * scale);

            break;

        case RAG_DUDE_SPHERE:

            geom->SetSphereDiameter(bones[i].size[0] * scale);

            inertiaTensor = neSphereInertiaTensor(bones[i].size[0]* scale, mass);

            rigidBodiesRender[cur].SetGraphicSphere(bones[i].size[0] * scale * 0.5f);

            break;

        case RAG_DUDE_CYLINDER:

            geom->SetCylinder(bones[i].size[0] * scale, bones[i].size[1] * scale);

            inertiaTensor = neCylinderInertiaTensor(bones[i].size[0], bones[i].size[1], mass);

            rigidBodiesRender[cur].SetGraphicCylinder(bones[i].size[0] * scale * 0.5f, bones[i].size[1] * scale);
            
            break;
        }

        rigidBodiesRender[cur].SetDiffuseColor(vec4(bones[i].colour[0],
                                                        bones[i].colour[1],
                                                        bones[i].colour[2], 1));

        rigidBodies[cur]->UpdateBoundingInfo();

        rigidBodies[cur]->SetInertiaTensor(inertiaTensor);

        rigidBodies[cur]->SetMass(mass);

        neV3 pos;
        
        pos = bones[i].pos * scale+ position;

        rigidBodies[cur]->SetPos(pos);

        neQ quat;

        neV3 zAxis; zAxis.Set(0.0f, 0.0f, 1.0f);

        quat.Set(bones[i].zRotation, zAxis);
        
        rigidBodies[cur]->SetRotation(quat);

        rigidBodies[cur]->SetSleepingParameter(0.5f); //make it easier to sleep
    }
    neJoint * joint;
        
    neT3 jointFrame;

    for (s32 i = 0; i < JOINTS_PER_DUDE; i++)
    {
        joint = sim->CreateJoint(rigidBodies[joints[i].bodyA + index], rigidBodies[joints[i].bodyB + index]);

        jointFrame.SetIdentity();

        jointFrame.pos = joints[i].pos * scale + position;
            
        if (joints[i].type == 0)
        {
            joint->SetType(neJoint::NE_JOINT_BALLSOCKET);

            neQ q;

            neV3 zAxis; zAxis.Set(0.0f, 0.0f, 1.0f);

            q.Set(joints[i].xAxisAngle, zAxis);

            jointFrame.rot = q.BuildMatrix3();
        }
        else
        {
            joint->SetType(neJoint::NE_JOINT_HINGE);

            if (i == 3)
            {
                jointFrame.rot[0].Set(1.0f, 0.0f, 0.0f);
                jointFrame.rot[1].Set(0.0f, 1.0f, 0.0f);
                jointFrame.rot[2].Set(0.0f, 0.0f, 1.0f);
            }
            else if (i == 4)
            {
                jointFrame.rot[0].Set(-1.0f, 0.0f, 0.0f);
                jointFrame.rot[1].Set(0.0f, -1.0f, 0.0f);
                jointFrame.rot[2].Set(0.0f, 0.0f, 1.0f);
            }
            else
            {
                jointFrame.rot[0].Set(0.0f, 0.0f, -1.0f);
                jointFrame.rot[1].Set(-1.0f, 0.0f, 0.0f);
                jointFrame.rot[2].Set(0.0f, 1.0f, 0.0f);
            }
        }

        joint->SetJointFrameWorld(jointFrame);
        
        if (i == 5|| i == 6) // right
        {
            neT3 body2w = rigidBodies[joints[i].bodyB+index]->GetTransform();

            neT3 w2Body = body2w.FastInverse();

            neM3 m;

            neQ q1, q2;

            neV3 zAxis; zAxis.Set(0.0f, 0.0f, 1.0f);

            q1.Set(joints[i].xAxisAngle, zAxis);

            neV3 xAxis; xAxis.Set(1.0f, 0.0f, 0.0f);

            q2.Set(-NE_PI * 0.30f, xAxis);

            neQ q; q = q2 * q1;

            m = q.BuildMatrix3();

            neT3 frame2body;

            frame2body.rot = w2Body.rot * m;

            frame2body.pos = w2Body * jointFrame.pos;

            //neT3 frame2w = body2w * frame2body;

            joint->SetJointFrameB(frame2body);
        }
        joint->SetLowerLimit(joints[i].lowerLimit);

        joint->SetUpperLimit(joints[i].upperLimit);

        if (joints[i].enableLimit)
            joint->EnableLimit(true);

        if (joints[i].enableTwistLimit)
        {
            joint->SetLowerLimit2(joints[i].twistLimit);

            joint->EnableLimit2(true);
        }

        joint->SetIteration(4);
        
        joint->Enable(true);
    }
    index = cur+1;
}

void CSampleRagDude::Reset()
{
    Cleanup();

    CreateSimulator();

    CreateGround();

    CreateSteps();

    neV3 position;

    position.SetZero();

    s32 i = 0;

    for (s32 j = 0; j < N_DUDE; j++)
    {
        position.Set(-5 + (j % 5) * 2.0f, 11.0f + 4.0f * (j / 5), 0.0f);

        CreateRagDude(position, i); 
    }
}

void CSampleRagDude::Cleanup()
{
    sample.groundRender.mMesh.Destroy();

    for (s32 i = 0; i < N_STEP; i++)
    {
        sample.stepsRender[i].mMesh.Destroy();
    }

    for (s32 i = 0; i < N_BODY; i++)
    {
        sample.rigidBodiesRender[i].mMesh.Destroy();
    }

    neSimulator::DestroySimulator(sample.sim);

    sim = NULL;
}

void appInit()
{
    sample.Reset();
};

void appTick()
{
    f32 t = 1.0f / 30.0f;//(f32)delta / 1000.0f;

    //sim->Advance(TIME_STEP, 1, &g_PerfReport);
    perfReport = -time_us();
    sample.sim->Advance(t, 1.0f / 60.0f, 1.0f/ 30.0f, NULL); // perfReport can be NULL
    perfReport += time_us();

    neV3 v = sample.rigidBodies[0]->GetPos();
    camera_lookat(&cam, vec3(v[0],v[1],v[2]));
}

void appDraw()
{
    // stats
    f32 processtime_ms = perfReport / 1000.f;
    f32 percentage =  processtime_ms / 1000.f * 60.0f; // for a 60Hz game
    s32 mem; sample.sim->GetMemoryAllocated(mem);
    window_title(va("Tokamak(RagDude) | physics time %.3f ms, %.0f%% frame budget, allocated: %dk", processtime_ms, percentage * 100.0f, mem/1024));
    if (counter.QuadPart == 100)
    {
        //sample.perfReport.Reset();
        counter.QuadPart = 0;
    }



    neT3 t;
    
    t = sample.ground->GetTransform();

    //t.MakeD3DCompatibleMatrix();

    sample.groundRender.Render(NULL, &t);

    for (s32 i = 0; i < N_STEP; i++)
    {
        t = sample.steps[i]->GetTransform();

        //t.MakeD3DCompatibleMatrix();

        sample.stepsRender[i].Render(NULL, &t);
    }

    for (s32 i = 0; i < N_BODY; i++)
    {
        t = sample.rigidBodies[i]->GetTransform();

        //t.MakeD3DCompatibleMatrix();

        sample.rigidBodiesRender[i].Render(NULL, &t);
    }
}

void appQuit()
{
    // SAFE_RELEASE( g_pEffect );

    sample.Cleanup();
}

void appInput()
{
    if( input_down(KEY_F5) )
        sample.Reset();

    {   
        neV3 vel;
        vel.Set(3 * (input(KEY_RIGHT) - input(KEY_LEFT)),10 * input(KEY_SPACE),-3 * (input(KEY_UP) - input(KEY_DOWN)));

        int actOnBody = 0;
        if( vel.Length() > 0 ) sample.rigidBodies[actOnBody]->SetVelocity(vel);
    }
}
