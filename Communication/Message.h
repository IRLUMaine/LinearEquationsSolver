#pragma once
#include <pthread.h>

enum MessageType {
	NoMessage=0,
	StringMessage,
	MatrixVals,
	ProcStatus
};
//#define DEEP

/**
 * Data that is given to a message will be freed/deleted by the message
 * be careful of this.
 */
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
#ifdef DEEP
		this->p = new char[bytes];
		for (int i = 0; i < bytes; i++) {
			((char*)this->p)[i] = ((char*)message.p)[i];
		}
#else
		this->p = message.p;
#endif
	}

	~Message() {
#ifdef DEEP
		if (this->p) {
			delete[] (int*)this->p;
		}
#endif
	}

	/**
	 * Get the type of the message
	 */
	MessageType getType() {
		return type;
	}

	/**
	 * Get the data
	 */
	const void *getData() {
		return p;
	}

	/**
	 * Get number of bytes contained in message
	 */
	int getSize() {
		return bytes;
	}

	/**
	 * Set the type of message
	 */
	void setType(MessageType type) {
		this->type = type;
	}

	/**
	 * Set the data in the message.
	 * Will be freed by Message.
	 */
	void setData(void *p, int bytes) {
#ifdef DEEP
		if (this->p) {
			delete[] (int*)this->p;
		}
#endif
		this->p = p;
		this->bytes = bytes;
	}

	/**
	 * Performs deep copy of data.
	 */
	Message &operator=(Message& message) {
#ifdef DEEP
		if (this->p) {
			delete[] (int*)this->p;
		}
#endif //DEEP
		this->type = message.type;
		this->bytes = message.bytes;
#ifdef DEEP
		this->p = new char[bytes];
		for (int i = 0; i < bytes; i++) {
			((char*)this->p)[i] = ((char*)message.p)[i];
		}
#else
		this->p = message.p;
#endif

		return message;
	}
private:
	MessageType type;
	void *p;
	int bytes;
	bool local;
};
