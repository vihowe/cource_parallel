EXECS=consecutive_odd_pri appro_pi
MPICC?=mpic++

all: $(EXECS)

consecutive_odd_pri: consecutive_odd_prime.cc
	$(MPICC) -o consecutive_odd_pri consecutive_odd_prime.cc

appro_pi: appro_pi.cc
	$(MPICC) -o appro_pi appro_pi.cc

clean:
	rm -f $(EXECS)