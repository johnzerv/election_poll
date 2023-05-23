# Paths
OBJ_DIR = ./src
INC_DIR = ./include
RESULTS_DIR = ./results

# Flags and compiler
CPPFLAGS = -Wall -Wno-pointer-arith -I $(INC_DIR) -lpthread
CC = g++

# All Object Files
OBJ_SERVER = $(OBJ_DIR)/server.o
OBJ_HELPERS = $(OBJ_DIR)/helpers.o
OBJ_MASTER = $(OBJ_DIR)/master.o
OBJ_WORKER = $(OBJ_DIR)/worker.o
OBJ_CLIENT = $(OBJ_DIR)/client.o

# Server's Object Files
OBJS_SERVER = $(OBJ_SERVER) $(OBJ_HELPERS) $(OBJ_MASTER) $(OBJ_WORKER)

# Client's Object Files
OBJS_CLIENT = $(OBJ_CLIENT) $(OBJ_HELPERS)

# Executables
EXEC_SERVER = poller
EXEC_CLIENT = pollSwayer

# Define all targets
all: target_server target_client

# And each one individually
target_server: $(EXEC_SERVER)

$(EXEC_SERVER): $(OBJS_SERVER)
	@$(CC) $(CPPFLAGS) $(OBJS_SERVER) -o $(EXEC_SERVER)

target_client: $(EXEC_CLIENT)

$(EXEC_CLIENT): $(OBJS_CLIENT)
	@$(CC) $(CPPFLAGS) $(OBJS_CLIENT) -o $(EXEC_CLIENT)

.SILENT: $(OBJS_SERVER) # Silence implicit rule output
.PHONY: clean

clean:
	@echo "Cleaning up ..."
	@rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(EXEC_SERVER) $(EXEC_CLIENT) $(RESULTS_DIR)/*

# run: $(EXEC_SERVER)
# 	./$(EXEC_SERVER)
