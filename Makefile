all:
	clang++ -Wall -g -include prelude.hpp -fsanitize=undefined,address main.cc && ./a.out
d: 
	clang++ -Wall -g -include prelude.hpp main.cc && ./a.out
	
run:
	./a.out
