#golden_ref=$1



l2_assoc=1

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

areastoconsider=(""
"524288"
)

assocs=(""
"1"
)

blocks=(""
"8"
"16"
"32"
"64"
"128"
"256"
"512"
"1024"
"2048"
"16384"
)

files=(""
"gcc_trace.txt")

data_blocks=(""
"1"
)

addr_tags=(""
"1"
)




rm block_size_trends.txt

for (( file=1; file<${#files[@]}; file++ ))
do
    echo "------------------------------------------------------" >> block_size_trends.txt    
    echo "L2 size  ${files[$file]}\n" >> block_size_trends.txt    
    echo "------------------------------------------------------" >> block_size_trends.txt   
    for (( area=1; area<${#areastoconsider[@]}; area++ ))
    do
        for (( p=1; p<${#data_blocks[@]}; p++ ))
        do
            for (( n=1; n<${#addr_tags[@]}; n++ ))
            do
                for (( los=1; los<${#l1_cache_sizes[@]}; los++ ))
                do
                    for (( oasoc=1; oasoc<${#assocs[@]}; oasoc++ ))
                    do
                        for (( bl=1; bl<${#blocks[@]}; bl++ ))
                        do    
                            l2_size=$((${areastoconsider[area]}-${l1_cache_sizes[los]}))
                            ./sim_cache ${blocks[bl]} ${l1_cache_sizes[los]} ${assocs[oasoc]} ${l2_size} ${l2_assoc} ${data_blocks[p]} ${addr_tags[n]} ${files[$file]} >> block_size_trends.txt
                        done
                    done
                    
                done
            done
        done
    done
done







# #Fully associative
# echo "gcc_trace.txt" >> block_size_trends.txt
# for (( i=1; i<${#configurations[@]}; i++ ))
# do
#     associativity=${configurations[$i]}/${block_size}
#     echo associativity
#     ./sim_cache ${block_size} ${l1_size} ${l1_assoc} ${configurations[$i]} ${associativity} ${data_blocks} ${address_tags} "gcc_trace.txt" >> block_size_trends.txt    
# done

# echo "perl_trace.txt" >> block_size_trends.txt
# for (( i=1; i<${#configurations[@]}; i++ ))
# do
#     associativity=${configurations[$i]}/${block_size}
#     echo associativity
#     ./sim_cache ${block_size} ${l1_size} ${l1_assoc} ${configurations[$i]} ${associativity} ${data_blocks} ${address_tags} "perl_trace.txt" >> block_size_trends.txt    
# done

# echo "go_trace.txt" >> block_size_trends.txt
# for (( i=1; i<${#configurations[@]}; i++ ))
# do
#     associativity=${configurations[$i]}/${block_size}
#     ./sim_cache ${block_size} ${l1_size} ${l1_assoc} ${configurations[$i]} ${associativity} ${data_blocks} ${address_tags} "go_trace.txt" >> block_size_trends.txt    
# done

# echo "vortex_trace.txt" >> block_size_trends.txt
# for (( i=1; i<${#configurations[@]}; i++ ))
# do
#     associativity=${configurations[$i]}/${block_size}
#     ./sim_cache ${block_size} ${l1_size} ${l1_assoc} ${configurations[$i]} ${associativity} ${data_blocks} ${address_tags} "vortex_trace.txt" >> block_size_trends.txt    
# done