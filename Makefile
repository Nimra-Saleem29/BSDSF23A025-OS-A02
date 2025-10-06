# --------------------------------------------
# Makefile for Feature-5 (ls-v1.4.0 — Alphabetical Sort)
# --------------------------------------------

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -O2

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source file for Feature 5
SRC = $(SRC_DIR)/lsv1.4.0.c
OBJ = $(OBJ_DIR)/lsv1.4.0.o
TARGET = $(BIN_DIR)/ls

# Default rule
all: $(TARGET)

# Ensure directories exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile source file
$(OBJ): $(SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

# Link object to binary
$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# --------------------------------------------
# Usage:
#   make        → build v1.4.0 binary
#   make clean  → remove obj/bin folders
# --------------------------------------------
