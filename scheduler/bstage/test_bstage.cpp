#include"bstage.hpp"


int main()
{
	Logger log;
	Bstage bstage(log);
	bstage.CloseFD();
	return 0;
}
