CC=ccache gcc

CFLAGS += -Wall -Wextra -Werror -Wvla -pedantic -Wswitch
CFLAGS += -Wno-error=unused-variable -Wno-error=unused-result
#CFLAGS += -fsanitize=address,undefined -g
CFLAGS += -O3 -std=c23 -fopenmp -Isrc
LDFLAGS = -lglfw -lGL -lm -lGLEW -ldl

ifeq ($(shell test -f /etc/NIXOS && echo yes),yes)
    CFLAGS += -DWIN_W=1920 -DWIN_H=1200
else
    CFLAGS += -DWIN_W=1920 -DWIN_H=1080
endif

CFLAGS += $(shell pkg-config --cflags glfw3)
LDFLAGS += $(shell pkg-config --libs glfw3)

SRC=$(shell find src -name "*.c")
OBJ=$(SRC:.c=.o)

TARGET=max_c_minecraft

all: $(TARGET)

src/glad/glad.o: src/glad/glad.c
	$(CC) $(CFLAGS) -Wno-pedantic -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm $(TARGET) $(OBJ) -fr

.PHONY: all clean
