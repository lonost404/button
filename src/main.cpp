#include <iostream>
#include <cstdint> // uint16_t, int32_t, ...
#include <cstring>
#include "Server.h"

int main() {
	std::cout<<"Starting server...\n";
	
	Server sv;
	sv.run();
}

