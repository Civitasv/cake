CXXFLAGS = -Wall -Wextra -std=c++17 -pedantic -ggdb
CC = g++

all: cake

cake: cake.cc
	$(CC) $(CXXFLAGS) -o cake cake.cc
