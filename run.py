import sys
import code
import os
from subprocess import Popen


def Score(filename):
	delay = float(os.popen("cat %s | grep signal | cut -d: -f2 | awk '{print $1}'" % filename).read().strip())
	throughput = float(os.popen("cat %s | grep throughput | cut -d: -f2 | awk '{print $1}'" % filename).read().strip())	
	return  throughput, delay, throughput/delay*1000.0


Window = 32

L0 = 68
L1 = 60
L2 = 80
TWB = 5000
Beta = 11


ret = []

for TWB in range(3000,8000,500):
	for Beta in range(2,30,4):
		filename = "log_TWB_B_"+str(TWB)+"_"+str(Beta)
		Popen("echo 'int P_WINDOW=%d;' > parameter.hh" % Window, shell=True).wait()
		Popen("echo 'double P_L0=%d;' >> parameter.hh" % L0, shell=True).wait()
		Popen("echo 'double P_L1=%d;' >> parameter.hh" % L1, shell=True).wait()
		Popen("echo 'double P_L2=%d;' >> parameter.hh" % L2, shell=True).wait()
		Popen("echo 'double P_BETA=%d;' >> parameter.hh" % Beta, shell=True).wait()
		Popen("echo 'double TimeWarpBase=%d;' >> parameter.hh" % TWB, shell=True).wait()
	
		Popen("make clean > tmp",shell=True).wait()
		Popen("make > tmp",shell=True).wait()
	
		#print(filename)	
		Popen("./run-contest-local test 2> %s" % filename, shell=True).wait()
		print(filename)
		r = Score("%s" % filename)
		print(r)
		cmd = "echo 'Score " + str(r[2]) + "' >> " + filename;
		Popen(cmd, shell=True).wait()
		ret.append(r)

code.interact(local=locals())
