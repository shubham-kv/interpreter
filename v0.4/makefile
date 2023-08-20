# -*- MakeFile -*-

CC=gcc

SRC=src
OUT=zout
DBG=debug
HDR=headers

SUB_DIRS_L=frontend intermediate  intermediate/ast  intermediate/ast/decls  intermediate/ast/exprs  intermediate/ast/stmts  intermediate/symbol  backend  backend/runtime backend/value utils
SUB_DIRS_W=frontend intermediate  intermediate\ast  intermediate\ast\decls  intermediate\ast\exprs  intermediate\ast\stmts  intermediate\symbol  backend  backend\runtime backend\value utils

SRCS=$(wildcard $(SRC)/*.c  $(patsubst %, $(SRC)/%/*.c, $(SUB_DIRS_L)))
OUTS=$(patsubst $(SRC)/%.c, $(OUT)/%.o, $(SRCS))
DBGS=$(patsubst $(SRC)/%.c, $(DBG)/%.o, $(SRCS))

HDRS= -I.\$(HDR)\	\
	-I.\$(HDR)\frontend	\
	-I.\$(HDR)\intermediate			\
	-I.\$(HDR)\intermediate\ast -I.\$(HDR)\intermediate\ast\decls  -I.\$(HDR)\intermediate\ast\exprs  -I.\$(HDR)\intermediate\ast\stmts  \
	-I.\$(HDR)\intermediate\symbol	\
	-I.\$(HDR)\backend  -I.\$(HDR)\backend\runtime  -I.\$(HDR)\backend\value	\
	-I.\$(HDR)\utils  -I.\$(HDR)\tools

all: $(OUT)/main $(DBG)/main

# $(OUT)/main: $(OUTS)
# 	$(CC) -DNDEBUG -Wall -o $@  $^  $(HDRS)
# $(OUT)/%.o: $(SRC)/%.c
# 	$(CC) -DNDEBUG  -o $@  -c $^  $(HDRS)

$(OUT)/main: $(OUTS)
	$(CC) -DNDEBUG -Wall -o $@  $^  $(HDRS)

$(OUT)/%.o: $(SRC)/%.c
	$(CC) -DNDEBUG -o $@  -c $^  $(HDRS)


$(DBG)/main: $(DBGS)
	$(CC) -DDEBUG -g -Wall  -o $@  $^  $(HDRS)

$(DBG)/%.o: $(SRC)/%.c
	$(CC) -DDEBUG -g  -o $@  -c $^  $(HDRS)


.PHONY: sub_dirs

sub_dirs: obj_sub_dirs dbg_sub_dirs

obj_sub_dirs:
	mkdir $(OUT) $(patsubst %,$(OUT)\\%,$(SUB_DIRS_W))

dbg_sub_dirs:
	mkdir $(DBG) $(patsubst %,$(DBG)\\%,$(SUB_DIRS_W))
