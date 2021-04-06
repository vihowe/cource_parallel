# !/bin/sh
rm pri_log
rm pri_time_log
rm appro_pi_log appro_time_log

for proc_num in `seq 1 8`;
do
    mpirun -n ${proc_num} ./consecutive_odd_pri 1 1000000 >> pri_log

    mpirun -n ${proc_num} ./appro_pi >> appro_pi_log
    
done

cat pri_log | grep ms | awk '{print $3}' >> pri_time_log 

cat appro_pi_log | grep ms | awk '{print $3}' >> appro_time_log