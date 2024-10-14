# Author: David Kopelent
# Title: Makefile

.PHONY: all compile run check doc clean
.DEFAULT_GOAL = all

## Variables and definitions

CXX = g++
CXXFLAGS = -std=c++20 -Wall -pedantic -Wno-long-long -Werror -O2 -ggdb

## Change this according to the location of the expression parser
LDFLAGS = -L/home/david/Desktop/FIT/PA2/2024/kopeldav/fitexcel/x86_64-linux-gnu -l:libexpression_parser.a

EXECUTABLE = excel
SOURCES := $(filter-out all_in_one.cpp, $(wildcard *.cpp))
OBJECTS := $(SOURCES:.cpp=.o)
CHECK = valgrind
CHECKFLAGS = --leak-check=full --tool=memcheck --show-leak-kinds=all

## Output colors

BLUE = \033[1;36m
GREEN = \033[1;32m
COLOR_END = \033[0m

## Tasks

all: doc compile

%.o: %.cpp
	@echo "$(BLUE)$^ -> $@$(COLOR_END)"
	$(CXX) -c $(CXXFLAGS) $< -o $@ $(LDFLAGS)
	@$(CXX) $(CXXFLAGS) -MM -MT $*.o $*.cpp

run: compile
	@./$(EXECUTABLE)
	@echo "$(GREEN)Program executed successfully!$(COLOR_END)"

doc: 
	@echo "$(BLUE)Generating documentation:$(COLOR_END)"
	doxygen Doxyfile
	@echo "$(GREEN)Documentation created successfully!$(COLOR_END)"

compile: $(OBJECTS)
	@echo "$(BLUE)Compiling ./$(EXECUTABLE) using '$(CXXFLAGS) $(LDFLAGS)' flags:$(COLOR_END)"
	@$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)
	@echo "$(GREEN)Compilation successfull!$(COLOR_END)"

check: CXXFLAGS += -g
check: clean compile
	@echo "$(BLUE)Preparing for program check using '$(CHECK)':$(COLOR_END)"
	$(CHECK) ./$(EXECUTABLE) $(CHECKFLAGS)
	@echo "$(GREEN)Check finished!$(COLOR_END)"

clean:
	@echo "$(BLUE)Removing object files$(COLOR_END)"
	@rm -f -- *.o
	@echo "$(BLUE)Removing executables$(COLOR_END)"
	@rm -f $(EXECUTABLE)
	@rm -f a.out
	@echo "$(BLUE)Removing documentation$(COLOR_END)"
	@rm -rf -- doc/
	@echo "$(GREEN)Clean successfull!$(COLOR_END)"
