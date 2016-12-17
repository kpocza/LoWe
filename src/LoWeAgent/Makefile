CPP=g++
CFLAGS=-std=c++11 -Wall -I. -IDeviceHandlers 
#CFLAGS=-std=c++11 -g3 -Wall -I. -IDeviceHandlers 
OUT=loweagent
OBJDIR=obj
OUTDIR=out
DH=DeviceHandlers/

_OBJ=loweagent.o DeviceHandler.o DeviceHandlerRegistry.o DeviceHandlerFactory.o ConfigHandler.o PidGuesser.o \
		ProgRuntimeHandler.o ProgRuntimeDispatcher.o SocketCommunicator.o DeviceProvisioner.o \
		ArgsParser.o CommunicatingDeviceHandler.o SigActions.o \
		$(DH)DeviceHandlerCatchAll.o $(DH)DeviceHandlerFrameBuffer.o $(DH)DeviceHandlerTTY.o \
		$(DH)DeviceHandlerALSA.o $(DH)DeviceHandlerMice.o $(DH)DeviceHandlerKeyboard.o \
		$(DH)FrameBufferTransporter.o
OBJ=$(patsubst %,$(OBJDIR)/%,$(_OBJ))

INC=DeviceHandler.h DeviceHandlerFactory.h DeviceHandlerRegistry.h ConfigHandler.h PidGuesser.h \
		ProgRuntimeHandler.h ProgRuntimeDispatcher.h SocketCommunicator.h Log.h DeviceProvisioner.h \
		ArgsParser.h CommunicatingDeviceHandler.h SigActions.h \
		$(DH)DeviceHandlerCatchAll.h $(DH)DeviceHandlerFrameBuffer.h $(DH)DeviceHandlerTTY.h \
		$(DH)DeviceHandlerALSA.h $(DH)DeviceHandlerMice.h $(DH)DeviceHandlerKeyboard.h \
		$(DH)FrameBufferTransporter.h

all: credirs build

$(OBJ): $(OBJDIR)/%.o: %.cpp $(INC) Makefile
	$(CPP) $< -c -o $@ $(CFLAGS)

credirs:
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/$(DH)
	mkdir -p $(OUTDIR)

build: $(OBJ)
	$(CPP) -o $(OUTDIR)/$(OUT) $^ -lconfig -lpthread
	cp loweagent.conf $(OUTDIR)

clean:
	rm -R -f $(OBJDIR)
	rm -R -f $(OUTDIR)

