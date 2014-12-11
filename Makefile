CXX=c++
CXXFLAGS=-pedantic -Wall -std=c++11 -g -O2 -MMD -Wno-write-strings

SRCDIR=src
OBJFILES=$(SRCDIR)/main.o $(SRCDIR)/colorbuffer.o $(SRCDIR)/lodepng.o $(SRCDIR)/raytracer.o

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
BIN=demo
else
BIN=demo.exe
endif

.PHONY:all clean

all: $(BIN) $(ANIMBIN)

$(BIN): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f $(SRCDIR)/*.o $(SRCDIR)/*.d $(BIN)

-include $(OBJFILES:.o=.d)
