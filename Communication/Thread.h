#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "Mailbox.h"

/**
 * This is a thread class designed to hide the pthreads interface
 * and make threading in C++ as similar to java as possible. With
 * a added feature of a working stop function.
 */
class Thread {
public:
	Thread();

	virtual ~Thread();

	/**
	 * This function gets called by the thread when starting up.
	 * This should be overloaded by child classes
	 */
	virtual void run();

	/**
	 * This starts the thread up. Will launch the pthread and run
	 * the run function.
	 */
	void start();

	/**
	 * This will cancel the pthread immediately.
	 */
	void stop();

	/**
	 * Tests if the thread is currently running.
	 */
	bool isAwake();

	/**
	 * This is a method of passing messages between threads using
	 * pthreads.
	 */
	Mailbox mailbox;
private:
	/**
	 * used by threadStartup;
	 */
	virtual void startup();
	/**
	 * Used by threadStartup;
	 */
	virtual void stopping();
	/**
	 * Used internally by threadStart
	 */
	void threadStartup();
	/**
	 * This is the function that gets called
	 * by pthreads and is passed a pointer to the thread.
	 */
	static void *threadStart(void *p);

	pthread_t threadId;
	bool isRunning;

};
