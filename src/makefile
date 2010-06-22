CC        = g++
CFLAGS    = -O3 -D_OSX_
OBJPATH  = .obj/json
COMPILE  = $(CC) $(CFLAGS) -c
LDFLAGS  = -lpthread
OBJFILES := $(JSON_handler.c,patsubst %.cpp,$(OBJPATH)/%.o,$(wildcard json/*.cpp))
LIBS    :=
INCPATH  = -I. -I/usr/include -I/usr/local/include -I/usr
BASEPATH  = 

all: dirs libJSON.a

dirs:
	if [ ! -e .obj ]; then mkdir .obj; fi;
	if [ ! -e .obj/json ]; then mkdir .obj/json; fi;

clean:
	rm $(OBJFILES) libJSON.a

objectify:
	$(OBJPATH)/%.o: json/%.cpp
		$(COMPILE) $(INCPATH) -o $@ $<

libJSON.a: $(OBJFILES)
	ar rcs libJSON.a $(OBJFILES)
