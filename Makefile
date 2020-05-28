
# archivos a compilar
TARGET    := button

SRC_DIR   := src/
OBJ_DIR   := obj/
SRC_FILES := $(wildcard $(SRC_DIR)*.cpp)
# Archivos de objeto
OBJ_FILES := $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRC_FILES))
DEP_FILES := $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.d,$(SRC_FILES))

# librerias
UWS      = ./lib/uWebSockets
USOCKETS = $(UWS)/uSockets

# opciones para el compilador
CPPFLAGS += -O2 -std=c++17 -Wall
CPPFLAGS += -I$(UWS)/src/
CPPFLAGS += -I$(USOCKETS)/src/

# opciones para el linker
LDFLAGS += -L$(USOCKETS)
LDFLAGS += -l z -l ssl -l:uSockets.a

# nombre del ejecutable final

# receta por defecto de make, depende de la libreria uSockets.a
all: $(TARGET)

$(TARGET): $(OBJ_FILES) $(USOCKETS)/uSockets.a
	g++ -o $@ $^ $(LDFLAGS)
	
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	g++ -MMD -MP $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ_FILES) $(DEP_FILES)
	

# receta para hacer uSockets.a, ejecuta make en su carpeta
$(USOCKETS)/uSockets.a:
	make -C $(USOCKETS)


-include $(DEP_FILES)