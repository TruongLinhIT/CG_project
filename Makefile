# ============================================================================
# Makefile for 3D Simulation of Asia Park (Da Nang) - C++ / FreeGLUT
# Supports Windows (MSYS2 / MinGW64 / UCRT64) and Linux / WSL2
# ============================================================================

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Isrc
TARGET   := AsiaPark3D

ifeq ($(OS),Windows_NT)
    TARGET := AsiaPark3D.exe
    LDFLAGS := -lfreeglut -lopengl32 -lglu32 -lm
else
    LDFLAGS := -lglut -lGL -lGLU -lEGL -lm
endif

SRCS := src/main.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
ifeq ($(OS),Windows_NT)
	del /Q $(TARGET) 2>nul || exit 0
else
	rm -f $(TARGET)
endif
