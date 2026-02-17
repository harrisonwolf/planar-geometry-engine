//implementation file for die.h
//currently empty since all the code in in die.h

#include "die.h"

using namespace std;

void die(){
	cout << "You probably violated one of the program's invariants (e.g. no two points colinear). Because of this, the program will now explode.)\n";
	cout << "ABORT\n";
	exit(1);
}
