#include "Thread.h"

Thread::Thread() {
	isRunning = false;
}

Thread::~Thread() {
	if (isRunning) {
		pthread_join(threadId, NULL);
	}
}

/**
 * This starts the thread up. Will launch the pthread and run
 * the run function.
 */
void Thread::start() {
	pthread_create(&threadId, NULL, &Thread::threadStart, this);
}

/**
 * This will cancel the pthread immediately.
 */
void Thread::stop() {
	if (isRunning) {
		pthread_cancel(threadId);
		pthread_join(threadId, NULL);
		isRunning = false;
	}
}

/**
 * This function gets called by the thread when starting up.
 * This should be overloaded by child classes
 */
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

/**
 * Used internally by threadStart
 */
void Thread::threadStartup() {
	this->startup();
	this->run();
	this->stopping();
}

/**
 * used by threadStartup;
 */
void Thread::startup() {
	isRunning = true;
}

/**
 * Used by threadStartup;
 */
void Thread::stopping() {
	isRunning = false;
}

/**
 * Tests if the thread is currently running.
 */
bool Thread::isAwake() {
	return isRunning;
}

/**
 * This is the function that gets called
 * by pthreads and is passed a pointer to the thread.
 */
void *Thread::threadStart(void *p) {
	((Thread*)p)->threadStartup();
	return NULL;
}
