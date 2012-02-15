#include <iostream>
#include "Matrix/FastookReader.h"
#include "Communication/Thread.h"
#include "Communication/Message.h"

using namespace std;

int main(int argc, char * argv[])
{
	FastookReader reader("matrix");
	Matrix *A, *x, *b;
	Matrix c(1, 1);
	const void *p = "Hello Thread";
	Message message(StringMessage, p, 13);
	Thread thread;
	thread.mailbox.addMessage(message);
	thread.start();

	cout << "Starting Read..." << endl;
	thread.mailbox.addMessage(message);

	reader.readFile();

	cout << "Read Complete!" << endl;
	thread.mailbox.addMessage(message);

	return 0;
}
