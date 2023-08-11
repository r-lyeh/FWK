/***************************************************************
    BILLIARD

    Filip Hammerstad. Spring 2004

    components: 
        -FWK
        -Tokamak Physics library(www.tokamakphysics.com)


   Revised june 2004: Texture usage removed for simplicity
   B.Stenseth

*****************************************************************/

#include "fwk.h"

#define APP_TEST
#define TOKAMAK_IMPLEMENTATION
#include "3rd_tokamak.h"

#define FLOORSIZE 40.0f

//materials
GLfloat mat_table[] = {0.0, 0.3, 0.0, 1.0};
GLfloat mat_walls[] = {0.5, 0.3, 0.1, 1.0};
GLfloat mat_ball[] = {1.0, 1.0, 1.0, 1.0};
GLfloat mat_balls[] = {1.0, 0.0, 0.0, 1.0};

GLfloat impx = 0.0;
GLfloat impy = 0.0;
GLfloat impz = 0.0;

//the simulator
neSimulator* gSim = NULL;

neV3 gravity = {0.0,0.0,0.0};   // A vector to store the direction and intensity of gravity

//the physics bodies
neRigidBody* gBall = NULL;
neRigidBody* gBalls[15] = {NULL};
neRigidBody* gStick = NULL;
neAnimatedBody* gFloor = NULL;
neAnimatedBody* gRwall = NULL;
neAnimatedBody* gLwall = NULL;
neAnimatedBody* gBwall = NULL;
neAnimatedBody* gFwall = NULL;

// function prototypes
void drawTable(int texIndex); //draws all 5 surfaces of the table(floor + 4 walls)
void drawBall(neRigidBody* localBall, float radius, int texIndex); // draws a ball
void drawStick(neRigidBody* localStick, float radius, float length, int texIndex); // draws a Stick


// Use the timer we previously initialised to determine
// how much time (in seconds, or a fraction thereof) has
// passed since this function was last called.
float GetElapsedTime()
{
    static double old = 0;
    double now = window_time();
    float rc = now - old;
    old = now;
    return rc;
}



