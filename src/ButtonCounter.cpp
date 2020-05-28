#include "ButtonCounter.h"

#include <cstring>
#include <iostream>

ButtonCounter::ButtonCounter(int posX, int posY, int width, int height, int counter)
: Object(T_BUTTON_COUNTER, posX, posY, width, height),
  counter(counter) {
    std::cout << "Init counter: " << this->counter << std::endl;
}

ButtonCounter::~ButtonCounter() { }

uint64_t ButtonCounter::getCounter() const {
	// retornar el contador
	return counter;
}

void ButtonCounter::click() {
	//Incrementar el contador
	++counter;
}

char * ButtonCounter::bufWrite(char * buf) {
	
	buf = Object::bufWrite(buf);
	std::memcpy(buf, &counter, sizeof(counter)); buf += sizeof(counter);
	
	return buf;
}

int ButtonCounter::bufSize() const {
	return Object::bufSize() + sizeof(counter);
}
