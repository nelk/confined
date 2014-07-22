SOURCES = $(wildcard common/*.cpp) $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DEPENDS = $(SOURCES:.cpp=.d)
LDFLAGS = -rdynamic -lGLEW -lGLU -lGL -lSM -lICE -lXext -lglfw3 -lXrandr -lrt -ldl -pthread -lXi -lX11 -lXxf86vm -lm -lassimp -lfreeimage -lopenal -lalut
CPPFLAGS = -Isrc #-Iexternal/glm-0.9.4.0/glm -Iexternal/glfw-3.0.3/include -Iexternal/glew-1.9.0/include
CXXFLAGS = $(CPPFLAGS) -W -Wall -g
CXX = g++
MAIN = confined

all: $(MAIN)

depend: $(DEPENDS)

clean:
	rm -f src/*.o src/*.d $(MAIN)

$(MAIN): $(OBJECTS)
	@echo Creating $@...
	@$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	@echo Compiling $<...
	@$(CXX) -o $@ -c $(CXXFLAGS) $<

%.d: %.cpp
	@echo Building $@...
	@set -e; $(CC) -M $(CPPFLAGS) $< \
                  | sed 's,\($(*)\)\.o[ :]*,\1.o $@ : ,g' > $@; \
                [ -s $@ ] || rm -f $@

include $(DEPENDS)
