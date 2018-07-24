#include"test_c.hpp"

c_show::c_show(std::string _name):name(_name){

}

void c_show::show(){
	std::cout << "c_show::show()_" + name << std::endl;
}
