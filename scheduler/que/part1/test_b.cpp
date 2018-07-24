#include"test.hpp"

b_show::b_show(std::string _name):name(_name){

}

void b_show::show(){
	std::cout << "b_show::_show() " + name << std::endl;
}
