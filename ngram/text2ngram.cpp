#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_map>

using namespace std;

// How to use: ./text2ngram.o <input file name> <output file name>
//
// g++ -std=c++11 text2ngram.cpp -o text2ngram.o
// ./text2ngram.o sample.txt sample
//
// => this will generate five files: sample_uni.out, sample_bi.out, sample_tri.out, sample_bbi.out, sample_btri.out

// This program parses throught the text file, counts unigram, bigram (forward/backward) and trigram (forward/backward).
// And then wrtie those into the output files.

// 1) Deal with <m> tag:
// <m> is added between two sentences. So, it could be seen as <s> or <e> when counting occurance.

int main(int argc, char **argv) {
	
	ifstream infile(argv[1]);
	//int ngram = atoi(argv[3]);

	string line;
	int countParagraph = 0;
	
	unordered_map<string, double> unigram;
	unordered_map<string, unordered_map<string, double>> bigram, trigram, backwardBigram, backwardTrigram;
	int countUnigram = 0;
	//unordered_map<string, int> countBigram, countTrigram, countBackwardBigram, countBackwardTrigram;

	//cout << ngram << endl;

	//string pre = "", prepre = "", preprepre = "";

	while (getline(infile, line)) {
		
		istringstream iss(line);
		string word;
		string pre = "", prepre = ""; //next paragraph is not related with the previous one

		while (iss >> word) {
			if (word == "<m>") unigram["<e>"]++;
			else unigram[word]++;

			if (pre != "") {
				if (word == "<m>") {
					bigram[pre]["<e>"]++;
					backwardBigram["<e>"][pre]++;
				} else if (pre == "<m>") {
					bigram["<s>"][word]++;
					bigram["<e>"][word]++;
					backwardBigram[word]["<s>"]++;
					backwardBigram[word]["<e>"]++;
				} else {
					bigram[pre][word]++;
					backwardBigram[word][pre]++;
				}
			}
			
			if (prepre != "" && pre != "") {
				if (word == "<m>") {
					trigram[prepre + " " + pre]["<e>"]++;
					backwardTrigram["<e> " + pre][prepre]++;
				} else if (pre == "<m>") {
					trigram[prepre + " <e>"][word]++;
					backwardTrigram[word + " <e>"][prepre]++;
				} else if (prepre == "<m>") {
					trigram["<s> " + pre][word]++;
					trigram["<e> " + pre][word]++;
					backwardTrigram[word + " " + pre]["<s>"]++;
					backwardTrigram[word + " " + pre]["<e>"]++;
				} else {
					trigram[prepre + " " + pre][word]++;
					backwardTrigram[word + " " + pre][prepre]++;
				}
			}

			prepre = pre;
			pre = word;
		}
	}

	string filename(argv[2]);
	ofstream outfileUnigram(filename + "_uni.out", ofstream::out);
	ofstream outfileBigram(filename + "_bi.out", ofstream::out);
	ofstream outfileBackwardBigram(filename + "_bbi.out", ofstream::out);
	ofstream outfileTrigram(filename + "_tri.out", ofstream::out);
	ofstream outfileBackwardTrigram(filename + "_btri.out", ofstream::out);

	for (auto it : unigram) countUnigram += it.second;
	for (auto it : unigram) {
		it.second /= countUnigram;
		outfileUnigram << setw(20) << it.first << "    " << std::setprecision(10) << to_string(it.second) << endl;
	}

	for (auto it : bigram) {
		auto map = it.second;
		int count = 0;
		for (auto it2 : map) count += it2.second;
		//countBigram[it.first] = count;
		for (auto it2 : map) {
			it2.second /= count;
			outfileBigram << setw(20) << it.first << " " << setw(20) << it2.first << "    "
				<< std::setprecision(10) << to_string(it2.second) << endl;
		}
	}

	for (auto it : backwardBigram) {
		auto map = it.second;
		int count = 0;
		for (auto it2: map) count += it2.second;
		//countBackwardBigram[it.first] = count;
		for (auto it2: map) {
			it2.second /= count;
			outfileBackwardBigram << setw(20) << it.first << " " << setw(20) << it2.first << "    "
				<< std::setprecision(10) << to_string(it2.second) << endl;
		}
	}

	for (auto it : trigram) {
		auto map = it.second;
		int count = 0;
		for (auto it2 : map) count += it2.second;
		//countTrigram[it.first] = count;
		for (auto it2 : map) {
			it2.second /= count;
			outfileTrigram << setw(20) << it.first << " " << setw(20) << it2.first << "    "
				<< std::setprecision(10) << to_string(it2.second) << endl;
		}
	}

	for (auto it : backwardTrigram) {
		auto map = it.second;
		int count = 0;
		for (auto it2 : map) count += it2.second;
		//countBackwardTrigram[it.first] = count;
		for (auto it2 : map) {
			it2.second /= count;
			outfileBackwardTrigram << setw(20) << it.first << " " << setw(20) << it2.first << "    "
				<< std::setprecision(10) << to_string(it2.second) << endl;
		}
	}

	return 0;
}