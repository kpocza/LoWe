CPP=g++
CFLAGS=-std=c++11 -Wall -I. -IDeviceHandlers 
#CFLAGS=-std=c++11 -g3 -Wall -I. -IDeviceHandlers 
OUT=loweagent
ODIR=obj
DH=DeviceHandlers/

_OBJ=loweagent.o DeviceHandler.o DeviceHandlerRegistry.o DeviceHandlerFactory.o ConfigHandler.o PidGuesser.o \
		ProgRuntimeHandler.o ProgRuntimeDispatcher.o SocketCommunicator.o DeviceProvisioner.o \
		ArgsParser.o CommunicatingDeviceHandler.o \
		$(DH)DeviceHandlerCatchAll.o $(DH)DeviceHandlerFrameBuffer.o $(DH)DeviceHandlerTTY.o \
		$(DH)DeviceHandlerALSA.o $(DH)DeviceHandlerMice.o $(DH)DeviceHandlerKeyboard.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

INC=DeviceHandler.h DeviceHandlerFactory.h DeviceHandlerRegistry.h ConfigHandler.h PidGuesser.h \
		ProgRuntimeHandler.h ProgRuntimeDispatcher.h SocketCommunicator.h Log.h DeviceProvisioner.h \
		ArgsParser.h CommunicatingDeviceHandler.h \
		$(DH)DeviceHandlerCatchAll.h $(DH)DeviceHandlerFrameBuffer.h $(DH)DeviceHandlerTTY.h \
		$(DH)DeviceHandlerALSA.h $(DH)DeviceHandlerMice.h $(DH)DeviceHandlerKeyboard.h

all: creobjdir build

$(OBJ): $(ODIR)/%.o: %.cpp $(INC) Makefile
	$(CPP) $< -c -o $@ $(CFLAGS)

creobjdir:
	mkdir -p $(ODIR)
	mkdir -p $(ODIR)/$(DH)

build: $(OBJ)
	$(CPP) -o $(OUT) $^ -lconfig 

clean:
	rm -R -f $(ODIR)
	rm -f $(OUT)

