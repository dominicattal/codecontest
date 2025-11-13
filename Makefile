CC = gcc
CFLAGS = -MMD -Wall -Wextra -Werror -Wfatal-errors -Wno-cast-function-type -Wno-unused-parameter \
		 -Wno-stringop-overflow -Wno-use-after-free -finline-functions -pthread
CFLAGS_DEV = -g -D DEBUG_BUILD
CFLAGS_RELEASE = -O3 -D RELEASE_BUILD
LINKER_FLAGS = -lglfw3dll -lws2_32 -lm
DIR_LIB = lib
DIR_SRC_SERVER = src-server
DIR_SRC_CLIENT = src-client
DIR_SRC_GUI = src-gui
DIR_OBJ = build
DIR_DEP = build
DIR_BIN = bin

$(shell mkdir -p build)
SOURCES_SERVER  = $(shell find $(DIR_SRC_SERVER) $(DIR_LIB) -name "*.c")
SOURCES_CLIENT  = $(shell find $(DIR_SRC_CLIENT) $(DIR_LIB) -name "*.c")
SOURCES_GUI = $(shell find $(DIR_SRC_GUI) $(DIR_LIB) -name "*.c")
DEPSH    = $(shell find build -name "*.d" -exec grep -Eoh "[^ ]+.h" {} +)
INCLUDES = -Ilib
LIBS     = -Llib
OBJS_DEV_SERVER = $(SOURCES_SERVER:%.c=$(DIR_OBJ)/dev/%.o)
OBJS_REL_SERVER = $(SOURCES_SERVER:%.c=$(DIR_OBJ)/release/%.o)
DEPS_DEV_SERVER = $(OBJS_DEV_SERVER:%.o=%.d)
DEPS_REL_SERVER = $(OBJS_REL_SERVER:%.o=%.d)
OBJS_DEV_CLIENT = $(SOURCES_CLIENT:%.c=$(DIR_OBJ)/dev/%.o)
OBJS_REL_CLIENT = $(SOURCES_CLIENT:%.c=$(DIR_OBJ)/release/%.o)
DEPS_DEV_CLIENT = $(OBJS_DEV_CLIENT:%.o=%.d)
DEPS_REL_CLIENT = $(OBJS_REL_CLIENT:%.o=%.d)
OBJS_DEV_GUI = $(SOURCES_GUI:%.c=$(DIR_OBJ)/dev/%.o)
OBJS_REL_GUI = $(SOURCES_GUI:%.c=$(DIR_OBJ)/release/%.o)
DEPS_DEV_GUI = $(OBJS_DEV_GUI:%.o=%.d)
DEPS_REL_GUI = $(OBJS_REL_GUI:%.o=%.d)

all: dev

dev: dev-gui dev-server dev-client
	rm -rf problems/problem1/runs problems/problem1/bin

dev-server: $(OBJS_DEV_SERVER)
	@mkdir -p $(DIR_BIN)/dev
	@$(CC) $(CFLAGS) $(CFLAGS_DEV) $(LIBS) $(INCLUDES) $(OBJS_DEV_SERVER) $(LINKER_FLAGS) -o $(DIR_BIN)/dev/server

dev-client: $(OBJS_DEV_CLIENT)
	@mkdir -p $(DIR_BIN)/dev
	@$(CC) $(CFLAGS) $(CFLAGS_DEV) $(LIBS) $(INCLUDES) $(OBJS_DEV_CLIENT) $(LINKER_FLAGS) -o $(DIR_BIN)/dev/client

dev-gui: $(OBJS_DEV_GUI)
	@mkdir -p $(DIR_BIN)/dev
	@$(CC) $(CFLAGS) $(CFLAGS_DEV) $(LIBS) $(INCLUDES) $(OBJS_DEV_GUI) $(LINKER_FLAGS) -o $(DIR_BIN)/dev/client_gui

$(DIR_OBJ)/dev/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@echo $<
	@$(CC) $(CFLAGS) $(CFLAGS_DEV) $(LIBS) $(INCLUDES) $(LINKER_FLAGS) -c -o $@ $<

release: release-server release-client release-gui

release-server: $(OBJS_REL_SERVER)
	@mkdir -p $(DIR_BIN)/release
	@$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LIBS) $(INCLUDES) $(OBJS_REL_SERVER) $(LINKER_FLAGS) -o $(DIR_BIN)/release/server

release-client: $(OBJS_REL_CLIENT)
	@mkdir -p $(DIR_BIN)/release
	@$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LIBS) $(INCLUDES) $(OBJS_REL_CLIENT) $(LINKER_FLAGS) -o $(DIR_BIN)/release/client

release-gui: $(OBJS_REL_GUI)
	@mkdir -p $(DIR_BIN)/release
	@$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LIBS) $(INCLUDES) $(OBJS_REL_GUI) $(LINKER_FLAGS) -o $(DIR_BIN)/release/client_gui

$(DIR_OBJ)/release/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@echo $<
	@$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LINKER_FLAGS) $(LIBS) $(INCLUDES) -c -o $@ $<

-include $(DEPS_DEV)
-include $(DEPS_REL)

clean-data:
	rm -f data/*
clean:
	rm -rf build bin problem1/runs problem1/bin
.PHONY: clean clean-data
