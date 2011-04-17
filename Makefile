CXX = g++
CC  = gcc
LD  = g++
CPP = cpp

ERROR_FILTER := 2>&1 | sed -e 's/\(.[a-zA-Z]\+\):\([0-9]\+\):/\1(\2):/g'

CFLAGS =-ggdb -Iinclude -Wall  `sdl-config --cflags` -Isrc -I/home/ska/local/include
DEFINES =
LDFLAGS = $(GCOV) `sdl-config --libs` -lSDL_ttf -lSDL_image -lpng -lmpdclient -L/home/ska/local/lib


CPP_SRCS=src/dialogue_box.cpp src/timer.cpp src/main.cpp \
	src/gui.cpp src/menu.cpp src/utils.cpp \
	src/listener.cpp src/status_bar.cpp src/widget.cpp


OBJS=$(patsubst %.cpp,objs/%.o,$(CPP_SRCS)) $(patsubst %.c,objs/%.o,$(C_SRCS))
DEPS=$(patsubst %.cpp,deps/%.d,$(CPP_SRCS)) $(patsubst %.c,deps/%.d,$(C_SRCS))

TARGET=menu


all: $(DEPS) $(TARGET)


deps/%.d: %.cpp
	@echo makedep $(notdir $<)
	@install -d deps/$(dir $<)
	@$(CPP) -M -MT objs/$(patsubst %.cpp,%.o,$<) $(DEFINES) $(CFLAGS) -o $@ $<

objs/%.o: %.cpp
	@echo CXX $(notdir $<)
	@install -d objs/$(dir $<)
	@$(CXX) $(CFLAGS) $(DEFINES) -c -o $@ $< $(ERROR_FILTER)

clean:
	rm -rf $(TARGET) *~ objs deps


$(TARGET): $(OBJS)
	@echo LD $@
	@$(LD) $(LDFLAGS) -o $@ $+ $(LDFLAGS)

-include $(DEPS)
