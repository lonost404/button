#include "Server.h"
#include <fstream>
#include <cstring>
#include <cmath>

#include "Player.h"

enum PacketTypeFromClient : uint8_t {
	PC_PING,
	PC_CLICK,
	PC_MOVE,
	PC_DRAW
};

enum PacketTypeFromServer : uint8_t {
	PS_COUNTER,
	PS_CLICK,
	PS_MOVE,
	PS_DRAW,
	PS_OBJECTS,
	PS_SET_ID,
	PS_DISCONNECT
};


struct TimerData {
	Server * s;
	int calls;
};

void timerFired(us_timer_t * timer) {
	// accede a los datos que guardamos
	TimerData * data = (TimerData*)us_timer_ext(timer);
	
	if (++data->calls > 900) {
		data->calls = 0;
		data->s->saveCount();
	}
	
	data->s->broadcastPlayerList();
}

Server::Server (int port)
: port(port),
  button(0, 0, 80, 40, getInitialCount()),
  playerIdCounter(0) {
  	
	// crea un temporizador con tamaño extra para 1 puntero (sizeof(Server*): 8 bytes) y 1 int (4 bytes)
	saveTimer = us_create_timer((us_loop_t *)uWS::Loop::get(), true, sizeof(Server*) + sizeof(int));

	
	int ms = 1000 / 15; // 15 veces por segundo

	us_timer_set(saveTimer, timerFired, ms, ms);
	
	
	// guarda una referencia de la instancia de este servidor
	// en el temporizador, para llamar a la funcion correcta en timerFired.
	// us_timer_ext nos devuelve un puntero a los 8 bytes nuestros, donde
	// guardaremos nuestro puntero a 'this':
	TimerData * td = (TimerData*)us_timer_ext(saveTimer);
	td->s = this;
	td->calls = 0;

	//Iinicializar callback del servidor
	ws.ws<Player>("/*", {
		.maxPayloadLength = 16,
		.idleTimeout = 30000,
		.open = [this] (auto * ws, auto * req) { onOpen(ws, req); },
		.message = [this] (auto * ws, auto msg, auto oc) { onMessage(ws, msg, oc); },
		.close = [this] (auto * ws, int code, std::string_view) { onClose(ws, code); }
	});
	
	// Especificar en que puerto escucha
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
	ws->subscribe("players");
	players.emplace(ws);
	Player * p = (Player*)ws->getUserData();
	
	p->id = playerIdCounter++;
	p->posX = 0;
	p->posY = 0;
	
	sendUserId(ws, p->id);
	sendObjects(ws);
}

void Server::sendUserId(uWS::WebSocket<false, true> * ws, int16_t id) {
	int idBufferSize = sizeof(uint8_t) + sizeof(int16_t);
	char idBuffer[idBufferSize];
	
	idBuffer[0] = PS_SET_ID;
	std::memcpy(idBuffer+1, &id, sizeof(int16_t));
	
	ws->send(std::string_view(idBuffer, idBufferSize), uWS::BINARY);
}

void Server::onClose(uWS::WebSocket<false, true> * cws, int code) {
	Player * p = (Player*)cws->getUserData();
	
	int idBufferSize = sizeof(uint8_t) + sizeof(int16_t);
	char idBuffer[idBufferSize];
	
	idBuffer[0] = PS_DISCONNECT;
	std::memcpy(idBuffer+1, &p->id, sizeof(int16_t));
	
	players.erase(cws);
	
	ws.publish("players", std::string_view(idBuffer, idBufferSize), uWS::BINARY);
}

// Cuando un usuario le de click al botón
// estructura de un paquete:
// - 1 byte: tipo (ping, click, mover, dibujar, chat)
// - - ping: (no hay datos extra)
// - - click: (int16_t, int16_t) x, y
// - - mover: (int8_t, int8_t) x, y
// - - dibujar: (int16_t, int16_t, int16_t, int16_t) 

