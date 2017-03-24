PROJECT = read_snapshot
CC = gcc

COMPILE_OPTIONS = -Wall -O2

#Headers
HEADERS = 

#Libraries
LIBS = 

# Subdirs to search for additional source files
SUBDIRS := $(shell ls -F | grep "\/" )
DIRS := ./ $(SUBDIRS)
SOURCE_FILES := $(foreach d, $(DIRS), $(wildcard $(d)*.c) )

# Create an object file of every c file
OBJECTS = $(patsubst %.c, %.o, $(SOURCE_FILES))

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) -o $(PROJECT) $(OBJECTS) $(LIBS)

%.o: %.c
	$(CC) -c $(COMPILE_OPTIONS) -o $@ $< $(HEADERS)

clean:
	rm -f $(PROJECT) $(OBJECTS)

.PHONY: depclean
depclean:
	rm -f $(DEPENDENCIES)

clean-all: clean depclean
