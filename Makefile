build: main.cpp a5.cpp a5.hpp core.cpp
	g++ main.cpp a5.cpp core.cpp -o main

clean:
	$(RM) main