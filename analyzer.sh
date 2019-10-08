#golden_ref=$1



l2_assoc=(""
# "1"
"2"
# "4"
# "8"
# "16"
)

#"${block_size} ${l1_size}  ${l2_size} ${l2_assoc} ${data_blocks} ${address_tags} ${trace_file}"


l1_cache_sizes=(""
"64"
"2048"
"4096"
"8192"
"16384"
"32768"
"65536"
"131072"
"262144"
)

l2_cache_sizes=(""
"65536"
)

assocs=(""
"1"
"2"
"4"
"8"
"16"
)

blocks=(""
# "8"
# "16"
# "32"
# "64"
"128"
# "256"
# "512"
# "1024"
# "2048"
)

files=(""
"gcc_trace.txt"
"perl_trace.txt"
"go_trace.txt"
"vortex_trace.txt"
)

data_blocks=(""
"1"
)

addr_tags=(""
"1"
)




rm analyze_out.txt

for (( file=1; file<${#files[@]}; file++ ))
do
    echo "------------------------------------------------------" >> analyze_out.txt    
    echo "L2 size  ${files[$file]}\n" >> analyze_out.txt    
    echo "------------------------------------------------------" >> analyze_out.txt   
    for (( area=1; area<${#l2_cache_sizes[@]}; area++ ))
    do
        for (( p=1; p<${#data_blocks[@]}; p++ ))
        do
            for (( n=1; n<${#addr_tags[@]}; n++ ))
            do
                for (( oasoc=1; oasoc<${#assocs[@]}; oasoc++ ))
                do
                    for (( los=1; los<${#l1_cache_sizes[@]}; los++ ))
                    
                    do
                        for (( sasoc=1; sasoc<${#l2_assoc[@]}; sasoc++ ))
                        do
                            for (( bl=1; bl<${#blocks[@]}; bl++ ))
                            do    
                                l2_size=${l2_cache_sizes[area]}
                                if [ $l2_size -gt 0 ]
                                then
                                    ./sim_cache ${blocks[bl]} ${l1_cache_sizes[los]} ${assocs[oasoc]} ${l2_size} ${l2_assoc[sasoc]} ${data_blocks[p]} ${addr_tags[n]} ${files[$file]} >> analyze_out.txt
                                fi
                            done
                        done
                    done
                    
                done
            done
        done
    done
done







# #Fully associative
# echo "gcc_trace.txt" >> analyze_out.txt
# for (( i=1; i<${#configurations[@]}; i++ ))
# do
#     associativity=${configurations[$i]}/${block_size}
#     echo associativity
#     ./sim_cache ${block_size} ${l1_size} ${l1_assoc} ${configurations[$i]} ${associativity} ${data_blocks} ${address_tags} "gcc_trace.txt" >> analyze_out.txt    
# done

# echo "perl_trace.txt" >> analyze_out.txt
# for (( i=1; i<${#configurations[@]}; i++ ))
# do
#     associativity=${configurations[$i]}/${block_size}
#     echo associativity
#     ./sim_cache ${block_size} ${l1_size} ${l1_assoc} ${configurations[$i]} ${associativity} ${data_blocks} ${address_tags} "perl_trace.txt" >> analyze_out.txt    
# done

# echo "go_trace.txt" >> analyze_out.txt
# for (( i=1; i<${#configurations[@]}; i++ ))
# do
#     associativity=${configurations[$i]}/${block_size}
#     ./sim_cache ${block_size} ${l1_size} ${l1_assoc} ${configurations[$i]} ${associativity} ${data_blocks} ${address_tags} "go_trace.txt" >> analyze_out.txt    
# done

# echo "vortex_trace.txt" >> analyze_out.txt
# for (( i=1; i<${#configurations[@]}; i++ ))
# do
#     associativity=${configurations[$i]}/${block_size}
#     ./sim_cache ${block_size} ${l1_size} ${l1_assoc} ${configurations[$i]} ${associativity} ${data_blocks} ${address_tags} "vortex_trace.txt" >> analyze_out.txt    
# done