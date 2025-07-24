server:
	g++ util.cpp client.cpp -o client && \
	g++ util.cpp server2.cpp Epoll.cpp InetAddress.cpp Socket.cpp -o server2
clean:
	rm server2 && rm client