#include "stdafx.h"
#include "RLearning.h"
#include <ode/ode.h>
#include <drawstuff/drawstuff.h>
#include <math.h>




static const	int state_count = 162; //liczba możliwych stanów
static const	int steps_max	= 10000; //liczba maks. kroków w jednej próbie
static const	int fails_max	= 1000;	//liczba maks. porażek
static const	float lambdaw	= 0.9;  //współ. zapominania główny
static const	float lambdav	= 0.8;  //współ. zapominania pomocniczy
static const	float alpha		= 1000; //współ. uczenia główny 
static const	float beta		= 0.5;  //współ. uczenia pomocniczy

	float w[state_count];
	float v[state_count];
	float e[state_count];
	float xbar[state_count];

int move, state;
float x, oldp, q1, q2, q3, z, y;

float random(void) {
	
	q1 = rand() ;
	q2 = ((float) ((1 << 31)));
	q3 = q1/q2 -1;
	//printf("%f \n", q3);
	return(
			q3
		) ;
};

float push(float s) {
	
	if (s > 50) s = 50;
	if (s < -50) s = -50;
	q1 = s;
	q2 = 1 + exp(s);
	q3 = (1/(1 + exp(s)));
	//printf("%f \n", q3);
	return s; //uprościć może całość?
		

		
};










//przelicznik wagowy




//granica porażki
dReal porazka_x			= 10;
dReal porazka_v			= 1;
dReal porazka_ang		= 0.2094384; //20stopni


//granice przedziałów
dReal lim_x1	= 5;
dReal lim_ang1	= 0.0174532 + 3.14; //1stopien
dReal lim_ang2	= 0.1047192 + 3.14; //	6 stopni
dReal lim_ang_div = 0.87266; //50st/sek








int stateMatrix::readState (dReal &x, dReal &v, dReal &ang, dReal &ang_div)
{

{
  state=0;
  steps++;
  //printf("%d \n",steps);
  
  //porażka
	
  if (steps > 500)
  {
  if (x < -porazka_x || x > porazka_x ||ang < -porazka_ang || ang > porazka_ang) 
	{
		fails++; 
		steps = 0; 
	  
		return(-1);  
	
	}
  }


	//limit prób lub kroków
	if (fails > 1000 || steps_max > 10000) return(-2);

  if (x < -lim_x1)  		       state = 0;
  else if (x < 0.8)     	       state = 1;
  else		    	               state = 2;

  if (v < -0.5) 		         state =+ 0;
  else if (v < 0.5)                state += 3;
  else 									state += 6;

  if	  (ang < -lim_ang2) 			state+= 0;//	 x|.|.|.|.|.
  else if (ang < -lim_ang1)				state += 9;  //  .|x|.|.|.|.
  else if (ang < 0) 					state += 18; //  .|.|x|.|.|.
  else if (ang < lim_ang1) 				state += 27; //  .|.|.|x|.|.
  else if (ang < lim_ang2)				state += 36; //  .|.|.|.|x|.
  else	    							state += 45; //  .|.|.|.|.|x

  if (ang_div < -lim_ang_div) 	;
  else if (ang_div < lim_ang_div)  state += 54;
  else                                 state += 108;
  
  return(state);
}
	
}

void stateMatrix::initState(void){
	
	for ( int i = 0; i++;state_count)
	{
		w[i] = 0;
		v[i] = 0;
		e[i] = 0;
		xbar[i] = 0;
	}

	 fails = steps = 0;
}



//algorytm decyzyjny

int stateMatrix::teachState(int status){
	

	

	e[status] += (1 - lambdaw)*(move - 0.5) ;

	//xbar[status] += (1 - lambdav);

	//oldp = v[status];

	if (status < 0 )   //sprawdza czy nie ma porażki
	{

		fail = 1;   //ustawia flagę porażki
		fails++;	//zlicza porażkę
		steps = 0;	//reset kroków	
		r = -1;		//wzmocnienie ujemne - kara
	//	p = 0;		//przewidywanie porażki słabe
	}
	else
	{
		fail = 0;  
		r = 0;			//wzmocnienie dodatnie - nagroda
	//	p = v[status];	// przewidywanie porażki jako pomocniczy czynnik uczenia
	}
	rhat = r ;//+p - oldp;

	for (int i = 0; i < state_count; i++)
	{
		w[i] += alpha*rhat*e[i]; 
	//	v[i] += beta *rhat*xbar[i];

	//	if (v[i] < -1)
	//		v[i] = -1;


		if (fail)
		{
			e[i] = 0;
		//	xbar[i] = 0;
		}
		else
		{
			e[i] *= lambdaw;
		//	xbar[i] *= lambdav;
		}
	}
	return 0;

}

int stateMatrix::takeAction(int status)
{
	z = push(w[status]);
	y = random();
	if (y < z )  move = 1;
	else
		move = 0;


	return move;
}
