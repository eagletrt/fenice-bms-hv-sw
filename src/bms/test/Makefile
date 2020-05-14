MUNIT_SOURCE=munit/munit.c
LIB_INCLUDE=munit

BUILD_DIR=build
EXECUTABLE=main
TARGET=$(BUILD_DIR)/$(EXECUTABLE)

SOURCES=main.c
OBJECTS=test_list.o
OBJ=$(BUILD_DIR)/$(OBJECTS)

CC?=gcc

CFLAGS=-I $(LIB_INCLUDE)

.PHONY: all
all: $(TARGET)

$(TARGET): $(SOURCES) $(OBJ)
	$(CC) -o $@ $^ $(MUNIT_SOURCE) $(CFLAGS)

$(BUILD_DIR)/%.o: %.c
	$(CC) -c -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJ)

.PHONY: test
test: all
	$(TARGET)