// Initialise the Tokamak physics engine.
void appInit()
{
    neGeometry *geom; // A Geometry object which used to define the shape/size of objects
    neV3 boxSize;     // A variable to store the length, width and height of the boxes ()

    neV3 pos;         // The position
    f32 mass;         // The mass 
    neSimulatorSizeInfo sizeInfo; // Stores data about how many objects we are going to model


    // Create and initialise the simulator
    // Tell the simulator how many rigid bodies we have
    sizeInfo.rigidBodiesCount = 16;
    // Tell the simulator how many animated bodies we have
    sizeInfo.animatedBodiesCount = 5;
    // Tell the simulator how many bodies we have in total
    s32 totalBody = sizeInfo.rigidBodiesCount + sizeInfo.animatedBodiesCount;
    sizeInfo.geometriesCount = totalBody;
    // The overlapped pairs count defines how many bodies it is possible to be in collision
    // at a single time. The SDK states this should be calculated as:
    //   bodies * (bodies-1) / 2
    // So we'll take its word for it. :-)
    sizeInfo.overlappedPairsCount = totalBody * (totalBody - 1) / 2;
  
    // We're not using any of these so set them all to zero
    sizeInfo.rigidParticleCount = 0;
    sizeInfo.constraintsCount = 0;
    sizeInfo.terrainNodesStartCount = 0;

    // Set the gravity. Try changing this to see the effect on the objects
    gravity.Set(0.0f, -10.0f, 0.0f);
    

    // Ready to go, create the simulator object
    gSim = neSimulator::CreateSimulator(sizeInfo, NULL, &gravity);
    
    //set materials in the simulator object
    gSim->SetMaterial(0, 0.4, 0.5); //table material 
    gSim->SetMaterial(1, 0.4, 0.5); //ball material


    /* **** Create rigid body for white ball **** */

    gBall = gSim->CreateRigidBody();

    // Add geometry to the body and set it to be a sphere with a diameter
    geom = gBall->AddGeometry();

    geom->SetSphereDiameter(0.8f);

    geom->SetMaterialIndex(1); //rubber ball material
    // Update the bounding info of the object -- must always call this
    // after changing a body's geometry.
    gBall->UpdateBoundingInfo();

    // Set other properties of the object (mass, position, etc.)
    mass = 10.0f;

    gBall->SetInertiaTensor(neSphereInertiaTensor(0.8f, mass));

    gBall->SetMass(mass);

    gBall->SetAngularDamping(0.01f);

    // set the starting position of ball
    pos.Set(0.0, -4.6, -10.0);
    gBall->SetPos(pos);

    /* **** End creation of white ball **** */


    /* **** Create rigid body for the 15 billiard balls **** */
    int x = 0;
    for(x=0;x<15;x++)
    {
        gBalls[x] = gSim->CreateRigidBody();

        // Add geometry to the body and set it to be a sphere with a diameter
        geom = gBalls[x]->AddGeometry();

        geom->SetSphereDiameter(0.8f);

        geom->SetMaterialIndex(1); // ball 
        // Update the bounding info of the object -- must always call this
        // after changing a body's geometry.
        gBalls[x]->UpdateBoundingInfo();

        // Set other properties of the object (mass, position, etc.)
        mass = 10.0f;

        gBalls[x]->SetInertiaTensor(neSphereInertiaTensor(0.8f, mass));

        gBalls[x]->SetMass(mass);

        gBalls[x]->SetAngularDamping(0.01f);

            // set the starting position of balls 
        switch(x)
        {
        //row 1
        case 0: pos.Set(0.0, -4.6, -20.0); break;
        //row 2
        case 1: pos.Set(-0.42, -4.6, -20.82); break;
        case 2: pos.Set(0.42, -4.6, -20.82); break;
        //row 3
        case 3: pos.Set(-0.82, -4.6, -21.62); break;
        case 4: pos.Set(0.0, -4.6, -21.62); break;
        case 5: pos.Set(0.82, -4.6, -21.62); break;
        //row 4
        case 6: pos.Set(-1.22, -4.6, -22.42); break;
        case 7: pos.Set(-0.42, -4.6, -22.42); break;
        case 8: pos.Set(0.42, -4.6, -22.42); break;
        case 9: pos.Set(1.22, -4.6, -22.42); break;
        //row 5
        case 10: pos.Set(-1.62, -4.6, -23.22); break;
        case 11: pos.Set(-0.82, -4.6, -23.22); break;
        case 12: pos.Set(0.0, -4.6, -23.22); break;
        case 13: pos.Set(0.82, -4.6, -23.22); break;
        case 14: pos.Set(1.62, -4.6, -23.22); break;
        }

        gBalls[x]->SetPos(pos);
  }
    /* **** End creation of the 15 billiard balls **** */

        
    // Create an animated body for the floor and walls
    gFloor = gSim->CreateAnimatedBody();
    gRwall = gSim->CreateAnimatedBody();
    gLwall = gSim->CreateAnimatedBody();
    gBwall = gSim->CreateAnimatedBody();
    gFwall = gSim->CreateAnimatedBody();

  /* **** 
    Add geometry to the floor and walls and set each to be a box with size as defined by the FLOORSIZE constant
  **** */  
    //floor
    geom = gFloor->AddGeometry();
    boxSize.Set((FLOORSIZE/2), 1.0f, FLOORSIZE);
    geom->SetBoxSize(boxSize[0],boxSize[1],boxSize[2]);
    geom->SetMaterialIndex(0); //brick material
    gFloor->UpdateBoundingInfo();
    // Set the position of the floor within the simulator
    pos.Set(0.0, -5.5, -20.0);
    gFloor->SetPos(pos);

    //right wall
    geom = gRwall->AddGeometry();
    boxSize.Set(2.0f, 2.0f, FLOORSIZE);
    geom->SetBoxSize(boxSize[0],boxSize[1],boxSize[2]);
    geom->SetMaterialIndex(0); //brick material
    gRwall->UpdateBoundingInfo();
    // Set the position of the right wall within the simulator
    pos.Set(10.0, -4.0, -20.0);
    gRwall->SetPos(pos);

    //left wall
    geom = gLwall->AddGeometry();
    boxSize.Set(2.0f, 2.0f, FLOORSIZE);
    geom->SetBoxSize(boxSize[0],boxSize[1],boxSize[2]);
    geom->SetMaterialIndex(0); //brick material
    gLwall->UpdateBoundingInfo();
    // Set the position of the left wall within the simulator
    pos.Set(-10.0, -4.0, -20.0);
    gLwall->SetPos(pos);

    //back wall
    geom = gBwall->AddGeometry();
    boxSize.Set(FLOORSIZE/2, 2.0f, 2.0f);
    geom->SetBoxSize(boxSize[0],boxSize[1],boxSize[2]);
    geom->SetMaterialIndex(0); //brick material
    gBwall->UpdateBoundingInfo();
    // Set the position of the back wall within the simulator
    pos.Set(0.0, -4.0, -40.0);
    gBwall->SetPos(pos);

    //front wall
    geom = gFwall->AddGeometry();
    boxSize.Set(FLOORSIZE/2, 2.0f, 2.0f);
    geom->SetBoxSize(boxSize[0],boxSize[1],boxSize[2]);
    geom->SetMaterialIndex(0); //brick material
    gFwall->UpdateBoundingInfo();
    // Set the position of the front wall within the simulator
    pos.Set(0.0, -4.0, 0.0);
    gFwall->SetPos(pos);
}


