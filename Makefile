# compiler info
CC=gcc

# project info
OUT=walkmap

# directories
INSTALL_DIR=./bin/
INCLUDE_DIR=./include/
LIB_DIRS=./lib/
SRC_DIR=./src/
OBJ_DIR=$(INSTALL_DIR)obj/

# installation prefix
ifeq ($(PREFIX),)
	PREFIX := /usr/local/
endif

# obj formatting
_OBJ=utils.o world.o walkmap.o main.o
OBJ=$(patsubst %,$(OBJ_DIR)%,$(_OBJ))

# lib directories string (-L./dir/ -L./otherdir/)
LIB=$(patsubst %,-L%,$(LIB_DIRS))

# lib includes
LIBS=

# compiler flags
CFLAGS=-Werror -g

# make all targets
.PHONY: all
all: $(INSTALL_DIR)$(OUT)

# install targets (add to /usr/local/bin)
.PHONY: install
install: all
	@echo installing $(OUT) to $(PREFIX)$(INSTALL_DIR)
	@install -d $(PREFIX)$(INSTALL_DIR)
	@install -m 777 $(INSTALL_DIR)$(OUT) $(PREFIX)$(INSTALL_DIR)
	@echo installed $(OUT)

# clean o files
.PHONY: clean
clean: clearobj all
	
.PHONY: clearobj
clearobj:
	@rm -rf $(OBJ_DIR)

# main target
$(INSTALL_DIR)$(OUT): $(OBJ) 
	@echo building $@
	@$(CXX) -o $@ $^ $(CFLAGS) -I$(INCLUDE_DIR) $(LIB) $(LIBS)
	@echo built $@
	
# define obj prerequisites
$(OBJ_DIR)utils.o: $(SRC_DIR)utils.cpp $(INCLUDE_DIR)utils.hpp
$(OBJ_DIR)world.o: $(SRC_DIR)world.cpp $(INCLUDE_DIR)world.hpp
$(OBJ_DIR)walkmap.o: $(SRC_DIR)walkmap.cpp $(INCLUDE_DIR)walkmap.hpp
$(OBJ_DIR)main.o: $(SRC_DIR)main.cpp

# obj rule
$(OBJ):
	@echo building $@
	@mkdir -p $(OBJ_DIR)
	@$(CXX) -c -o $@ $< $(CFLAGS) -I$(INCLUDE_DIR) $(LIB) $(LIBS)
	@echo built $@