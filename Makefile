
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

TARGET = raycast

SRC = ray_casting.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)


clean:
	rm -f $(TARGET)