n=$1
filename=$2
result=0
total=0
for i in `seq $n`
do
    $result=`./time_reads 2 "$filename" | cut -d " " -f 1`
    total=`expr $total + $result`
done
echo `expr $total / $n`