void Server::onMessage(uWS::WebSocket<false, true> * ws, std::string_view msg, uWS::OpCode oc) {
	switch ((uint8_t)msg[0]) { // 
		case PC_PING: // ping
			break;
			
		case PC_CLICK: { // click
			if (msg.size() == 5) {
				
				int16_t x;
				int16_t y;
				
				std::memcpy(&x, msg.data() + 1, sizeof(int16_t));
				std::memcpy(&y, msg.data() + 3, sizeof(int16_t));
				
				float px = ((float)x / 32768.f) * 100.f; // -100%...100%
				float py = ((float)y / 32768.f) * 100.f;
				
				int clickBufferSize = sizeof(uint8_t) + sizeof(int16_t) * 2;
				
				
				char clickBuffer[clickBufferSize];
				
				clickBuffer[0] = 0x01;
				
				std::memcpy(clickBuffer + 1, &x, sizeof(int16_t));
				std::memcpy(clickBuffer + 3, &y, sizeof(int16_t));

				//std::cout << "click en: " << (uint16_t)x << ", " << (uint16_t)y << std::endl;
				ws->publish("players", std::string_view(clickBuffer, clickBufferSize), uWS::BINARY);
				
				//Comprobar si esta dentro del botón
				px += button.posX;
				py += button.posY;
				//std::cout << abs(px) << ", " << (float)button.width / 2.f << ", ";
				//std::cout << abs(py) << ", " << (float)button.height / 2.f << std::endl;
				if (abs(px) < (float)button.width
						&& abs(py) < (float)button.height) {
					button.click();
					uint64_t counter = button.getCounter();

					int countBufferSize = sizeof(uint8_t) + sizeof(uint64_t);
					char countBuffer[countBufferSize];
					std::cout<<"Clicado"<<std::endl;
					countBuffer[0] = 0x00;
					
					std::memcpy(countBuffer + 1, &counter, sizeof(uint64_t));
					
					ws->publish("players", std::string_view(countBuffer, countBufferSize), uWS::BINARY);
				}
				
			}
		} break;
			
		case PC_MOVE: { // mover
			if (msg.size() == 5) {
				int16_t x;
				int16_t y;
				
				std::memcpy(&x, msg.data() + 1, sizeof(int16_t));
				std::memcpy(&y, msg.data() + 3, sizeof(int16_t));
				
				Player * p = (Player*)ws->getUserData();
				p->posX = x;
				p->posY = y;
				
				p->moved = true;
			}
		} break;

		case PC_DRAW: // dibujar
			if (msg.size() == 9) {
				int16_t x1;
				int16_t y1;
				int16_t x2;
				int16_t y2;
				
				std::memcpy(&x1, msg.data() + 1, sizeof(int16_t));
				std::memcpy(&y1, msg.data() + 3, sizeof(int16_t));
				std::memcpy(&x2, msg.data() + 5, sizeof(int16_t));
				std::memcpy(&y2, msg.data() + 7, sizeof(int16_t));
				
				char resp[1 + sizeof(int16_t) * 4];
				resp[0] = PS_DRAW;
				std::memcpy(resp + 1, msg.data() + 1, sizeof(int16_t) * 4);
				ws->publish("players", std::string_view(resp, 1 + sizeof(int16_t) * 4), uWS::BINARY);
				
				Player * p = (Player*)ws->getUserData();
				p->posX = x2;
				p->posY = y2;
				
				p->moved = true;
			}
			break;
		
	}
}

void Server::broadcastPlayerList() {
	
	int moveBufferSize = 0;
	for (auto * w : players) {
		Player * p = (Player*)w->getUserData();
		
		if (p->moved) {
			moveBufferSize += 1;
		}
	}
	
	if (moveBufferSize > 0) {
		moveBufferSize *= sizeof(Player::id) + sizeof(Player::posX) + sizeof(Player::posY);
		moveBufferSize += 1;
		
		// posible stack overflow aqui con muchos jugadores
		char moveBuffer[moveBufferSize];
		
		moveBuffer[0] = PS_MOVE;
		
		char * cursor = moveBuffer + 1;
		
		for (auto * w : players) {
			Player * p = (Player*)w->getUserData();
			
			if (p->moved) {
				std::memcpy(cursor, &p->id, sizeof(Player::id));
				cursor += sizeof(Player::id);
				std::memcpy(cursor, &p->posX, sizeof(Player::posX));
				cursor += sizeof(Player::posX);
				std::memcpy(cursor, &p->posY, sizeof(Player::posY));
				cursor += sizeof(Player::posY);
			}
			p->moved = false;
		}
		
		
		
		ws.publish("players", std::string_view(moveBuffer, moveBufferSize), uWS::BINARY); 
	}
	
}




// Aqui enviamos los objetos a un cliente
void Server::sendObjects(uWS::WebSocket<false, true> * ws) {
	uint32_t size = button.bufSize() + 1;
	
	char buffer[size];
	
	buffer[0] = PS_OBJECTS;

	button.bufWrite(buffer + 1);
	
	ws->send(std::string_view(buffer, size), uWS::BINARY);
}

//Inicialización del servidor WebSocket
void Server::run() {
	ws.run();
}
