# makefile for Linux builds
TARGET ?= ssfi
SRC_DIRS ?= .

# CC := gcc
LDLIBS := -lstdc++fs -lpthread 

SRCS := $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.cpp' -or -name '*.c' -or -name '*.s')
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -maxdepth 1 -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

PCH_SRC := ./src/pch.cxx 
PCH_H := ./src/pch.h 
PCH_OUT = ./src/pch.h.gch

#  -std=c++17 -std=c++1z 
CPPFLAGS ?= $(INC_FLAGS) -std=c++1z -D USE_FILESYSTEMX -MMD -MP -Wall -Wextra -pedantic-errors

all: $(TARGET)
	@echo Done!

# link it. $(PCH_OUT) 
$(TARGET): $(OBJS)
	@echo Linking
	g++ $(LDFLAGS) $(OBJS) $(LOADLIBES) $(LDLIBS) -o $@ 

# $(OBJS): $(SRCS) $(PCH_OUT)
# 	g++ $(CPPFLAGS) -c -o $@ $<

# $(PCH_OUT): $(PCH_SRC) $(PCH_H) 
# 	g++ $(CPPFLAGS) -c -o $@ $<
 
.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
