all: server.out client.out

client.out: Computer1.cpp 
	g++ -pthread -o client.out Computer1.cpp 

server.out: Computer2.cpp 
	g++ -pthread -o server.out Computer2.cpp 
clean:
	rm server.out client.out