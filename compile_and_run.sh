#!/bin/bash

g++ preprocess/preprocess.cpp -o preprocess/preprocess
g++ ngram/ngram.cpp -o ngram/ngram 
g++ ngram/text2ngram.cpp -o ngram/text2ngram

# preprocess the available text

cd preprocess
./preprocess.sh ../data/1.txt ../data/1.out
cd ../ngram
./text2ngram ../data/1.out ../data/1
cd ..

# compile scala classes
scalac markovDSL/MarkovDSL.scala ; scalac Test.scala && scala Test
