#ifndef RLearning
#define RLearning

#include <ode/ode.h>
#include <drawstuff/drawstuff.h>

class stateMatrix // klasa danych dynamicznych
{
private:
	
	float fails, steps, fail, r, p, rhat;

public:
	void initState (void); //czysczenie pamięci stanów
	int readState (dReal &x, dReal &v, dReal &a, dReal &ang_div); //odczytanie aktualnego stanu
	int teachState(int status);

};





#endif

