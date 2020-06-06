#include "Object.h"
#include <cstring> // std::memcpy

Object::Object(uint8_t typeId, int16_t posX, int16_t posY, uint8_t width, uint8_t height)
: typeId(typeId),
  posX(posX),
  posY(posY),
  width(width),
  height(height) {}

Object::~Object() {
	// empty
}

void Object::click() {}
	
char * Object::bufWrite(char * buf) {
	std::memcpy(buf, &typeId, sizeof(typeId)); buf += sizeof(typeId);
	std::memcpy(buf, &posX,   sizeof(posX));   buf += sizeof(posX);
	std::memcpy(buf, &posY,   sizeof(posY));   buf += sizeof(posY);
	std::memcpy(buf, &width,  sizeof(width));  buf += sizeof(width);
	std::memcpy(buf, &height, sizeof(height)); buf += sizeof(height);
	
	return buf;
}
	
int Object::bufSize() const {
	return sizeof(uint8_t)
		+ sizeof(posX) + sizeof(posY)
		+ sizeof(width) + sizeof(height);
}
