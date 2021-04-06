import matplotlib.pyplot as plt
import numpy as np

proc_num = np.arange(1, 9, 1)

y_pi = []
y_pri = []
with open("appro_time_log", "r") as f:
    for line in f:
        y_pi.append(float(line))

with open("pri_time_log", "r") as f:
    for line in f:
        y_pri.append(float(line))

y_pi = np.array(y_pi)
y_pi = 100 * y_pi
y_pri = np.array(y_pri)
y_pri = 100 * y_pri
plt.plot(proc_num, y_pi)
plt.savefig("./pi_benchmarking.png")
plt.close()

plt.plot(proc_num, y_pri)
plt.savefig("./pri_benchmarking.png")