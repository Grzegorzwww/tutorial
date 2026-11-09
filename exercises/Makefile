# Uzycie:
#   make run N=01                 - kompiluje i uruchamia zadanie z katalogu tasks
#   make run N=01 DIR=solutions   - to samo dla rozwiazan
#   make build N=01               - tylko kompilacja
#   make clean
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wno-unused-parameter
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wno-unused-parameter

DIR = tasks
BUILD_DIR = build/$(DIR)
SRC = $(firstword $(wildcard $(DIR)/ex$(N)_*.cpp $(DIR)/ex$(N)_*.c))

ifeq ($(OS),Windows_NT)
SHELL = cmd.exe
.SHELLFLAGS = /C
EXT = .exe
else
EXT =
endif

BIN = $(BUILD_DIR)/$(basename $(notdir $(SRC)))$(EXT)

ifneq ($(MAKECMDGOALS),clean)
ifeq ($(SRC),)
$(error Podaj numer zadania, np.: make run N=01)
endif
endif

.PHONY: build run clean

build: $(BIN)

run: $(BIN)
ifeq ($(OS),Windows_NT)
	@$(subst /,\,$(BIN))
else
	@./$(BIN)
endif

$(BIN): $(SRC) | $(BUILD_DIR)
ifeq ($(suffix $(SRC)),.c)
	$(CC) $(CFLAGS) $< -o $@
else
	$(CXX) $(CXXFLAGS) $< -o $@
endif

$(BUILD_DIR):
ifeq ($(OS),Windows_NT)
	@if not exist "$(subst /,\,$@)" mkdir "$(subst /,\,$@)"
else
	@mkdir -p "$@"
endif

clean:
ifeq ($(OS),Windows_NT)
	@if exist build rmdir /S /Q build
else
	rm -rf build
endif
