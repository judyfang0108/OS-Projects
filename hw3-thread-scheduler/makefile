
GIT_HOOKS := .git/hooks/applied
CC := gcc
CFLAGS += -std=gnu99 -g -Wall
CLIB += -ljson-c
OBJ := simulator.o os2021_thread_api.o function_library.o

all: $(GIT_HOOKS) 

$(GIT_HOOKS):
	@.githooks/install-git-hooks
	@echo

simulator:$(OBJ)
	$(CC) $(CFLAGS) -o simulator $(OBJ) $(CLIB)

simulator.o: os2021_thread_api.h
os2021_thread_api.o: os2021_thread_api.h function_library.h 
function_libary.o: function_library.h os2021_thread_api.h 

.PHONY: clean
clean:
	-rm $(OBJ) simulator
