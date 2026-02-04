# Makefile for CG_TP_2

CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
INCLUDES := -Isrc
LIBS := -lGL -lGLEW -lglfw -lpng

SRC_DIR := src
BUILD_DIR := build

SOURCES := CG_TP_2.cpp \
           $(SRC_DIR)/Model.cpp \
           $(SRC_DIR)/ObjLoader.cpp \
           $(SRC_DIR)/ShaderProgram.cpp \
           $(SRC_DIR)/TextureLoader.cpp

OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(SOURCES)))

TARGET := $(BUILD_DIR)/CG_TP_2

.PHONY: all clean run assets

all: $(TARGET) assets

$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(BUILD_DIR)/CG_TP_2.o: CG_TP_2.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -DPROJECT_SOURCE_DIR=\"$(CURDIR)\" -c $< -o $@

$(BUILD_DIR)/Model.o: $(SRC_DIR)/Model.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/ObjLoader.o: $(SRC_DIR)/ObjLoader.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/ShaderProgram.o: $(SRC_DIR)/ShaderProgram.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/TextureLoader.o: $(SRC_DIR)/TextureLoader.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

assets: | $(BUILD_DIR)
	@echo "Copying UFO assets..."
	@cp -r UFO $(BUILD_DIR)/
	@echo "Copying shader files..."
	@mkdir -p $(BUILD_DIR)/shaders
	@cp assets/shaders/* $(BUILD_DIR)/shaders/

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
