CXX      ?= g++
CXXFLAGS ?= -std=c++11 -O2 -Wno-deprecated
INCLUDES := -I. -I./Leap -I./Model

ifeq ($(shell uname),Darwin)
GL_LIBS := -framework GLUT -framework OpenGL
else
GL_LIBS := -lglut -lGLU -lGL
endif

# Leap Motion SDK v2.3.1 (proprietary, not in this repo): make client LEAP_SDK=/path/to/LeapSDK
LEAP_SDK ?=
LEAP_LIB  = $(LEAP_SDK)/lib/x64/libLeap.so

# Everything that builds without the Leap Motion SDK
all: server tortuga smooth check-client

server: server.cpp utilidad.h
	$(CXX) $(CXXFLAGS) -o $@ server.cpp -pthread

# Compile-only check of the clients (linking needs libLeap)
check-client:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c client.cc -o client.o
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c main2.cc -o main2.o

client main2: %: %.cc
	@test -n "$(LEAP_SDK)" || { echo "set LEAP_SDK=/path/to/LeapSDK (Leap Motion SDK v2.3.1)"; exit 1; }
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(GL_LIBS) $(LEAP_LIB) -Wl,-rpath,$(LEAP_SDK)/lib/x64 -lpng -pthread

TORTUGA_SRC := $(addprefix iluminandoTortuga/,main.cpp Camara.cpp light.cpp primitivas.cpp Vector_tools.cpp)

tortuga: $(TORTUGA_SRC)
	$(CXX) $(CXXFLAGS) -o iluminandoTortuga/iluminandoTortuga $(TORTUGA_SRC) $(GL_LIBS)

smooth: iluminandoTortuga/smooth.cpp
	$(CXX) $(CXXFLAGS) -o iluminandoTortuga/smooth $< $(GL_LIBS)

clean:
	rm -f server client main2 *.o iluminandoTortuga/iluminandoTortuga iluminandoTortuga/smooth

.PHONY: all check-client tortuga smooth clean
