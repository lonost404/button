#pragma once

#include <cstdint>
#include "Object.h"

class ButtonCounter : public Object {
	
	uint64_t counter;
	
public:
	ButtonCounter(int posX, int posY, int width, int height, int counter);
	
	virtual ~ButtonCounter();
	
	uint64_t getCounter() const;
	
	void click();
	
	char * bufWrite(char * buf);
	
	int bufSize() const;
};