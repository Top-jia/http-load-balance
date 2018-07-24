#include"log.hpp"

int main()
{
	Logger log;
	log.WriteFile(false, 7, "jia");
	log.WriteFile(true, 1, "lai");
	return 0;
}
