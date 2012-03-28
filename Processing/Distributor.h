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

	void run();

private:
	Mailbox** receivers;
	JacobiCPU* owner;
	Mailbox* collector;
};

#endif /* DISTRIBUTOR_H_ */
