#include"sepoll.hpp"

int main(){
	Sepoll edata;
	
	edata.addfd(7);
	edata.addfd(8);

	edata.delfd(7);
	edata.setnonblocking(7);
	
	return 0;
}
