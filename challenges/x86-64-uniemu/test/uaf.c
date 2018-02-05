#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <iostream>

using namespace std;

class Bling {
	public:
	virtual void hello();
};

void Bling::hello()
{
	cout << "Hello World" << endl;
}

void fill(size_t s)
{
	void *p = (void*) malloc(s);
	if(p) memset(p, 0x41, s);
}

int main(int argc, char **argv)
{
	Bling *b1 = new Bling();
	Bling *b2 = b1;

	b1->hello();
	delete b1;
	fill(16);  fill(16);

	b2->hello();
}
