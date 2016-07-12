CC = g++
CFLAGS = -Werror -std=c++11 -O3
LDFLAGS = -ltgui -lsfml-graphics -lsfml-window -lsfml-system
TARGET = gravity-simulator
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean destroy rebuild

clean:
	rm -f $(OBJ)

destroy: clean
	rm -f $(TARGET)

rebuild: destroy all
