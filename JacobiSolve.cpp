#include <iostream>
#include "Matrix/FastookReader.h"
#include "Communication/Thread.h"
#include "Communication/Message.h"

using namespace std;

int main(int argc, char * argv[])
{
	FastookReader reader("matrix.file");
	Matrix A(3,3), x(3,1), b(3, 1);
	Matrix c(1, 1);
	double sum = 0;
	const void *p = "Hello Thread";
	Message message(StringMessage, p, 13);
	Thread thread;
	thread.mailbox.addMessage(message);
	thread.start();

	cout << "Starting Read..." << endl;
	thread.mailbox.addMessage(message);

	int count = 1;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			A.setVal(j, i, count++);
		}
		x.setVal(i, 0, i+1);
	}
	b.setVal(0, 0, 30);
	b.setVal(1, 0, 36);
	b.setVal(2, 0, 42);

/*
	reader.readFile();
	A = reader.getA();
	x = reader.getX();
	b = reader.getB();
*/
	c = ((A) * (x));
	c = (c - b);
	for (int i = 0; i < c.getHeight(); i++) {
		sum += (c.getVal(i, 0) * c.getVal(i, 0));
	}
	sum /= c.getHeight();
	printf("Average Diff = %lf\n", sum);
	printf("Height is %d\n", c.getHeight());

	cout << "Read Complete!" << endl;
	thread.mailbox.addMessage(message);

	return 0;
}
