// Wahadlo_ML.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ode/ode.h>
#include <drawstuff/drawstuff.h>
#include <conio.h>
#include "RLearning.h"

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)
#endif

#ifdef dDOUBLE
#define dsDrawBox      dsDrawBoxD
#define dsDrawSphere   dsDrawSphereD
#define dsDrawCylinder dsDrawCylinderD
#define dsDrawCapsule  dsDrawCapsuleD
#define dsDrawLine     dsDrawLineD
#endif

static dWorldID      world;
static dSpaceID      space;
static dGeomID       ground;
static dJointGroupID contactgroup;
 float xyz[3] = {   5.0, 0.0,2.0};
static float hpr[3] = {-180.0, 0.0,0.0};
dsFunctions fn;
dJointID joint0,joint1,joint2;

//zmienne dynamiczne symulacji
dReal a,w,x,v;		//kat i przyspieszenie kąt., polozenie i przyspieszenie
 //zmienne wykorzystywane w połączeniu z RL
 stateMatrix statem;  //macierz danych potrzebnych do RL
 int act_state, action;   //zmienne do obsługi resetowania symulacji od czasu i porażek, wykonywania ruchu

typedef struct
{
    dBodyID body;
    dGeomID geom;
}	MyObject;
	MyObject podstawa,kulka,laczenie;

void keyControl(void)
{
	int cmd1;
	if (_kbhit())
	{
	cmd1 = _getch();
	switch (cmd1) {
  
  case 'k':dBodyAddRelForce(podstawa.body, 0, 150, 0);
												break;

  case 'j':dBodyAddRelForce(podstawa.body, 0, -150, 0);
													break;

  case 'a':statem.readState(x,v,a,w);							break;  	
	}
  }
	else
	{
				
				if (v < 0) dBodyAddRelForce(podstawa.body, 0, -10, 0);
				if (v > 0) dBodyAddRelForce(podstawa.body, 0, 10, 0);
	}
}

void restart(void);


void createKulka()
{
    dMass m1;
    dReal x0 = -0.05, y0 = 0.5, z0 = 3.0, radius = 0.1; 
    dReal mass = 0.001; 

    kulka.body = dBodyCreate(world);   
    dMassSetZero(&m1);
    dMassSetSphereTotal(&m1,mass,radius);
    dBodySetMass(kulka.body,&m1);             
    dBodySetPosition(kulka.body, x0, y0, z0); 

    kulka.geom = dCreateSphere(space,radius); 
    dGeomSetBody(kulka.geom,kulka.body);       
}

void createPodstawa()
{
  dMass m1;
  dReal mass = 10;
  dReal side[3] = {0.1,2,0.1};

  podstawa.body = dBodyCreate(world);
  dMassSetZero(&m1);
  dMassSetBoxTotal(&m1,mass,side[0],side[1],side[2]); 
  dBodySetMass(podstawa.body,&m1);
  dBodySetPosition(podstawa.body,-0.15,0.5,2.0); 


  podstawa.geom = dCreateBox(space,side[0],side[1],side[2]);
  dGeomSetBody(podstawa.geom,podstawa.body);
}

void createLaczenie()
{
	dMass m1;	//masa abstr.
	dReal mass = 1; //masa rzecz.
	dReal side[3] = {0.1,0.1,1}; //rozmiar

	laczenie.body = dBodyCreate(world); //przypisanie do przestrzeni nowoutworzonego obiektu
	dMassSetZero(&m1); //deklaracja pustej zmiennej do przechowywania masy
	dMassSetBoxTotal(&m1,mass,side[0],side[1],side[2]); //utworzenie rozkładu masy w określonej przestrzeni
	dBodySetMass(laczenie.body,&m1); //przypisanie ww. rozkładu do obiektu faktycznego
	dBodySetPosition(laczenie.body,-0.05,0.5,2.45); // ustawienie obiektu

	laczenie.geom = dCreateBox(space,side[0],side[1],side[2]); //stworzenie geomu
	dGeomSetBody(laczenie.geom,laczenie.body); //połączenie obiektu oraz "jego" geomu

}


	
static void nearCallback(void *data, dGeomID o1, dGeomID o2)
{
    const int N = 10;
    dContact contact[N];

    int isGround = ((ground == o1) || (ground == o2));

    int n =  dCollide(o1,o2,N,&contact[0].geom,sizeof(dContact));
    if (isGround)
    {
        for (int i = 0; i < n; i++)
        {
            contact[i].surface.mode = dContactBounce|dContactSoftERP|dContactSoftCFM;
            contact[i].surface.bounce = 0.5; // (0.0~1.0)
            contact[i].surface.soft_erp = 1.0;
            contact[i].surface.soft_cfm = 0.0;
            dJointID c = dJointCreateContact(world,contactgroup,&contact[i]);
            dJointAttach (c,dGeomGetBody(contact[i].geom.g1),
                          dGeomGetBody(contact[i].geom.g2));
        }
    }
}

void destroyBall()
{
    dBodyDestroy(kulka.body);
    dGeomDestroy(kulka.geom);
}

