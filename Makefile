CC      = gcc
CFLAGS  = -Wall
LDFLAGS = -lpthread
TARGET  = proxy

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(TARGET).o

$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $(TARGET).c

clean:
	rm $(TARGET) *.o