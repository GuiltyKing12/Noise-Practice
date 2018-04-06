########################################
## SETUP MAKEFILE
##      Set the appropriate TARGET (our
## executable name) and any OBJECT files
## we need to compile for this program.
##
## Next set the path to our local
## include/, lib/, and bin/ folders.
## (If you we are compiling in the lab,
## then you can ignore these values.
## They are only for if you are
## compiling on your personal machine.)
##
## Set if we are compiling in the lab
## environment or not.  Set to:
##    1 - if compiling in the Lab
##    0 - if compiling at home
##
## Finally, set the flags for which
## libraries are using and want to
## compile against.
########################################

BUILDING_IN_LAB = 0

TARGET = bHer_A3
OBJECTS = main.o src/OpenGLUtils.o src/ShaderProgram.o src/ShaderUtils.o src/BezierSurfaceUtils.o

ifeq ($(BUILDING_IN_LAB),1)
	CXX    = C:/Rtools/mingw_64\bin\g++.exe
else
	CXX = g++
endif

CFLAGS = -Wall -g

INCPATH += -I./include
LIBPATH += -L./lib

ifeq ($(BUILDING_IN_LAB),0)
	LIBPATH += -L/Users/BrandonHer/Codes/lib/libs
	INCPATH += -I/Users/BrandonHer/Codes/lib/includes
	LIBPATH += -L/usr/local/lib
	INCPATH += -I/usr/local/include
endif

#############################
## SETUP OpenGL & GLUT 
#############################

ifeq ($(BUILDING_IN_LAB),1)
	INCPATH += -IC:/sw/opengl/include
	LIBPATH += -LC:/sw/opengl/lib
	LIBS +=  -lopengl32 -lglut -lglu32
else
	LIBS += -lglut -framework OpenGL -lglfw
endif

#############################
## SETUP GLEW 
#############################

ifeq ($(BUILDING_IN_LAB),1)
	INCPATH += -I./include
	LIBPATH += -L./lib
	LIBS += -lglew32
else
	LIBS += -lglew
endif

#############################
## COMPILATION INSTRUCTIONS 
#############################

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

.c.o: 	
	$(CXX) $(CFLAGS) $(INCPATH) -c -o $@ $<

.cc.o: 	
	$(CXX) $(CFLAGS) $(INCPATH) -c -o $@ $<

.cpp.o: 	
	$(CXX) $(CFLAGS) $(INCPATH) -c -o $@ $<

$(TARGET): $(OBJECTS) 
	$(CXX) $(CFLAGS) $(INCPATH) -o $@ $^ $(LIBPATH) $(LIBS)
