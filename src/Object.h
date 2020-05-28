#pragma once

#include <cstdint> // uint8_t

enum ObjectType : uint8_t {
	T_NULL = 0,
	T_BUTTON_COUNTER = 1
};

class Object { // objeto en la pantalla, abstracto
	
public:
	const uint8_t typeId;
	const int16_t posX;
	const int16_t posY;
	const uint8_t width;
	const uint8_t height;
	
	Object(uint8_t typeId, int16_t posX, int16_t posY, uint8_t width, uint8_t height);
	virtual ~Object();
	
	virtual void click();
	virtual char * bufWrite(char * buf);
	virtual int bufSize() const;
};
