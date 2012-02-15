#include "Thread.h"

Thread::Thread() {
	isRunning = false;
}

Thread::~Thread() {
	if (isRunning) {
		pthread_join(threadId, NULL);
	}
}

void Thread::start() {
	pthread_create(&threadId, NULL, &Thread::threadStart, this);
}

void Thread::stop() {
	if (isRunning) {
		pthread_cancel(threadId);
		isRunning = false;
	}
}

void Thread::run() {
	printf("Thread startup test!\n");
	printf("Forgot to override Thread::run()?\n");

	for (int i = 0; i < 3; i++) {
		while (mailbox.hasMessage()) {
			Message message = mailbox.getMessage();

			if (message.getType() == StringMessage) {
				printf("Received: %s\n", message.getData());
			}
		}
		printf("Thread awake\n");
		sleep(5);
	}
	printf("Thread shutdown?!?!?!\n");
}

void Thread::threadStartup() {
	this->startup();
	this->run();
	this->stopping();
}

void Thread::startup() {
	isRunning = true;
}

void Thread::stopping() {
	isRunning = false;
}

bool Thread::isAwake() {
	return isRunning;
}

void *Thread::threadStart(void *p) {
	((Thread*)p)->threadStartup();
}
