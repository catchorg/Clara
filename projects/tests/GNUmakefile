TEST_PROGRAM := clara_test
OBJECTS := main.o tests.o

PYTHON := python

ROOT_DIR := ../..
CLARA_H_DIR := $(ROOT_DIR)/include
CLARA_H := $(CLARA_H_DIR)/clara.h

CPPFLAGS := -I$(CLARA_H_DIR)

all: $(TEST_PROGRAM)

$(OBJECTS): catch.hpp $(CLARA_H)

$(CLARA_H): $(wildcard $(ROOT_DIR)/srcs/*.h)
	$(PYTHON) $(ROOT_DIR)/scripts/stitch.py

$(TEST_PROGRAM): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

check: $(TEST_PROGRAM)
	./$<

clean:
	$(RM) $(TEST_PROGRAM) $(OBJECTS)

.PHONY: all check clean
