all: *.cpp
	clang++ -std=c++11 ./main.cpp ./helper.cpp -o kage
clean:
	rm ./kage
