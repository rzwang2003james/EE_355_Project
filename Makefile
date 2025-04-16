CC = g++
CFLAGS = -Wall -g

.PHONY: all clean

all: test_person test_network

test_person: test_person_eq.cpp person.cpp contact.cpp date.cpp
	$(CC) $(CFLAGS) -o test_person test_person_eq.cpp person.cpp contact.cpp date.cpp

test_network: test_network.cpp network.cpp person.cpp contact.cpp date.cpp misc.cpp
	$(CC) $(CFLAGS) -o test_network test_network.cpp network.cpp person.cpp contact.cpp date.cpp misc.cpp

clean:
	rm -f test_person test_network *.o 