server:
	g++ src/util.cpp client03.cpp -o client && \
	g++ src/util.cpp server06.cpp src/Epoll.cpp src/InetAddress.cpp src/Socket.cpp src/Channel.cpp src/EventLoop.cpp src/Server.cpp -o server
clean:
	rm server && rm client