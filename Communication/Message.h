#pragma once

enum MessageType {
	NoMessage=0,
	StringMessage
};

// Responsibility of sender to allocate p
// responsibility of receiver to free p
class Message {
public:
	Message(MessageType type, const void *p, int bytes) {
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
		this->p = message.p;
		this->bytes = message.bytes;
	}

	~Message() {
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

	Message &operator=(Message& message) {
		this->type = message.type;
		this->p = message.p;
		this->bytes = message.bytes;
		
		return *this;
	}
private:
	MessageType type;
	const void *p;
	int bytes;
};