void destroyBox()
{
    dBodyDestroy(podstawa.body);
    dGeomDestroy(podstawa.geom);
	dBodyDestroy(laczenie.body);
    dGeomDestroy(laczenie.geom);
}

void printVel(dJointID msr_joint){

	v = dJointGetSliderPositionRate(msr_joint);
	x = dJointGetSliderPosition(msr_joint);
	xyz[1] = -x;
	dsSetViewpoint(xyz,hpr);

	//printf(" x=%-9f v=%-9f", x,v);
}
void printAngle(dJointID msr_joint) // print a position
{
    // Do not forget const to prevent an error
    
    a = dJointGetHingeAngle(msr_joint);
	w = dJointGetHingeAngleRate(msr_joint);
    // Return value is an pointer to a structure
    
    //printf(" ang=%-9f ang_div=%-9f \n",  a,w);
}

void drawObject(dGeomID gID,float red,float green,float blue)
{
    switch (dGeomGetClass(gID))
    {
    case dSphereClass:
    {
        dsSetColor(red,green,blue);
        dsDrawSphere(dGeomGetPosition(gID),dGeomGetRotation(gID),
                     dGeomSphereGetRadius(gID));
        break;
    }
    case dBoxClass:
    {
//        dReal ;
		dVector3 sides;
        dsSetColor(red,green,blue);
		dGeomBoxGetLengths(gID,sides);
        dsDrawBox(dGeomGetPosition(gID), dGeomGetRotation(gID), sides);
        
        break;
    }
    }
}
//PĘTLA GŁÓWNA
//---------------------------------------------**/**/**//
static void simLoop (int pause)
{
    if (!pause)
    {
        dSpaceCollide(space,0,&nearCallback);
        dWorldStep(world,0.01);
        dJointGroupEmpty(contactgroup);
    }

	
	printVel(joint0);
	printAngle(joint1);
	
	keyControl();						// kontrola przycisków

	act_state = statem.readState(x,v,a,w);
	if (act_state == -1 )  restart(); //koniec próby
	if (act_state == -2 ) { getchar(); statem.initState();  restart(); }
	action = statem.teachState(act_state);
	if (action == 0 ) dBodyAddRelForce(podstawa.body, 0, -30, 0); ; //ruch w lewo
	if (action == 1 ) dBodyAddRelForce(podstawa.body, 0, 30, 0); ; //ruch w prawo



    drawObject(kulka.geom,1.3,0,0);
    drawObject(podstawa.geom,0,0,1);
	drawObject(laczenie.geom,0,1,0);


	
	

}

void start()
{
    
    dsSetViewpoint (xyz,hpr);
}


void restart(void)
{
    
    dJointGroupDestroy(contactgroup);
    destroyBall();
    destroyBox();

    
    contactgroup = dJointGroupCreate(0);
    createKulka();
    createPodstawa();
	createLaczenie();

	joint0 = dJointCreateSlider(world,0);
	dJointAttach(joint0, 0 ,podstawa.body);
	dJointSetSliderAxis (joint0, 0.0, 1.0, 0.0);
    
	joint1 = dJointCreateHinge(world, 0);
    dJointAttach(joint1, podstawa.body, laczenie.body);
    dJointSetHingeAnchor(joint1, -0.1,0.5,2.0);
    dJointSetHingeAxis(joint1, 1, 0, 0);

	joint2 = dJointCreateHinge(world, 0);
    dJointAttach(joint2, kulka.body, laczenie.body);
}


void command(int cmd)
{ 
	if (cmd == 'r') restart();
}

void  prepDrawStuff()
{
    fn.version = DS_VERSION;
    fn.start   = &start;
    fn.step    = &simLoop;
    fn.command = &command;
    fn.path_to_textures = "D:";
}

int main (int argc, char **argv)
{
    prepDrawStuff();
	dInitODE();

    world = dWorldCreate();
    space = dHashSpaceCreate(0);
    contactgroup = dJointGroupCreate(0);

    dWorldSetGravity(world,0,0,-9.8);
    ground = dCreatePlane(space,0,0,1,0);

	statem.initState();

    createKulka();      
    createPodstawa();    
	createLaczenie();
	dBodySetGravityMode (podstawa.body, 0);

	joint0 = dJointCreateSlider(world,0);
	dJointAttach(joint0, 0 ,podstawa.body);
	dJointSetSliderAxis (joint0, 0.0, 1.0, 0.0);
    
	joint1 = dJointCreateHinge(world, 0);
	dJointAttach(joint1, podstawa.body, laczenie.body);
    dJointSetHingeAnchor(joint1, -0.1,0.5,2.0);
    dJointSetHingeAxis(joint1, 1, 0, 0);

	joint2 = dJointCreateHinge(world, 0);
    dJointAttach(joint2, kulka.body, laczenie.body);


   // dJointSetHingeAnchor(joint2, -0.05,0.5,1.0);
    //dJointSetHingeAxis(joint2, 0, 0, 0);
	//dJointSetFixed(joint2);

    dsSimulationLoop (argc,argv,640,480,&fn);


    dJointGroupDestroy(contactgroup);
    dSpaceDestroy(space);
    dWorldDestroy(world);
    dCloseODE();
    return 0;
}

