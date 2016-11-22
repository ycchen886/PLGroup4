#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <unordered_map>

using namespace std;

// How to use: ./ngram.o <unigram file> <bigram file> <trigram file> <sentence length>
// g++ -std=c++11 ngram.cpp -o ngram.o
// ./ngram.o sample_uni.out sample_bi.out sample_tri.out 10

// parameters:
// We can change the three parameters lamda1, lamda2 and lamda3 
// for different dependency on unigram/bigram/trigram models.

// TODO: this is just a naive way of n gram.
// 1) deal with given keyword
// 2) don't output <s> <e> tag when generating sentences

double lamda1 = 0.0, lamda2 = 0.5, lamda3 = 1 - lamda1 - lamda2;

unordered_map<string, double> unigram;
unordered_map<string, unordered_map<string, double>> bigram, trigram;

void generateSentence(int);

void readUnigram(char *filename) {
	ifstream infile(filename);
	string line;

	while (getline(infile, line)) {	
		istringstream iss(line);
		string word;
		double prob;

		while (iss >> word >> prob) {
			unigram[word] = prob;
		}
	}
}

void readBigram(char *filename) {
	ifstream infile(filename);
	string line;

	while (getline(infile, line)) {	
		istringstream iss(line);
		string word1, word2;
		double prob;

		while (iss >> word1 >> word2 >> prob) {
			bigram[word1][word2] = prob;
		}
	}
}

void readTrigram(char *filename) {
	ifstream infile(filename);
	string line;

	while (getline(infile, line)) {	
		istringstream iss(line);
		string word1, word2, word3;
		double prob;

		while (iss >> word1 >> word2 >> word3 >> prob) {
			trigram[word1 + " " + word2][word3] = prob;
		}
	}
}

string sampleWord(string prepre, string pre) {
	//unordered_map<string, double> interpolateProb;
	int n = unigram.size();
	vector<string> words(n, "");
	vector<double> probs(n, 0);
	vector<double> accumProbs(n, 0);

	int i = 0;
	double sum = 0, accumSum = 0, unigramSum = 0;
	for (auto it : unigram) {
		string word = it.first;
		words[i] = word;
		probs[i] = lamda1 * unigram[word] + lamda2 * bigram[pre][word] + lamda3 * trigram[prepre + " " + pre][word];
		sum += probs[i];
		unigramSum += unigram[word];
		i++;
	}

	if (sum == 0) { // no probs for the next word
		i = 0;
		for (auto it : unigram) {
			string word = it.first;
			accumProbs[i] = ((i == 0) ? 0 : accumProbs[i-1]) + unigram[word]/unigramSum;
			i++;
		}
	} else {
		for (int i = 0; i < n; i++) {
			accumSum += probs[i];
			accumProbs[i] = accumSum / sum;
		}
	}

	//generate random number [0,1]
    double num = rand()/(double)RAND_MAX;
    //cout << num << " " << endl;
    vector<double>::iterator it;

    if (num == 0) it = upper_bound(accumProbs.begin(), accumProbs.end(), num);
    else it = lower_bound(accumProbs.begin(), accumProbs.end(), num);
    
    return words[it-accumProbs.begin()];
}

void test_sampleWord() {
	//cout << sampleWord("", "<s>");
	//cout << sampleWord(".", "<e>"); // end of sentence
	//generateSentence(10);
	cout << endl;
}

void generateSentence(int length) {
	string pre = "<s>", prepre = "";
	for (int i = 0; i < length; i++) {
		string cur = sampleWord(prepre, pre);
		if (cur == "<e>") cur = sampleWord("", "<s>");

		cout << cur << " ";
		prepre = pre;
		pre = cur;
	}
	cout << endl;
}

int main(int argc, char **argv) {
    srand(time(0));
    rand(); // To prevent the psuedo random generator depend on increasing time.

	readUnigram(argv[1]);
	readBigram(argv[2]);
	readTrigram(argv[3]);

	int sentenceLength = atoi(argv[4]);
	generateSentence(sentenceLength);
	//test_sampleWord();
	
	return 0;
}