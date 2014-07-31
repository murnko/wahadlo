#include "stdafx.h"
#include "RLearning.h"
#include <ode/ode.h>
#include <drawstuff/drawstuff.h>
#include <math.h>




static const	int state_count = 896; //liczba możliwych stanów
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

int move, state, works = 0;
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


//granica porażki
dReal porazka_x			= 10;
dReal porazka_v			= 1;
dReal porazka_ang		= 0.2094384; //12stopni  //chyba trzeba zamienic na granicę na dole, a 20st przeniesc do stanow "normalnych"


//granice przedziałów
//przesunięcie
dReal lim_x1	= 5;

//kąt
dReal lim_ang1	= 3.1241393; // 179 st
dReal lim_ang2  = 2.9670597; // 170   <
dReal lim_ang3	= 3.0368728; //	174   
dReal lim_ang4  = 2.7925268; // 160   <
dReal lim_ang5  = 2.3561944; // 135   <
dReal lim_ang6  = 1.5707963; // 90    <
dReal lim_ang7  = 0.7853981; // 45    <
dReal lim_ang8  = 0.3490658; // 20    <








dReal lim_ang_div = 0.87266; //50st/sek








int stateMatrix::readState (dReal &x, dReal &v, dReal &ang, dReal &ang_div)
{
	
	state =0;
	steps++;
	printf("%d \n",steps);	

  if (ang < -lim_ang6 || ang > lim_ang6) works = 1;  //mały sukces odciąga karę (wstępnie zamiast rozdrabniania nagród)

  if (steps > (1000 + 1000*works))
  {
  if (x < -porazka_x || x > porazka_x ||((ang > -lim_ang2 || ang < lim_ang2) && !works)  ) 
	{ 
		works = 0;
		return -1;  
	}
  }


	//limit prób lub kroków
  //if (fails > 1000 || steps_max > 10000) return(-2);

  

  if	  (ang < -lim_ang8)				state = 0;		//20
  else if (ang < -lim_ang7)				state += 1;		//45
  else if (ang < -lim_ang6)				state += 2;		//90
  else if (ang < -lim_ang5)				state += 3;		//135
  else if (ang < -lim_ang4) 			state += 4;	//160
  else if (ang < -lim_ang2)				state += 5;	//170
  else if (ang < 3.14) 					state += 6;//  0
  else if (ang < lim_ang2) 				state += 7;	//170
  else if (ang < lim_ang4)				state += 8;	//160
  else if (ang < lim_ang5)				state += 9;		//135
  else if (ang < lim_ang6)				state += 10;	//90
  else if (ang < lim_ang7)    			state += 11;	//45
  else if (ang < lim_ang8)				state += 12;	//20
  else									state += 13;    //reszta

  if (x < -lim_x1)  		       state +=  14;
  else if (x < 0.8)     	       state +=  28;
  else		    	               state +=  42;

  if (v < -0.5) 					state =+ 56;
  else if (v < 0.5)					state += 112;
  else 								state += 168;

  if (ang_div < -lim_ang_div)		state += 224;	
  else if (ang_div < lim_ang_div)   state += 448;
  else                              state += 672;
  
  return(state);

	
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
		r = 0;	
			//wzmocnienie dodatnie - nagroda
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
