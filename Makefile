app := udpviewer

CXX 	:= g++
LD		:= ld
CXXFLAGS:= -g -std=c++11
LDFLAGS := -g 
LDLIBS 	:= -lm -lX11 -lpthread

SRCS := $(shell find . -name "*.cpp")
OBJS := $(subst .cpp,.o,$(SRCS))

all: $(app)

$(app): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(app) $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

cleanall: clean
	$(RM) *~ .depend
	$(RM) $(app)

include .depend
