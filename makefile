all: clean compile run

compile: main.cpp
	
	 g++ -std=c++11 -o shell main.cpp 
run:
	
	@./shell
	
clean:
	
	@rm -f *.o
	@rm -f shell
