#include"scheduler.hpp"

int main()
{
	Scheduler elem;
	elem.CreateLink();
	elem.Run();
	Logger log;
	log.WriteFile(false, 2, "main() failed");
	return 0;
}
