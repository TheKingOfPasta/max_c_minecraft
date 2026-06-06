CC  = ccache gcc
CXX = ccache g++

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

TRACY_CFLAGS = -I$(TRACY_DIR)/include/tracy -DTRACY_ENABLE
TRACY_LDFLAGS = -L$(TRACY_DIR)/lib -lTracyClient -Wl,-rpath,$(TRACY_DIR)/lib

SRC = $(shell find src -name "*.c")
OBJ = $(SRC:.c=.o)

TARGET = max_c_minecraft
PROFILE_TARGET = $(TARGET)_profile
PROFILE_OBJ = $(SRC:.c=.profile.o)
TRACY_OBJ = src/opengl/tracy.profile.o

all: $(TARGET)

src/glad/glad.o: src/glad/glad.c
	$(CC) $(CFLAGS) -Wno-pedantic -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(PROFILE_TARGET): CFLAGS += $(TRACY_CFLAGS)
$(PROFILE_TARGET): LDFLAGS += $(TRACY_LDFLAGS)
$(PROFILE_TARGET): $(PROFILE_OBJ) $(TRACY_OBJ)
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.profile.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

src/glad/glad.profile.o: src/glad/glad.c
	$(CC) $(CFLAGS) -Wno-pedantic -c $< -o $@

src/opengl/tracy.profile.o: src/opengl/tracy.cpp
	$(CXX) $(TRACY_CFLAGS) -Isrc -std=c++17 -O3 -c $< -o $@

profile:
	@[ -n "$(TRACY_DIR)" ] || { echo "error: TRACY_DIR not set :P"; exit 1; }
	$(MAKE) $(PROFILE_TARGET)
	@$(TRACY_DIR)/bin/tracy-capture -o trace.tracy -f & sleep 0.5 && ./$(PROFILE_TARGET) && wait
	@echo "open trace with: tracy trace.tracy"

clean:
	rm -f $(TARGET) $(OBJ) $(PROFILE_TARGET) $(PROFILE_OBJ) $(TRACY_OBJ) trace.tracy

.PHONY: all clean profile
