CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror

BUILD_DIR = build
SRC_DIR = main.cpp
TARGET = $(BUILD_DIR)/a.out
OBJS = $(SRC_DIR:.cpp=.o)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD_DIR)