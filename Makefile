TARGET=jacobi_solve
SRCS=JacobiSolve.cpp \
	Matrix/SparseMatrix.cpp \
	Communication/Thread.cpp \
	Processing/JacobiCPU.cpp \
	Processing/JacobiGPU.cpp \
	Processing/JacobiGPU.cu \
	Processing/Distributor.cpp
OBJDIR=objs
OBJS=$(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
BUILDDIR= objs/Matrix\
		  objs/Communication\
		  objs/Processing
CFLAGS=-O2 -g #-Wall
NFLAGS=$(CFLAGS)# -arch sm_20
LIBS=-lpthread
NVCC=nvcc

.PHONY:all clean
all:$(TARGET)

$(TARGET):$(OBJS)
	$(NVCC) -o $(TARGET) $(CFLAGS) $(OBJS) $(LIBS)

$(OBJDIR)/%.o : %.cpp
	$(NVCC) -c -o $@ $(NFLAGS) $<

$(OBJDIR)/%.o : %.c
	$(NVCC) -c -o $@ $(NFLAGS) $<

$(OBJDIR)/%.o : %.cu
	$(NVCC) -c -o $@ $(NFLAGS) $<

clean:
	rm -rf $(OBJDIR)/* core* $(TARGET)
	mkdir $(BUILDDIR)
