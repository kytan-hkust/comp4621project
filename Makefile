CXX      = g++
CXXFLAGS = -Wall -O2 -std=c++11
LDFLAGS  = -lpthread
TARGET   = proxy

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(TARGET).o

$(TARGET).o: $(TARGET).cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $(TARGET).cpp

clean:
	rm $(TARGET) *.o