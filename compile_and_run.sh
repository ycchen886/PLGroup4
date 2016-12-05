#!/bin/bash

g++ -std=c++11 preprocess/preprocess.cpp -o preprocess/preprocess
g++ -std=c++11 ngram/ngram.cpp -o ngram/ngram 
g++ -std=c++11 ngram/text2ngram.cpp -o ngram/text2ngram

# preprocess the available text

cd preprocess
./preprocess.sh ../data/1.txt ../data/1.out
./preprocess.sh ../data/Trump/all_Trump.txt ../data/Trump/all_Trump.out

cd ../ngram
./text2ngram ../data/1.out ../data/1
./text2ngram ../data/Trump/all_Trump.out ../data/Trump/all_Trump

cd ..

# compile scala classes
scalac markovDSL/MarkovDSL.scala ; scalac Test.scala && scala Test
