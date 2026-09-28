# BTTF CPP - Top Down Shooter Makefile

# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I.

# Libraries
ifeq ($(OS),Windows_NT)
    LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET_EXT = .exe
else
    LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    TARGET_EXT =
endif

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Target
TARGET = bttf_shooter$(TARGET_EXT)

# Source files
SRCS = $(SRC_DIR)/main.cpp

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Colors for output
GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[0;33m
NC = \033[0m # No Color

# Default target
all: $(TARGET)
	@echo "$(GREEN)✓ Build complete!$(NC)"
	@echo "$(YELLOW)Run with: ./$(TARGET)$(NC)"

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(OBJS)
	@echo "$(YELLOW)Linking $(TARGET)...$(NC)"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

# Clean build files
clean:
	@echo "$(RED)Cleaning build files...$(NC)"
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)
	@echo "$(GREEN)✓ Clean complete!$(NC)"

# Rebuild
re: clean all

# Run the game
run: all
	@echo "$(GREEN)Starting game...$(NC)"
	@./$(TARGET)

# Help
help:
	@echo "$(YELLOW)BTTF CPP - Top Down Shooter$(NC)"
	@echo ""
	@echo "Available targets:"
	@echo "  $(GREEN)all$(NC)     - Build the game (default)"
	@echo "  $(GREEN)clean$(NC)   - Remove build files"
	@echo "  $(GREEN)re$(NC)      - Rebuild everything"
	@echo "  $(GREEN)run$(NC)     - Build and run the game"
	@echo "  $(GREEN)help$(NC)    - Show this help message"
	@echo ""
	@echo "Dependencies:"
	@echo "  - Raylib library"
	@echo "  - C++17 compatible compiler"

.PHONY: all clean re run help