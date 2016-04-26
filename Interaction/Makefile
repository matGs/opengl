CC = gcc
LD = gcc

OBJ = Interaction.o LoadShader.o Matrix.o StringExtra.o OBJParser.o List.o
TARGET = Interaction

CFLAGS = -g -Wall 
LDLIBS = -lm -lglut -lGLEW -lGL -lGLU
INCLUDES = -Isource

SRC_DIR = source
BUILD_DIR = build
VPATH = source

# Rules
all: $(TARGET)

$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

clean:
	rm -f $(BUILD_DIR)/*.o *.o $(TARGET) 

.PHONY: clean

# Dependencies
$(TARGET): $(BUILD_DIR)/LoadShader.o $(BUILD_DIR)/Matrix.o $(BUILD_DIR)/StringExtra.o $(BUILD_DIR)/OBJParser.o  $(BUILD_DIR)/List.o | $(BUILD_DIR)



