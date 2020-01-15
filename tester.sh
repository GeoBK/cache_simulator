golden_ref=$1
configurations=(""
"32 1024 2 65536 8 1 1 gcc_trace.txt"
"32 4096 2 16384 4 1 1 gcc_trace.txt"
"16 2048 8 16384 16 1 1 gcc_trace.txt"
"32 8192 4 32768 1 8 2 perl_trace.txt"
"16 4096 4 65536 1 8 4 perl_trace.txt"
"32 4096 8 131072 2 1 1 perl_trace.txt"
"32 4096 4 32768 4 1 1 perl_trace.txt"
"32 4096 4 32768 1 16 4 perl_trace.txt")

for (( i=1; i<${#configurations[@]}; i++ ))
do
    ./sim_cache ${configurations[$i]} > validation_run_${i}.txt
    diff -iw validation_run_${i}.txt "${golden_ref}${i}" > diff${i}.txt
    if [ -s "./diff${i}.txt" ]
    then
        echo "Differences found in run # ${i}"
        echo "Configuration details ${configurations[$i]}" 
    fi   
done