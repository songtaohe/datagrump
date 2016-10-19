import sys
import code
import os
from subprocess import Popen


def Score(filename):
	delay = float(os.popen("cat %s | grep signal | cut -d: -f2 | awk '{print $1}'" % filename).read().strip())
	throughput = float(os.popen("cat %s | grep throughput | cut -d: -f2 | awk '{print $1}'" % filename).read().strip())	
	return  throughput, delay, throughput/delay*1000.0


Window = 128

L0 = 68
L1 = 60
L2 = 80
TWB = 5000
Beta = 3


ret = []

for L0 in range(62,68,2):
	for Beta in range(2,5):
		for L1 in range(54,60,2):
			for L2 in range(72,78,2):
				filename = "log_L0_L1_L2_B"+str(L0)+"_"+str(L1) + "_" + str(L2) + "_"+ str(Beta)
				Popen("echo 'int P_WINDOW=%d;' > parameter.hh" % Window, shell=True).wait()
				Popen("echo 'double P_L0=%d;' >> parameter.hh" % L0, shell=True).wait()
				Popen("echo 'double P_L1=%d;' >> parameter.hh" % L1, shell=True).wait()
				Popen("echo 'double P_L2=%d;' >> parameter.hh" % L2, shell=True).wait()
				Popen("echo 'double P_BETA=%d;' >> parameter.hh" % Beta, shell=True).wait()
				Popen("echo 'double TimeWarpBase=%d;' >> parameter.hh" % TWB, shell=True).wait()
	
				Popen("make clean > tmp",shell=True).wait()
				Popen("make > tmp",shell=True).wait()
	
		#print(filename)	
				Popen("./run-contest Songtao 2> %s" % filename, shell=True).wait()
				print(filename)
				r = Score("%s" % filename)
				print(r)
				cmd = "echo ' result " + str(r[0]) + " " + str(r[1]) + " " + str(r[2]) + "' >> " + filename + "_Score";
				print(cmd)
				Popen("%s" % cmd, shell=True).wait()
				ret.append(r)
		#exit()
code.interact(local=locals())
