/*
 * Distributor.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: jmonk
 */

#include "Distributor.h"

Distributor::Distributor(Mailbox** receivers, JacobiCPU* owner, Mailbox* collector) {
	this->receivers = receivers;
	this->owner = owner;
	this->collector = collector;
	mailbox.setLocking(false);
}

Distributor::~Distributor() {

}

void Distributor::setControl(Mailbox** receivers, JacobiCPU* owner, Mailbox* collector) {
	this->receivers = receivers;
	this->owner = owner;
	this->collector = collector;
	mailbox.setLocking(false);
}

void Distributor::run() {
	while (owner->isRunning()) {
		while (mailbox.hasMessage()) {
			Message message = mailbox.getMessage();
			switch (message.getType()) {
			case MatrixVals:
				for (int i = 0; receivers[i] != NULL; i++) {
					if (!receivers[i]->isFull()) {
						receivers[i]->addMessage(message);
					}
				}
				break;
			case ProcStatus:
				if (!collector->isFull()) {
					collector->addMessage(message);
				}
				break;
			default:
				break;
			}
		}
	}
}



