#include "Server.h"
#include <fstream>
#include <cstring>

void timerFired(us_timer_t * timer) {
	// accede al 'this' que guardamos
	Server * s = *(Server **)us_timer_ext(timer);

	s->saveCount();
}

Server::Server (int port)
: port(port),
  button(0, 0, 100, 20, getInitialCount()) {
	
	// crea un temporizador con tamaño extra para 1 puntero (sizeof(Server*): 8 bytes)
	saveTimer = us_create_timer((us_loop_t *)uWS::Loop::get(), true, sizeof(Server*));

	int ms = 1000 * 60;

	us_timer_set(saveTimer, timerFired, ms, ms); // cada 1 min
	
	// guarda una referencia de la instancia de este servidor
	// en el temporizador, para llamar a la funcion correcta en timerFired.
	// us_timer_ext nos devuelve un puntero a los 8 bytes nuestros, donde
	// guardaremos nuestro puntero a 'this', por eso es un puntero doble:
	*(Server **)us_timer_ext(saveTimer) = this;

	//Iinicializar callback del servidor
	ws.ws<PerSocketData>("/", {
		.maxPayloadLength = 2,
		.idleTimeout = 0,
		.open = [this] (auto * ws, auto * req) { onOpen(ws, req); },
		.message = [this] (auto * ws, auto msg, auto oc) { onMessage(ws, msg, oc); }
	});
	
	//Especificar en que puerto escucha
	ws.listen(port, [this] (auto *token) {
		if (token) {
			std::cout << "Listening on port " << this->port << "\n";
		}
	});
}

uint64_t Server::getInitialCount() {
	// Cargar numero de clicks del archivo count.bin
	std::fstream countFile;
	countFile.open("count.bin", std::ios::binary | std::ios::in);
	int size = sizeof(uint64_t);
	uint64_t count = 0;
	
	if (countFile) {
		countFile.read((char *)&count, size);
		
	} else {
		countFile.open("count.bin", std::ios::binary | std::ios::out | std::ios::trunc);
		std::cout<<"Creating file count"<<std::endl;
		countFile.write((char *)&count, size);
		
	}
	countFile.close();
	std::cout<<count<<std::endl;
	return count;
}

void Server::saveCount() {
	// Guardar numero de clicks del archivo count.bin
	uint64_t count = button.getCounter();
	std::cout << "Saving button counter: " << count << std::endl;
	std::ofstream countFile;
	countFile.open("count.bin", std::ios::binary | std::ios::trunc);
	countFile.write((char *)&count, sizeof(uint64_t));
	countFile.close();
}

void Server::onOpen(uWS::WebSocket<false, true> * ws, uWS::HttpRequest * req) {
	std::cout << "New player" << std::endl;
	ws->subscribe("clicks");
	sendObjects(ws);
}

//Cuando un usuario le de click al botón
void Server::onMessage(uWS::WebSocket<false, true> * ws, std::string_view msg, uWS::OpCode oc) {
	if (msg.size() == 2) {
		uint8_t x = *(uint8_t *)(msg.data());
		uint8_t y = *(uint8_t *)(msg.data() + 1);
		
		int buffersize = msg.size() + sizeof(uint64_t);
		char response[buffersize];
	
		button.click();
		
		uint64_t counter = button.getCounter();
		std::memcpy(response, msg.data(), sizeof(uint8_t) * 2);
		std::memcpy(response + sizeof(uint8_t) * 2, &counter, sizeof(uint64_t));
		
		std::cout << "click en: " << (uint16_t)x << ", " << (uint16_t)y << std::endl;
		ws->publish("clicks", std::string_view(response, buffersize), uWS::BINARY);
	}
}

// Aqui enviamos los objetos a un cliente
void Server::sendObjects(uWS::WebSocket<false, true> * ws) {
	uint32_t size = button.bufSize();
	
	char buffer[size];

	button.bufWrite(buffer);
	
	ws->send(std::string_view(buffer, size), uWS::BINARY);
}

//Inicialización del servidor WebSocket
void Server::run() {
	ws.run();
}
