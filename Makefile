CC=gcc
NAME_SERVER=server.exe
NAME_CLIENT=client.exe
LINKER_FLAGS= -pthread -lsqlite3
CFLAGS_ALL = -Ilib -Wall -Wextra -Werror -Wfatal-errors -Wno-unused-parameter -pthread -mshstk -Wno-implicit-fallthrough
CFLAGS_TMP = -Wno-unused-function -Wno-unused-variable -Wno-deprecated-declarations

ifeq ($(OS),Windows_NT)
	LINKER_FLAGS += -lws2_32
	BUILD_SUFFIX=w
else
	NAME_SERVER=server
	NAME_CLIENT=client
	BUILD_SUFFIX=l
endif

SRC_LIB = lib/json.c lib/networking.c 
SRC_SERVER = src-server/main.c src-server/run.c src-server/process.c
SRC_CLIENT = src-client/main.c
SRC_VALIDATORS = src-validators/ints.c src-validators/floats.c
OBJ_DEV_LIB = $(SRC_LIB:%.c=build/dev$(BUILD_SUFFIX)/%.o)
OBJ_DEV_SERVER = $(SRC_SERVER:%.c=build/dev$(BUILD_SUFFIX)/%.o)
OBJ_DEV_CLIENT = $(SRC_CLIENT:%.c=build/dev$(BUILD_SUFFIX)/%.o)
EXE_VALIDATORS = $(SRC_VALIDATORS:src-validators/%.c=bin/validators/%)

all: dev
	@rm -rf example/problem1/bin example/problem1/runs example/runs.db example/runs.db-shm example/runs.db-wal

dev: build dev-server dev-client validators
client: dev-client
server: dev-server

dev-client: $(OBJ_DEV_LIB) $(OBJ_DEV_CLIENT)
	@mkdir -p bin/dev
	@$(CC) $(OBJ_DEV_LIB) $(OBJ_DEV_CLIENT) $(LINKER_FLAGS) -o bin/dev/$(NAME_CLIENT)

dev-server: $(OBJ_DEV_LIB) $(OBJ_DEV_SERVER)
	@mkdir -p bin/dev
	@$(CC) $(OBJ_DEV_LIB) $(OBJ_DEV_SERVER) $(LINKER_FLAGS) -o bin/dev/$(NAME_SERVER)

validators: $(EXE_VALIDATORS)

bin/validators/%: src-validators/%.c
	@echo $<
	@mkdir -p bin/validators
	@$(CC) $(CFLAGS_ALL) $(CFLAGS_TMP) -g3 -o bin/validators/$(@F) $<


build/dev$(BUILD_SUFFIX)/%.o: %.c
	@echo $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS_ALL) $(CFLAGS_TMP) -g3 -c -o $@ $<

build:
	@mkdir -p build

clean:
	rm -rf build bin example/problem1/runs example/problem1/bin example/problem1/tmp example/runs.db example/runs.db-shm example/runs.db-wal
	
.PHONY: clean validators

