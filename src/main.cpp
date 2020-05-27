#include <iostream>
#include <vector>
#include <string_view>
#include <cstdint> // uint16_t, int32_t, ...
#include <cstring>
#include <functional>

#include <App.h> // from uWebSockets


class Object { // objeto en la pantalla, abstracto
	
public:
	const uint8_t typeId;
	const int posX;
	const int posY;
	const int width;
	const int height;
	
	Object(uint8_t typeId, int posX, int posY, int width, int height)
	: typeId(typeId),
	  posX(posX),
	  posY(posY),
	  width(width),
	  height(height) {
		
	}
	
	virtual ~Object() {
		// empty
	}
	
	virtual void click() {}
	
	virtual char * bufWrite(char * buf) {
		std::memcpy(buf, &typeId, sizeof(uint8_t)); buf += sizeof(uint8_t);
		std::memcpy(buf, &posX, sizeof(int)); buf += sizeof(int);
		std::memcpy(buf, &posY, sizeof(int)); buf += sizeof(int);
		std::memcpy(buf, &width, sizeof(int)); buf += sizeof(int);
		std::memcpy(buf, &height, sizeof(int)); buf += sizeof(int);
		
		return buf;
	}
	
	virtual int bufSize() const {
		return sizeof(uint8_t) + sizeof(int) * 4; // typeid, x, y, w, h
	}
};

class ButtonCounter : public Object {
	
	uint64_t counter;
	
public:
	ButtonCounter(int posX, int posY, int width, int height, int counter)
	: Object(1, posX, posY, width, height),
	  counter(counter) { }
	
	virtual ~ButtonCounter() { }
	
	uint64_t getCounter() const {
		// retornar el contador
		return counter;
	}

	void click() {
		//Incrementar el contador
		++counter;
	}
	
	char * bufWrite(char * buf) {
		
		buf = Object::bufWrite(buf);
		std::memcpy(buf, &counter, sizeof(uint64_t)); buf += sizeof(uint64_t);
		
		return buf;
	}
	
	int bufSize() const {
		return Object::bufSize() + sizeof(uint64_t);
	}
};


class Server {
	
	int port = port;
	uWS::App ws;
	struct PerSocketData {};
	ButtonCounter button;
		

	uint64_t getCountSql() {
		
	}
protected:
	std::vector<Object *> objects;

public:
	Server(int port = 1337)
	: port(port),
	  button(0, 0, 100, 20, getCountSql()) {
		//Cargar botón y cargar numero de clicks de la bdd
		
		
		//Iinicializar callback del servidor
		ws.ws<PerSocketData>("/", {
			.maxPayloadLength = 8,
			.idleTimeout = 20,
			.open = [this] (auto * ws, auto * req) { onOpen(ws, req); },
			.message = [this] (auto * ws, auto msg, auto oc) { onMessage(ws, msg, oc); }
		});
		
		//Especificar en que puerto escucha
		ws.listen(port, [this] (auto *token) {
			if (token) {
				std::cout << "Listening on port " << this->port << "\n";
				//std::cout<<"Done!\n";
			}
		});
	}
	
	void onOpen(auto * ws, auto * req) {
		std::cout << "New player" << std::endl;
		ws->subscribe("clicks");
		ws->subscribe("counter_update");
	}
	
	//Cuando un usuario le de click al botón
	void onMessage(uWS::WebSocket<true, false> * ws, std::string_view msg, uWS::OpCode oc) {
		if (msg.size() == 8) {
			int32_t x = *(int32_t *)(msg.data());
			int32_t y = *(int32_t *)(msg.data() + 4);
			
			if (x <= 100 && y <= 100) {
				std::cout << "click en: " << x << ", " << y << std::endl;
				ws->publish("clicks", msg, uWS::BINARY);
			}
			
			
		}
	}
	
	// aqui enviamos los objetos a un cliente
	void sendObjects(uWS::WebSocket<true, false> * ws) {
		uint32_t size = 0;
		for (Object * obj : &objects) {
			size += obj->bufSize();
		}
		
		char buffer[size];
		char * cursor = &buffer[0];
		
		for (Object * obj : &objects) {
			cursor = obj->bufWrite(cursor);
		}
		
		ws->send(buffer, size, uWS::BINARY);
	}
	
	//Añadir objeto a la lista de objetos
	void addObject(Object * obj) {
		objects.push_back(obj);
	}
	
	//Inicialización del servidor WebSocket
	void run() {
		ws.run();
	}
};

int main() {
	std::cout<<"Starting server...\n";
	
	Server sv;
	
	// falta leer archivo
	uint64_t counter = 0;
	sv.addObject(new ButtonCounter(0, 0, 100, 20, counter));
	
	sv.run();
}

