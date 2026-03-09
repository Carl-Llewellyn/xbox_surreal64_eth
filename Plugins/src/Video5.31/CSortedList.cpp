#include "stdafx.h"

class A
{
	int a;
	int b;

public:
	A(int aa, int bb)
	{
		a=aa;
		b=bb;
	}
};
CSortedList<DWORD, A*> mlist(100);