NAME_SERVER=server.exe
NAME_CLIENT=client.exe
NAME_GUI=gui.exe
BUILD_SUFFIX=w
LINKER_FLAGS= -pthread
CFLAGS_ALL = -Ilib -MMD -Wall -Wextra -Werror -Wfatal-errors -Wno-unused-parameter -pthread -mshstk -Wno-implicit-fallthrough

ifeq ($(OS),Windows_NT)
	LINKER_FLAGS += -lws2_32
else
	NAME_SERVER=server
	NAME_CLIENT=client
	NAME_GUI=gui
	BUILD_SUFFIX=l
endif

SRC_LIB = lib/json.c lib/networking.c lib/process.c
SRC_SERVER = src-server/main.c src-server/run.c
SRC_CLIENT = src-client/main.c
SRC_GUI = src-gui/main.c
OBJ_DEV_LIB = $(SRC_LIB:%.c=build/dev$(BUILD_SUFFIX)/%.o)
OBJ_DEV_SERVER = $(SRC_SERVER:%.c=build/dev$(BUILD_SUFFIX)/%.o)
OBJ_DEV_CLIENT = $(SRC_CLIENT:%.c=build/dev$(BUILD_SUFFIX)/%.o)
OBJ_DEV_GUI = $(SRC_GUI:%.c=build/dev$(BUILD_SUFFIX)/%.o)

all: dev
	@rm -rf problems/problem1/bin problems/problem1/runs

dev: build dev-server dev-client
client: dev-client
server: dev-server

dev-server: $(OBJ_DEV_LIB) $(OBJ_DEV_SERVER)
	@mkdir -p bin/dev
	@gcc $(OBJ_DEV_LIB) $(OBJ_DEV_SERVER) $(LINKER_FLAGS) -o bin/dev/$(NAME_SERVER)

dev-client: $(OBJ_DEV_LIB) $(OBJ_DEV_CLIENT)
	@mkdir -p bin/dev
	@gcc $(OBJ_DEV_LIB) $(OBJ_DEV_CLIENT) $(LINKER_FLAGS) -o bin/dev/$(NAME_CLIENT)

dev-gui: $(OBJ_DEV_LIB) $(OBJ_DEV_GUI)
	@mkdir -p bin/dev
	@gcc $(OBJ_DEV_LIB) $(OBJ_DEV_GUI) $(LINKER_FLAGS) -o bin/dev/$(NAME_GUI)

build/dev$(BUILD_SUFFIX)/%.o: %.c
	@echo $<
	@mkdir -p $(dir $@)
	@gcc $(CFLAGS_ALL) -g3 -c -o $@ $<

build:
	@mkdir -p build

clean:
	rm -rf build bin problems/problem1/runs problems/problem1/bin
	
.PHONY: clean

