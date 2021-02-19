#############################
# RedHate
#############################

UNAME		= $(shell uname)
DATE		= `date`
WHOAMI		= `whoami`
CURTIME		= $(date)
TARGET		=  scope
OBJ			=  main.o DspBuildingBlocks/src/Nco.o DspBuildingBlocks/src/PhaseAccumulator.o
LDLIBS		=  -lGL -lX11 -lSDL2main -lSDL2 -pthread
ARCH		=  `arch`
CC			=  gcc
CXX			=  g++
LD			=  gcc
MV			=  mv
CP			=  cp
ECHO		=  echo
RM 			=  rm
AR			=  ar
RANLIB   	=  ranlib
STRIP		=  strip
PNG2H		=  tools/png2texh
OBJ2H		=  tools/obj2hGL
INCLUDES	= -I/usr/include
LIBS		= -L/usr/lib
CFLAGS   	= -Wall -g -O2 $(INCLUDES) $(LIBS) -fPIC -no-pie
CXXFLAGS 	= -Wall -g -O2 $(INCLUDES) $(LIBS) -fPIC -no-pie
WARNINGS	= -w

## colors for fun
COLOR		= \033[1;33m
NOCOLOR		= \033[0m

.PHONY: all run clean

all: $(ASSETS) $(OBJ) $(TARGET) $(TARGET_LIB)

run:  $(ASSETS) $(OBJ) $(TARGET) $(TARGET_LIB)
	@./$(TARGET)-$(ARCH)

clean:
	@printf "$(COLOR)[CLEANING]$(NOCOLOR)\n"
	@rm $(OBJ) $(TARGET)-$(ARCH) $(TARGET_LIB) $(ASSETS)

install:
	@cp -Rp $(TARGET) /usr/bin/$(TARGET)

list:
	@echo $(INCLUDES) $(LIBS)

%.o: %.cpp
	@printf "$(COLOR)[CXX]$(NOCOLOR) $(notdir $(basename $<)).o\n"
	@$(CXX) $(WARNINGS) -c $< $(CXXFLAGS) -o $(basename $<).o

%.o: %.cxx
	@printf "$(COLOR)[CXX]$(NOCOLOR) $(notdir $(basename $<)).o\n"
	@$(CXX) $(WARNINGS) -c $< $(CFLAGS) -o $(basename $<).o

%.o: %.c
	@printf "$(COLOR)[CC]$(NOCOLOR) $(notdir $(basename $<)).o\n"
	@$(CC) $(WARNINGS) -c $< $(CFLAGS) -o $(basename $<).o

%.a:
	@printf "$(COLOR)[CC]$(NOCOLOR) $(basename $(TARGET_LIB)).a\n"
	@$(AR) -cru $(basename $(TARGET_LIB)).a $(OBJ)

$(TARGET): $(ASSETS) $(OBJ)
	@printf "$(COLOR)[CXX]$(NOCOLOR) $(TARGET)-$(ARCH)\n"
	@$(CXX) $(WARNINGS)  $(OBJ) -o $(TARGET)-$(ARCH) $(CXXFLAGS) $(LDLIBS) `sdl2-config --cflags --libs` -Wl,-E
	@chmod a+x $(TARGET)-$(ARCH)
	@$(STRIP) -s $(TARGET)-$(ARCH)
