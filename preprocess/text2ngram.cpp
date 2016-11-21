#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_map>

using namespace std;

// How to use: ./text2ngram.o <input file name> <output file name>
// g++ -std=c++11 text2ngram.cpp -o text2ngram.o
// ./text2ngram.o sample.txt sample

// This program parses throught the text file, counts unigram, bigram and trigram.
// And then wrtie those into the output files.


int main(int argc, char **argv) {
	
	ifstream infile(argv[1]);
	//int ngram = atoi(argv[3]);

	string line;
	int countParagraph = 0;
	
	unordered_map<string, double> unigram;
	unordered_map<string, unordered_map<string, double>> bigram, trigram;
	int countUnigram = 0;
	unordered_map<string, int> countBigram, countTrigram;

	//cout << ngram << endl;

	string pre = "", prepre = "";

	while (getline(infile, line)) {
		
		istringstream iss(line);
		string word;

		while (iss >> word) {
			unigram[word]++;
			if (pre != "") bigram[pre][word]++;
			if (prepre != "") trigram[prepre][word]++;
			prepre = (pre == "") ? "" : pre + " " + word;
			pre = word;
		}
	}

	string filename(argv[2]);
	ofstream outfileUnigram(filename + "_uni.out", ofstream::out);
	ofstream outfileBigram(filename + "_bi.out", ofstream::out);
	ofstream outfileTrigram(filename + "_tri.out", ofstream::out);

	for (auto it : unigram) countUnigram += it.second;
	for (auto it : unigram) {
		it.second /= countUnigram;
		outfileUnigram << setw(20) << it.first << "    " << std::setprecision(10) << to_string(it.second) << endl;
	}

	for (auto it : bigram) {
		auto map = it.second;
		int count = 0;
		for (auto it2 : map) count += it2.second;
		countBigram[it.first] = count;
		for (auto it2 : map) {
			it2.second /= count;
			outfileBigram << setw(20) << it.first << " " << setw(20) << it2.first << "    "
						  << std::setprecision(10) << to_string(it2.second) << endl;
		}
	}

	for (auto it : trigram) {
		auto map = it.second;
		int count = 0;
		for (auto it2 : map) count += it2.second;
		countTrigram[it.first] = count;
		for (auto it2 : map) {
			it2.second /= count;
			outfileTrigram << setw(20) << it.first << " " << setw(20) << it2.first << "    "
						  << std::setprecision(10) << to_string(it2.second) << endl;
		}
	}

	return 0;
}