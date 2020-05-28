#pragma once
#include <string_view>
#include <App.h> // from uWebSockets
#include "ButtonCounter.h"

class Server {
	
	struct PerSocketData {};
	int port;
	uWS::App ws;
	us_timer_t * saveTimer;
	ButtonCounter button;
	

	uint64_t getInitialCount();
	
public:
	Server(int p = 1337);
	
    void run();
	void saveCount();
	void onOpen(uWS::WebSocket<false, true> * ws, uWS::HttpRequest *);
	void onMessage(uWS::WebSocket<false, true> * ws, std::string_view msg, uWS::OpCode oc);
    void sendObjects(uWS::WebSocket<false, true> * ws);
};
