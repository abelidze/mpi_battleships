PROJECT = game
BUILD   = mpic++
MAKEDIR = mkdir
REMOVE  = rm -rf
OUT_DIR = bin
OUTPUT  = ./$(OUT_DIR)/$(PROJECT)

DBGFLAGS = -g
CFLAGS  = -flto
#-I.\include
LDFLAGS = -flto
#-L. -l:pdcurses.a -s

EXCLUDE = table_printer.cpp
TARGETS = main.cpp
ALLSRCS = $(wildcard *.cpp)
SOURCES = $(filter-out $(TARGETS) $(EXCLUDE),$(ALLSRCS))
OBJECTS = $(SOURCES:.cpp=.o)

.SECONDEXPANSION:
.PHONY: all debug build list clean run

all: build clean

list:
	@echo -e "Targets: $(TARGETS)\nSources: $(SOURCES)\nObjects: $(OBJECTS)"

debug: CFLAGS += $(DBGFLAGS)
debug: build

build: $(OUT_DIR) main

run:
	@mpiexec -n 1 ${OUTPUT}

$(OUT_DIR):
	$(MAKEDIR) $(OUT_DIR)

$(TARGETS:.cpp=): $(OBJECTS) $$(@).o
	$(BUILD) -o $(OUTPUT) $(OBJECTS) $(@).o $(LDFLAGS)
# $(BUILD) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(BUILD) -o $@ -c $< $(CFLAGS)

clean:
	$(REMOVE) *.o *.gc*
