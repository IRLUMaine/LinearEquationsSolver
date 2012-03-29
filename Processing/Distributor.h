/*
 * Distributor.h
 *
 *  Created on: Mar 26, 2012
 *      Author: jmonk
 */
#ifndef DISTRIBUTOR_H_
#define DISTRIBUTOR_H_
#include "../Communication/Thread.h"
#include "JacobiCPU.h"

class Distributor : public Thread {
public:
	Distributor(Mailbox** receivers, JacobiCPU* owner, Mailbox* collector);

	Distributor() { }
	virtual ~Distributor();

	/**
	 * For arrays
	 */
	void setControl(Mailbox** receivers, JacobiCPU* owner, Mailbox* collector);

	/**
	 * This waits for packets coming from the processor and distributes it to
	 * a list of receivers. If it is a ProcStatus packet it is sent to the
	 * collecting mailbox so the collector can know when all have converged.
	 */
	void run();

private:
	Mailbox** receivers;
	JacobiCPU* owner;
	Mailbox* collector;
};

#endif /* DISTRIBUTOR_H_ */
