INC:=. munit ../Inc
INC_PARAMS:=$(addprefix -I, $(INC))
vpath %.h $(INC)

BUILD_DIR:=build
EXECUTABLE:=test
TARGET:=$(BUILD_DIR)/$(EXECUTABLE)

CSRC:=test_main.c test_bal.c test_volt_data.c munit.c bal.c
OBJ:=$(addprefix $(BUILD_DIR)/, $(CSRC:.c=.o))

vpath %.c .
vpath %.c munit
vpath %.c ../Src

CC?=gcc

CFLAGS=$(INC_PARAMS) -Wall# -fprofile-arcs -ftest-coverage

.PHONY: all
all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $@

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJ)
	rm -d $(BUILD_DIR)

.PHONY: test
test: all
	$(TARGET)
