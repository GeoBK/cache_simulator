#golden_ref=$1


block_size=16
l1_size=16384
l1_assoc=2
l2_size=65536
l2_assoc=8
data_blocks=1
address_tags=1

#"${block_size} ${l1_size}  ${l2_size} ${l2_assoc} ${data_blocks} ${address_tags} ${trace_file}"

configurations=(""
# # "1"
# # "2"
# # "4"
# # "8"
# # "16"
# # "32"
# # "64"
"128"
"256"
"512"
"1024"
"2048"
"4096"
"8192"
"16384"
"32768"
"65536"
"131072"
"262144"
"524288")
rm analyze_out.txt

for(( j=1; j<=16; j=j*2 ))
do
echo "---------------------------------------------------------------" >> analyze_out.txt
echo "Associativity = ${j}" >> analyze_out.txt
echo "---------------------------------------------------------------" >> analyze_out.txt
echo "gcc_trace.txt" >> analyze_out.txt
for (( i=1; i<${#configurations[@]}; i++ ))
do
    ./sim_cache ${block_size} ${l1_size} ${l2_assoc} ${configurations[$i]} ${j} ${data_blocks} ${address_tags} "gcc_trace.txt" >> analyze_out.txt    
done

echo "perl_trace.txt" >> analyze_out.txt
for (( i=1; i<${#configurations[@]}; i++ ))
do
    ./sim_cache ${block_size} ${l1_size} ${l2_assoc} ${configurations[$i]} ${j} ${data_blocks} ${address_tags} "perl_trace.txt" >> analyze_out.txt    
done

echo "go_trace.txt" >> analyze_out.txt
for (( i=1; i<${#configurations[@]}; i++ ))
do
    ./sim_cache ${block_size} ${l1_size} ${l2_assoc} ${configurations[$i]} ${j} ${data_blocks} ${address_tags} "go_trace.txt" >> analyze_out.txt    
done

echo "vortex_trace.txt" >> analyze_out.txt
for (( i=1; i<${#configurations[@]}; i++ ))
do
    ./sim_cache ${block_size} ${l1_size} ${l2_assoc} ${configurations[$i]} ${j} ${data_blocks} ${address_tags} "vortex_trace.txt" >> analyze_out.txt    
done
done