# Makefile for compiling only lsv1.5.0.c

CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -O2

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRC = $(SRC_DIR)/lsv1.5.0.c
OBJ = $(OBJ_DIR)/lsv1.5.0.o
TARGET = $(BIN_DIR)/ls

all: $(TARGET)

# Create directories if needed
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile .c to .o
$(OBJ): $(SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

# Link .o to executable
$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
