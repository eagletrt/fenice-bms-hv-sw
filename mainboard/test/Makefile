INC:=. munit ../Inc
INC_PARAMS:=$(addprefix -I, $(INC))
vpath %.h $(INC)

BUILD_DIR:=build
EXECUTABLE:=mainboard_test
TARGET:=$(BUILD_DIR)/$(EXECUTABLE)

CSRC:=main.c test_bal.c test_volt_data.c munit.c bal.c
OBJ:=$(addprefix $(BUILD_DIR)/, $(CSRC:.c=.o))

vpath %.c .
vpath %.c munit
vpath %.c ../Src

CC?=gcc

CFLAGS=$(INC_PARAMS) -Wall -fprofile-arcs -ftest-coverage

.PHONY: all
all: $(TARGET) test

test:
	$(TARGET)
	@gcov -n build/bal

$(BUILD_DIR):
	mkdir -p $@

$(TARGET): $(OBJ)
	@mkdir -p $(@D)
	$(CC) -o $@ $^ $(CFLAGS)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJ)
	rm -f $(OBJ:.o=.gcda) $(OBJ:.o=.gcno)
	rm -d $(BUILD_DIR)

