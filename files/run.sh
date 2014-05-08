#!/bin/sh
echo '>--------Simulation Beginning-------'
cores=%1
state=0
for folder in *; do
    if [ ! -z "${folder##*[!0-9]*}" ]; then
    for i in $(seq $(($(cat $folder/times)-1)) -1 0); do
        begin=$(cat mark)
        begin=$(($begin+1))
        sds=$(sed -n "${begin}p" seeds)
        echo $begin > mark
        cd $folder && mkdir $i && cd $i || { echo "Failed to access to $folder/$i"; exit 0; }
        echo '>building...'
        echo '>Current seed: '$sds
        echo ">Current  dir: $(pwd)"
        %2
        ./out.exe &

        state=$(($state+1))
        if [ $i -eq 0 -o  $state -eq $cores ]; then
            state=0
            echo '------------running---------------'
            wait
        fi

        cd ../../
        echo $i > $folder/times
    done
    fi
done

echo '>----The process completes successfully!----'

