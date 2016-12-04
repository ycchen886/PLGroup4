# remove class files
rm -f *.class markovDSL/*.class

# remove compiled C++ binaries
rm -f ngram/ngram preprocess/preprocess ngram/text2ngram

# remove preprocessed data
rm -f data/*.out

# remove other binaries
rm -f markovDSL/test 
