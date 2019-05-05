all: server
server: server.o httpUtils.o
	g++ server.o httpUtils.o -pthread -o server
server.o: server.cpp
	g++ -c server.cpp
httpUtils.o: httpUtils.h
	g++ -c httpUtils.cpp

clean :
	rm *.o
