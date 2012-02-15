#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "Mailbox.h"

class Thread {
public:
	Thread();

	~Thread();

	virtual void run();
	void threadStartup();
	void start();
	void stop();
	bool isAwake();

	Mailbox mailbox;
private:
	virtual void startup();
	virtual void stopping();
	static void *threadStart(void *p);

	pthread_t threadId;
	bool isRunning;

};
