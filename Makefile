
$(shell if [ ! -d "objs" ]; then mkdir -p objs; fi)
DIR=.


#动态库名以lib为前缀，以.so为后缀
#BIN=$(DIR)/libuvutils.so
BIN=$(DIR)/liblog.dylib

#SHARE=--share $(LIB) -luv
SHARE= -shared -fpic $(LIB)


INC= -I./include
LIB= -L./lib
OBJS=objs
#动态链接库编译时需要-fPIC选项
CFLAG=-g -Wall -fPIC $(INC) -std=c++11 -D__POSIX__

#链接动态库需要--share选项
LFLAG=$(SHARE)

CC=g++ -o

AR=ar -cr

#编译动态链接库用gcc -o命令
CO=$(CC)
#-------------------------以下为通用不变区域-----------------------

SRC=$(wildcard $(DIR)/*.cpp)
OBJ=$(patsubst %.cpp, $(OBJS)/%.o, $(SRC))

all:$(BIN)
$(BIN):$(OBJ)
	$(CO) $@ $^ $(LFLAG)
	g++ -o test test.cpp -llog -std=c++11 -L.
	

$(OBJS)/%.o:$(DIR)/%.cpp
	$(CC) $@ -c $< $(CFLAG)

.PHONY:clean
clean:
	rm $(OBJ) $(BIN)
