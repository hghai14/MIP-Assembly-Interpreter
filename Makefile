OUT = main

CC := g++

OUTPUT_DIR := ./output
SRC_DIR := ./src

OBJS := $(OUTPUT_DIR)/main.o  $(OUTPUT_DIR)/Core.o  $(OUTPUT_DIR)/dram.o  $(OUTPUT_DIR)/helper.o


build: $(OBJS)
	$(CC) -o $(OUT) $^ 

$(OUTPUT_DIR)/main.o: $(SRC_DIR)/main.cpp
	$(CC) $< -c -o $@ 

$(OUTPUT_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $< -c -o $@ 

clean:
	$(RM) main