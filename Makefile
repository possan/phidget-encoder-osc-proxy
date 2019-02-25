all: proxy

clean:
	rm proxy

proxy: proxy.o
	gcc proxy.o -o proxy -F/Library/Frameworks -framework Phidget21

proxy.o: proxy.cpp
	gcc proxy.cpp -c -o proxy.o -F/Library/Frameworks -framework Phidget21 -I/Library/Frameworks/Phidget21.framework/Headers
