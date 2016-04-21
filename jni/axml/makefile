SOURCE=$(wildcard *.c *.cpp)
OBJS=$(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SOURCE)))
CC=gcc
XX=g++
CFLAGS=-Wall -O -g  
TARGET=manifestAmbiguity

all: $(TARGET)  

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o:%.cpp
	$(XX) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)


clean:
	rm -rf *.o manifestAmbiguity
