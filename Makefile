# file to build
source = snake.cpp

# file to output
target = SNAKE

compiler = g++

# compiler flags
flags = -std=c++17

# build file
build:
	$(compiler) $(flags) $(source) -o $(target)

# "clean" and "test" are not produced by make
.PHONY: clean test

# delete built files
clean:
	rm -f $(target)

# check if the programs run correctly
test: build
	@echo
	./$(target)
	@echo Game Ran Successfully
