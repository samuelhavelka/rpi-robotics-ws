# Usage:
# make        # compile all binary
# make clean  # remove ALL binaries and objects

CC=gcc
IDIR=include
ODIR=src
CFLAGS=-I$(IDIR)

# link libraries
LIBS=

# prepend header files with IDIR
_DEPS = controller.h serial.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# prepend object files with ODIR
_OBJ = controller.o serial.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS) $(LIBS)


main: $(OBJ)
	$(CC) $@.c -o $@.o $^ $(CFLAGS) $(LIBS)

# perform cleanup
.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core
	rm -f *.o *~ core