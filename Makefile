
FILES = src/main.cpp

UWS      = ./lib/uWebSockets
USOCKETS = $(UWS)/uSockets

CPPFLAGS += -O2 -std=c++17
CPPFLAGS += -I$(UWS)/src/
CPPFLAGS += -I$(USOCKETS)/src/

LDFLAGS += -L$(USOCKETS)
LDFLAGS += -l z -l ssl -l:uSockets.a

OUT = button

all: $(USOCKETS)/uSockets.a
	g++ $(CPPFLAGS) $(FILES) $(LDFLAGS) -o $(OUT)

$(USOCKETS)/uSockets.a:
	make -C $(USOCKETS)
