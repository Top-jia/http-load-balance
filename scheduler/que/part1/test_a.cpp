#include"test.hpp"

a_show::a_show(std::string _name):name(_name){

}

void a_show::show(){
	std::cout << "a_show::_show() " + name << std::endl;
}
