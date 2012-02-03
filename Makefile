TARGET=jacobi_solve
OBJS=jacobi_solve.o
CFLAGS=-O2 -Wall
NFLAGS=$(CFLAGS) -arch sm_20
NVCC=nvcc
LIBS=

.PHONY:all clean
all:$(TARGET)

$(TARGET):$(OBJS)
	$(CXX) -o $(TARGET) $(CFLAGS) $(OBJS) $(LIBS)

%.o : %.cu
	$(NVCC) -c -o $@ $(NFLAGS)

clean:
	rm -f *.o core* $(TARGET)
