all: edge.cpp server_and.cpp server_or.cpp client.cpp
	g++ -std=c++0x edge.cpp -o edge_hh -lnsl -lresolv -lm
	g++ -std=c++0x server_and.cpp -o server_and -lnsl -lresolv -lm
	g++ -std=c++0x server_or.cpp -o server_or -lnsl -lresolv -lm
	g++ -std=c++0x client.cpp -o client -lnsl -lresolv -lm

edge:
	./edge_hh
serverAnd:
	./server_and
serverOr:
	./server_or
