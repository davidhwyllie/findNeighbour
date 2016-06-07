#!/bin/bash

cd /home/compass
echo "get samples from isilon..."
find /mnt/microbio/ndm-hicf/ogre/pipeline_output/ -name "*v3.fasta.gz" | grep R000000$1 | rev | cut -d"/" -f1 | rev | cut -d"_" -f1 > samples$1.txt
echo "extracting new samples"
cat R000000$1/model_ides.txt | cut -d$'\t' -f2 > samples_p$1.txt
sort samples$1.txt > sorted_samples$1.txt
sort samples_p$1.txt > sorted_samples_p$1.txt
join -v 1 sorted_samples$1.txt sorted_samples_p$1.txt > samples_n$1.txt

echo "pushing samples to elephantwalk..."
cd /home/compass/elephantwalk
python push_new_samples.py $1 $2 $3
echo "process finished !!"
