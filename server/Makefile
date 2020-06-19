CUR_PATH = $(shell pwd)
COMMON_PATH = ${CUR_PATH}/../common

SRC = ${wildcard  ${CUR_PATH}/*.cpp}

OBJS = ${patsubst %.cpp, %.o, ${SRC}}
TARGET=server
CPP=g++
CPPFLAGS=-g -Wall -std=c++11
INC_DIR=-I${COMMON_PATH}
LIB_DIR=-L.

LIBS=-lpthread

RM=rm -f

${TARGET}: ${OBJS}
	${CPP} ${OBJS} -o $@  ${LIB_DIR} $(LIBS)

$(OBJS):%.o:%.cpp
	${CPP} ${CPPFLAGS} ${INC_DIR} -c $< -o $@

.PHONY:clean
clean:
	$(RM) $(OBJS) ${TARGET}