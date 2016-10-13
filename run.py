import sys
import code
import os
from subprocess import Popen


def Score(filename):
	delay = float(os.popen("cat %s | grep signal | cut -d: -f2 | awk '{print $1}'" % filename).read().strip())
	throughput = float(os.popen("cat %s | grep throughput | cut -d: -f2 | awk '{print $1}'" % filename).read().strip())	
	return  throughput, delay, throughput/delay*1000.0


Window = 32

L0 = 70
L1 = 60
L2 = 80

Beta = 11


ret = []

for L0 in range(60,80,2):
	Popen("echo 'int P_WINDOW=%d;' > parameter.hh" % Window, shell=True).wait()
	Popen("echo 'double P_L0=%d;' >> parameter.hh" % L0, shell=True).wait()
	Popen("echo 'double P_L1=%d;' >> parameter.hh" % L1, shell=True).wait()
	Popen("echo 'double P_L2=%d;' >> parameter.hh" % L2, shell=True).wait()
	Popen("echo 'double P_BETA=%d;' >> parameter.hh" % Beta, shell=True).wait()
	
	Popen("make clean",shell=True).wait()
	Popen("make",shell=True).wait()
	
	print("L0 " + str(L0))	
	#os.popen("./run-contest-local test > log_L0_%d" % L0, shell=True).wait()
	
	#r = Score("log_L0_%d" % L0)
	#print(r)
 
	#ret.append(r)

code.interact(local=locals())