void appQuit() {
    if (gSim)
    {
        // Destroy the simulator.
        // Note that this will release all related resources that we've allocated.
        neSimulator::DestroySimulator(gSim);
        gSim = NULL;
    }
}

void appTick()
{   
    float fElapsed;
    static float fLastElapsed;

    /* **** TIMER **** */

    // Find out how much time has elapsed since we were last called
    fElapsed = GetElapsedTime();

    // In order to keep passing a reasonably stable value into the simulator's Advance
    // method, we'll try to stop occasional variations in the elapsed time from occurring.
    // We'll do this by preventing the elapsed time from being more than 20% greater or
    // less than the previous elapsed time.
    if (fLastElapsed != 0)
    {
        if (fElapsed > fLastElapsed * 1.2f) fElapsed = fLastElapsed * 1.2f;
        if (fElapsed < fLastElapsed * 0.8f) fElapsed = fLastElapsed * 0.8f;
    }
    // To avoid particularly large spikes, we'll stop the elapsed time from exceeding
    // 1/45th of a second. Values larger than this can lead to an unstable simulation.
    if (fElapsed > 1.0f / 45.0f) fElapsed = 1.0f / 45.0f;
    // Store the elapsed time so that we can use it the next time around
    fLastElapsed = fElapsed;

    // Tell the Tokamak simulator to advance all its calculations by one frame.
    // Note that we pass the elapsed time here so that the simulation speed
    // is constant regardless of the refresh frequency, frame rate, etc.
    //IMPORTANT: seems this now tells the Advance() function the amount of time that
    //has passed in SECONDS(I get like 0,005 if i write fElapsed out), not in MILLISECONDS
    //like the Tokamak Documentation says (might be an old documentation). 
    //If I pass the time in ms it behaves strangely.
    gSim->Advance(fElapsed);

    /* **** END TIMER **** */
}

// DRAWING FUNCTIONS 

void appDraw() {
    drawTable(0);
    
    ddraw_color( WHITE );
    drawBall(gBall, 0.4f, 1);

    for(int x=0;x<15;x++)
    {
        ddraw_color( RED );
        drawBall(gBalls[x], 0.4f, x+2); 
    }
}

void drawTable(int texIndex)
{
    neT3 t;
    
    //all the walls
    ddraw_color(rgbaf(0.5,0.3,0.1,1));

    //right wall
    t = gRwall->GetTransform(); 
    ddraw_cube33(ptr3(&t.pos[0]), vec3(2.0, 2.0, FLOORSIZE/*+2*/), &t.rot.M[0][0]);
    
    //left wall
    t = gLwall->GetTransform(); 
    ddraw_cube33(ptr3(&t.pos[0]), vec3(2.0, 2.0, FLOORSIZE/*+2*/), &t.rot.M[0][0]);
    
    //back wall
    t = gBwall->GetTransform(); 
    ddraw_cube33(ptr3(&t.pos[0]), vec3((FLOORSIZE/2)/*+2*/, 2.0, 2.0), &t.rot.M[0][0]);
    
    //front wall
    t = gFwall->GetTransform(); 
    ddraw_cube33(ptr3(&t.pos[0]), vec3((FLOORSIZE/2)/*+2*/, 2.0, 2.0), &t.rot.M[0][0]);

    //floor
    t = gFloor->GetTransform(); 
    ddraw_color(rgbaf(0.0,0.3,0.0,1));
    ddraw_cube33(ptr3(&t.pos[0]), vec3(FLOORSIZE/2, 1.0, FLOORSIZE), &t.rot.M[0][0]);
}


void drawBall(neRigidBody* localBall, float radius, int texIndex)
{
    neT3 t;

    t = localBall->GetTransform();

    ddraw_sphere(ptr3(&t.pos[0]), radius);
}


//INPUT FUNCTIONS

void appInput()
{   
    /**/ if(input(KEY_ESC)) exit(0);
    else if(input(KEY_F5)) appInit();
    else if(input(KEY_RIGHT)) impx++;
    else if(input(KEY_LEFT)) impx--;
    else if(input(KEY_UP)) impz--;
    else if(input(KEY_DOWN)) impz++;

    if(input_up(KEY_UP) || input_up(KEY_DOWN) || input_up(KEY_LEFT) || input_up(KEY_RIGHT))
    {
        neV3  impulse;
        impulse.Set(impx*10, impy*10, impz*10);
        gBall->ApplyImpulse(impulse);

        impx = impy = impz = 0.0;
    }
}
