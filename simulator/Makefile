CC = gcc
CXX = g++
CFLAGS = -g -O0 -Wall -I../lib/uzlib/src -fsanitize=address -fsanitize=undefined
CXXFLAGS = -g -O0 -std=c++17 -Wall -Iinclude -I../src -I../lib/Epub -I../lib/uzlib/src -fsanitize=address -fsanitize=undefined
LDFLAGS = -fsanitize=address -fsanitize=undefined
SRCS = main.cpp \
       ../src/util/SdRawReader.cpp \
       ../src/util/WikiDecompressor.cpp \
       ../src/util/WikiSearchEngine.cpp \
       ../src/util/HtmlToPlainText.cpp \
       ../src/activities/wiki/WikiReaderActivity.cpp \
       ../lib/Epub/Epub/htmlEntities.cpp

C_SRCS = ../lib/uzlib/src/tinflate.c

OBJS = $(SRCS:.cpp=.o) $(C_SRCS:.c=.o)

simulator: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) simulator
