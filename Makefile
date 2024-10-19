CC = gcc

TARGET = main.exe

SRC = client.c
QUERIES = helper_queries.c 
UTILS = helper_utilities.c 
 
all: $(TARGET) 
$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(QUERIES) $(UTILS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
