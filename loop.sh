


for (( i=60; i <= 80; i=i+4 ))
do
for (( j=i+4; j <= 88; j=j+4))
do
for (( b=10; b <=18; b=b+4))
do
 echo " "
 echo "=========================="
 echo "Parameter: L1 $i L2 $j BETA $b"
 echo "Parameter: L1 $i L2 $j BETA $b" > log_t_${i}_${j}_${b}
 echo "int P_WINDOW=32;" > parameter.hh
 echo "double P_L1=$i;" >> parameter.hh
 echo "double P_L2=$j;" >> parameter.hh
 echo "double P_BETA=$b;" >> parameter.hh
 make clean
 make
 ./run-contest-local test 2>> log_t_${i}_${j}_${b}
 cat log_t_${i}_${j}_${b}
 echo " "
done
done
done
