TARGET=jacobi_solve
SRCS=JacobiSolve.cpp \
	Matrix/SparseMatrix.cpp \
	Communication/Thread.cpp \
	Processing/JacobiCPU.cpp \
	Processing/JacobiGPU.cpp \
	Processing/JacobiGPU.cu \
	Processing/ResidualCalculator.cpp \
	Processing/Distributor.cpp
OBJDIR=.objs
OBJS1=$(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
OBJS=$(patsubst %.cu,$(OBJDIR)/%CU.o,$(OBJS1))
BUILDDIR= .objs/Matrix\
		  .objs/Communication\
		  .objs/Processing
CFLAGS=-g# --ptxas-options=-v #-Wall
NFLAGS=$(CFLAGS) -arch sm_20
LIBS=-lpthread -lrt
NVCC=nvcc

.PHONY:all clean ODIR
all:$(TARGET)

ODIR:
	mkdir -p $(OBJDIR) 
	mkdir -p $(BUILDDIR)

$(TARGET):$(OBJS)
	$(NVCC) -o $(TARGET) $(CFLAGS) $(OBJS) $(LIBS)

$(OBJDIR)/%.o : %.cpp ODIR
	$(NVCC) -c -o $@ $(NFLAGS) $<

$(OBJDIR)/%.o : %.c ODIR
	$(NVCC) -c -o $@ $(NFLAGS) $<

$(OBJDIR)/%CU.o : %.cu ODIR
	$(NVCC) -c -o $@ $(NFLAGS) $<

clean:
	rm -rf $(OBJDIR)/* core* $(TARGET)
	mkdir -p $(OBJDIR) 
	mkdir -p $(BUILDDIR)
