CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc
BIN_DIR = bin

ifeq ($(OS),Windows_NT)
	EXT = .exe
	MKDIR = if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	RM = if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)
else
	EXT =
	MKDIR = mkdir -p $(BIN_DIR)
	RM = rm -rf $(BIN_DIR)
endif

TARGETS = $(BIN_DIR)/brute_force$(EXT) $(BIN_DIR)/genetic$(EXT) $(BIN_DIR)/greedy$(EXT)

all: $(TARGETS)

$(BIN_DIR)/brute_force$(EXT): src/brute_force/main.cpp src/task.hpp
	@$(MKDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BIN_DIR)/genetic$(EXT): src/genetic/main.cpp src/task.hpp
	@$(MKDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BIN_DIR)/greedy$(EXT): src/greedy/main.cpp src/task.hpp
	@$(MKDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	$(RM)

.PHONY: all clean