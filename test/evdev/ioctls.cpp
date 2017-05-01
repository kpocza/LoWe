#include <iostream>
#include <sys/ioctl.h>
#include <linux/input.h>

using namespace std;

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define ISBITSET(x,y) ((x)[LONG(y)] & BIT(y))
#define OFF(x)   ((x)%BITS_PER_LONG)
#define LONG(x)  ((x)/BITS_PER_LONG)
#define BIT(x)         (1 << OFF(x))

int main()
{
	long relbits[NBITS(REL_MAX + 1)];
   	unsigned long ev[NBITS(EV_MAX)];

	cout << "EV_MAX " << EV_MAX << endl;
	cout << "NBITS(EV_MAX) " << NBITS(EV_MAX) << endl;
	cout << EVIOCGBIT(0, sizeof(ev)) << endl;
	cout << EVIOCGBIT(EV_KEY, sizeof(relbits)) << endl;
	cout << EVIOCGBIT(EV_REL, sizeof(relbits)) << endl;
	cout << EVIOCGUNIQ(8)	<< endl;
	cout << EVIOCGNAME(255) << endl;
	cout << EVIOCGID << endl;
	cout << EV_REL << endl;
	cout << EVIOCGKEY(60) << endl;
	ev[0] = 1 << EV_REL;
	cout << ISBITSET(ev, EV_REL) << endl;
	return 0;
}

