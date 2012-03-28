#pragma once
#include <pthread.h>
#include <stdio.h>
#include "Message.h"

#ifndef MAILBOX_SIZE
#define MAILBOX_SIZE 16
#endif
#define DEBUG

class Mailbox {
public:
	Mailbox() {
		pthread_mutex_init(&mutex, NULL);
		front = MAILBOX_SIZE - 1;
		back = 0;
		notLocking = false;
		debug = false;
	}

	~Mailbox() {
		pthread_mutex_destroy(&mutex);
	}

	Message getMessage() {
		if (hasMessage()) {
			front++;
			if (front == MAILBOX_SIZE) front = 0;
#ifdef DEBUG
			if (debug) {
				printf("Mailbox %d %d\n", front, back);
			}
#endif

			return messages[front];
		} else {
			printf("There may be a problem...\n");
			return *(new Message(NoMessage, NULL, 0));
		}
	}

	bool hasMessage() {
		return (((front + 1) % MAILBOX_SIZE) != back);
	}

	bool isFull() {
		if (notLocking || (pthread_mutex_lock(&mutex) == 0)) {
			bool full = (back == front);
			if (!notLocking) {
				pthread_mutex_unlock(&mutex);
			}
			return full;
		} else {
			return true;
		}
	}

	bool addMessage(Message& message) {
		if (notLocking || (pthread_mutex_lock(&mutex) == 0)) {
			if (back != front) {
				messages[back] = message;
				back++;
				if (back == MAILBOX_SIZE) back = 0;
			}
#ifdef DEBUG
			if (debug) {
				printf("Mailbox %d %d\n", front, back);
			}
#endif
			if (!notLocking) {
				pthread_mutex_unlock(&mutex);
			}
			return true;
		} else {
			return false;
		}
	}

	void setLocking(bool lock) {
		notLocking = !lock;
	}

#ifdef DEBUG
	bool debug;
#endif
private:
	bool notLocking;
	int front;
	int back;
	pthread_mutex_t mutex;
	Message messages[MAILBOX_SIZE];
};
