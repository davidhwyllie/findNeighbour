all: program

program: 
	g++ -std=c++11 -fopenmp -O3 findNeighbour.cpp -lz -o findNeighbour 

clean:
	rm -rf *o findNeighbour

windows_64:
	x86_64-w64-mingw32-g++ -std=c++11 -fopenmp findNeighbour.cpp -o findNeighbour_64.exe

windows_32:
	i686-w64-mingw32-g++ -std=c++11 -fopenmp findNeighbour.cpp -o findNeighbour_32.exe
