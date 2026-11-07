CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc

SRC_DIR = src
BIN_DIR = bin

TARGET_BF = $(BIN_DIR)/brute_force
TARGET_GA = $(BIN_DIR)/genetic
TARGET_GREEDY = $(BIN_DIR)/greedy

all: directories $(TARGET_BF) $(TARGET_GA) $(TARGET_GREEDY)

directories:
	@mkdir -p $(BIN_DIR)

$(TARGET_BF): $(SRC_DIR)/brute_force/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TARGET_GA): $(SRC_DIR)/genetic/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TARGET_GREEDY): $(SRC_DIR)/greedy/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean directories
