#
CC       := gcc 
CXX      := g++ 
LD       := g++ 
AR       := ar rc
RANLIB   := ranlib
MAKE	 :=
LINK	 := libtool --mode=link
COMPILE  := libtool --mode=compile
RM		 := rm -rf

DEBUG_CFLAGS       := -Wall -Wno-format -g -DDEBUG
DEBUG_CXXFLAGS     := -std=c++11 -Wall -ggdb -D_XPOKER_TEST_ -D_ENDABLE_TIME_TRACER_ -DTIXML_USE_STL
DEBUG_LDFLAGS      := -g -Wl,-rpath -Wl,LIBDIR
RELEASE_CFLAGS     := -Wall -Wno-unknown-pragmas -Wno-format -O3 
RELEASE_LDFLAGS    := -Wl,-rpath -Wl,LIBDIR

ifeq (YES, ${RELEASE})
CFLAGS         := ${RELEASE_CFLAGS}
CXXFLAGS       := ${RELEASE_CXXFLAGS}
LDFLAGS        := ${RELEASE_LDFLAGS}
else
CFLAGS         := ${DEBUG_CFLAGS}
CXXFLAGS       := ${DEBUG_CXXFLAGS}
LDFLAGS        := ${DEBUG_LDFLAGS}
endif

ifeq (YES, ${PROFILE})
CFLAGS    := ${CFLAGS} -pg -O3
CXXFLAGS  := ${CXXFLAGS} -pg -O3
LDFLAGS   := ${LDFLAGS} -pg
endif

BIN_DIR		:= bin
BIN         := ${BIN_DIR}/pdk_poker
BUILD_DIR	:= obj
INCPATH  	:= -I/usr/include/mysql -I/home/centos7/code.git/include -I/usr/local/assistx2/include
INCPATH		:= ${INCPATH} -I./poker -I./src -I./db -I./algorithm -I./frame -I./game_data -I./scenes -I./scenes/rooms -I./timer -I./3rdpart -I./datacenter

LINKPATH  	:= -L/usr/lib64/ -L/usr/lib64/mysql -L/usr/local/lib/boost -L/usr/local/libmemcached/lib -L/usr/local/assistx2/lib -L/home/centos7/code.git/lib 

LIBS   		:= -Wl,-dn -lassistx2_core -lassistx2_db -lassistx2_json -lassistx2_cache -Wl,-dy
LIBS 		:= ${LIBS} -lgflags -lglog -lprotobuf -lcpp_redis -ltinyxml -lboost_thread -lboost_system -lboost_date_time -lsnappy 
LIBS 		:= ${LIBS} -lmemcached -lmysqlclient -lboost_regex -ljson_spirit -lcurl -lgtest -pthread

#排除的目录
exclude_dirs := .git obj

#递归遍历3级子目录
DIRS := $(shell find . -maxdepth 3 -type d)
DIRS := $(filter-out ${exclude_dirs}, ${DIRS})
DIRS := $(basename $(patsubst ./%, %, ${DIRS}))
DIRS := ${DIRS} ${PWD}
SRCPATH := $(basename $(patsubst :%, %, ${DIRS}))

CPP_SRCS := $(foreach dir,${DIRS}, $(wildcard ${dir}/*.cpp))
CC_SRCS := $(foreach dir,${DIRS}, $(wildcard ${dir}/*.cc))
C_SRCS := $(foreach dir,${DIRS}, $(wildcard ${dir}/*.c))

OBJS := $(patsubst %.cpp, ${BUILD_DIR}/%.o, $(notdir ${CPP_SRCS}))
OBJS := ${OBJS} $(patsubst %.cc, ${BUILD_DIR}/%.o, $(notdir ${CC_SRCS}))
OBJS := ${OBJS} $(patsubst %.c, ${BUILD_DIR}/%.o, $(notdir ${C_SRCS}))

VPATH := ${SRCPATH}
#VPATH := poker:src:db:plugin:runfast:match:datacenter:3rdpart/base64/
#vpath %.h include
#vpath %.cpp src

.PHONY: all
all: _PRE $(BIN) 
	@echo "Start compiling"

${BIN}: ${OBJS}
	${CXX} -o $@ $^ ${LINKPATH} ${LIBS}

${BUILD_DIR}/%.o:%.cpp
	${CXX} -c -o $@ $< ${CXXFLAGS} ${INCPATH} 

${BUILD_DIR}/%.o:%.cc
	${CXX} -c -o $@ $< ${CXXFLAGS} ${INCPATH} 

${BUILD_DIR}/%.o:%.c
	${CC} -c -o $@ $< ${CXXFLAGS} ${INCPATH} 

.PHONY: _PRE
_PRE:
	@mkdir -p ${BUILD_DIR} ${BIN_DIR}
	
.PHONY: clean
clean:
	rm -rf ${OBJS} ${BUILD_DIR} ${BIN_DIR} ${BIN} 
