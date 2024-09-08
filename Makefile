CC = gcc

TARGET = client.exe

SRC = client.c
 
all: $(TARGET) 
$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)