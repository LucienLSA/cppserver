server:
	g++ util.cpp client03.cpp -o client && \
	g++ util.cpp server03.cpp Epoll.cpp InetAddress.cpp Socket.cpp Channel.cpp -o server
clean:
	rm server && rm client