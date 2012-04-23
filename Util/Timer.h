/**
 * Timer.h
 *
 *   Created on: Apr 23, 2012
 * 		Author: jmonk
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <sys/time.h>

class Timer {
public:
	Timer(const char* name) {
		this->name = name;
		runningTime = 0;
		count = 0;
	}

	~Timer() {
		double ave = runningTime / count;
		printf("%s Timer Report: \n\tCalled %g Times\n\tTook %gs on Ave\n\tTook %gs Total\n", name, (double)count, ave, runningTime);
	}

	void start() {
    	//gettimeofday(&t, NULL);
		clock_gettime(CLOCK_REALTIME, &t);
    	time1 = t.tv_sec + (t.tv_nsec / 1000000000.0);
	}

	void stop() {
    	//gettimeofday(&t, NULL);
		clock_gettime(CLOCK_REALTIME, &t);
    	time2 = t.tv_sec + (t.tv_nsec / 1000000000.0);
		count++;
		//printf("Call took: %g\n", time2 - time1);
		runningTime += time2 - time1;
	}

private:
	long long count;
	double time1;
	double time2;
	double runningTime;
	struct timespec t;
	const char* name;
	
};

#endif //__TIMER_H__
