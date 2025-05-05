CC = g++
CFLAGS = -Wall

.PHONY: all clean

all: test_person test_network test_phase2

test_person: test_person_eq.cpp person.cpp contact.cpp date.cpp
	$(CC) $(CFLAGS) -o test_person.o test_person_eq.cpp person.cpp contact.cpp date.cpp

test_network: test_network.cpp network.cpp person.cpp contact.cpp date.cpp misc.cpp
	$(CC) $(CFLAGS) -o test_network.o test_network.cpp network.cpp person.cpp contact.cpp date.cpp misc.cpp

test_phase2: test_phase2.cpp network.cpp person.cpp contact.cpp date.cpp misc.cpp
	$(CC) $(CFLAGS) -o test_phase2.o test_phase2.cpp network.cpp person.cpp contact.cpp date.cpp misc.cpp

clean:
	rm -f test_person.o test_network.o test_phase2.o *.o
	rm -rf *.dSYM 