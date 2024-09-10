# Usage:
# make        # compile all binary
# make clean  # remove ALL binaries and objects

CC=gcc

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c $< -o $@


controller: $(OBJ)
	$(CC) $@.c -o $@.o $^

serial: $(OBJ)
	$(CC) $@.c -o $@.o $^

# perform cleanup
.PHONY: clean
clean:
	rm -f *.o *~ core
