EXECS=primes_out my_bcast_out
MPICC?=mpic++

all: $(EXECS)

primes_out: 5_9.cc
	$(MPICC) -o primes_out 5_9.cc

my_bcast_out: 6_10.cc
	$(MPICC) -o my_bcast_out 6_10.cc

clean:
	rm -f $(EXECS)