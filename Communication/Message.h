#pragma once
#include <pthread.h>

enum MessageType {
	NoMessage=0,
	StringMessage,
	MatrixVals,
	ProcStatus
};

// Responsibility of sender to allocate p
// responsibility of receiver to free p
class Message {
public:
	Message(MessageType type, void *p, int bytes) {
		this->type = type;
		this->p = p;
		this->bytes = bytes;
	}

	Message() {
		this->type = NoMessage;
		this->p = NULL;
		this->bytes = 0;
	}

	Message(const Message& message) {
		this->type = message.type;
		this->bytes = message.bytes;
		this->p = new char[bytes];
		for (int i = 0; i < bytes; i++) {
			((char*)this->p)[i] = ((char*)message.p)[i];
		}
	}

	~Message() {
		if (this->p) {
			delete[] (int*)this->p;
		}
	}

	MessageType getType() {
		return type;
	}

	const void *getData() {
		return p;
	}

	int getSize() {
		return bytes;
	}

	void setType(MessageType type) {
		this->type = type;
	}

	void setData(void *p, int bytes) {
		if (this->p) {
			delete[] (int*)this->p;
		}
		this->p = p;
		this->bytes = bytes;
	}

	Message &operator=(Message& message) {
		if (this->p) {
			delete[] (int*)this->p;
		}
		this->type = message.type;
		this->bytes = message.bytes;
		this->p = new char[bytes];
		for (int i = 0; i < bytes; i++) {
			((char*)this->p)[i] = ((char*)message.p)[i];
		}

		return message;
	}
private:
	MessageType type;
	void *p;
	int bytes;
	bool local;
};
