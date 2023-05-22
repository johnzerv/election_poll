# Paths
OBJ_DIR = ./src
INC_DIR = ./include
RESULTS_DIR = ./results

# Flags and compiler
CPPFLAGS = -Wall -Wno-pointer-arith -I $(INC_DIR) -lpthread
CC = g++

# Objects
OBJ_SERVER = $(OBJ_DIR)/server.o
OBJ_HELPERS = $(OBJ_DIR)/helpers.o
OBJ_MASTER = $(OBJ_DIR)/master.o
OBJ_WORKER = $(OBJ_DIR)/worker.o
OBJS = $(OBJ_SERVER) $(OBJ_HELPERS) $(OBJ_MASTER) $(OBJ_WORKER)

# Executables
EXEC_SERVER = server

# Define all targets
all: target_server

# And each one individually
target_server: $(EXEC_SERVER)

$(EXEC_SERVER): $(OBJS)
	@$(CC) $(CPPFLAGS) $(OBJS) -o $(EXEC_SERVER)

.SILENT: $(OBJS) # Silence implicit rule output
.PHONY: clean

clean:
	@echo "Cleaning up ..."
	@rm -f $(OBJS) $(EXEC_SERVER) $(RESULTS_DIR)/*

# run: $(EXEC_SERVER)
# 	./$(EXEC_SERVER)
