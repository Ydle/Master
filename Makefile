CC=g++
CFLAGS=-c -Wall -D RASPBERRY -std=gnu++0x -fpermissive

LDFLAGS=  -lwiringPi  -lpthread -lmicrohttpd -ljsoncpp -lcurl

SOURCES=src/master.cpp src/IhmCommunicationThread.cpp src/RestBrowser.cpp src/logging.cpp src/protocolRF.cpp src/webServer.cpp src/NodeRequestHandler.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=ydlemaster

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC)  $(OBJECTS) $(LDFLAGS) -o $@

src/IhmCommunicationThread.o: src/RestBrowser.h
src/master.o: src/master.h src/protocolRF.h src/webServer.h
src/logging.o: src/logging.h 
src/webServer.o: src/webServer.h 
src/NodeRequestHandler.o: src/NodeRequestHandler.h
src/node.o: src/master.h src/protocolRF.h
src/RestBrowser.o: src/RestBrowser.h
src/protocolRF.o:  src/protocolRF.h

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf src/*.o
	rm -rf src/httpd/*.o
	rm -rf $(EXECUTABLE)
