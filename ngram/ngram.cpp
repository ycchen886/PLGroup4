#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <queue>
#include <cstdlib>
#include <climits>
#include <algorithm>
#include <unordered_map>

using namespace std;

// How to use: ./ngram.o <unigram file> <bigram file> <trigram file> <sentence length>
// g++ -std=c++11 ngram.cpp -o ngram.o
// ./ngram.o sample_uni.out sample_bi.out sample_tri.out 10

// parameters:
// 1) lamda1, lamda2 and lamda3: for different dependency on unigram/bigram/trigram models.
// 2) 

// TODO: this is just a naive way of n gram.
// 1) deal with given keyword
// 2) don't output <s> <e> tag when generating sentences

//double lamda1 = 0.0, lamda2 = 0.5, lamda3 = 1 - lamda1 - lamda2;

int shortWordLenUpperBnd = 3, longWordLenLowerBnd = 7;
int shortSentenceLenUpperBnd = 8, longSentenceLenLowerBnd = 25, maxSentencelen = 35;

unordered_map<string, double> unigram;
unordered_map<string, unordered_map<string, double>> bigram, trigram, backwardBigram, backwardTrigram;

void generateSentence(int);

struct WordPath{
	string totalPath;
	int len;
	double pathProb;
	string pre;
	string prepre;
	
	WordPath(string startWord) : totalPath(startWord), len(1), pathProb(1.0), pre(startWord), prepre("") {}
	
	void add(string word, double prob) {
		prepre = pre;
		pre = word;
		totalPath += " " + word;
		len++;
		pathProb *= prob;
	}

	void addBackward(string word, double prob) {
		prepre = pre;
		pre = word;
		totalPath = word + " " + totalPath;
		len++;
		pathProb *= prob;
	}
};

struct Sentence {
	string sentence;
	double prob;

	int getLenOfLastWord() {
		return 0;
	}


};

void clear(std::queue<WordPath> &q)
{
	std::queue<WordPath> empty;
	std::swap( q, empty );
}

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
			backwardBigram[word2][word1] = prob;
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
			backwardTrigram[word3 + " " + word2][word1] = prob;
		}
	}
}

// prepre: the string that is two words before
// pre: the string that is one word before
// direction: +1 means forward, -1 means backward
string sampleWord(string prepre, string pre, int direction, double lamda1, double lamda2, double lamda3) {
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
		if (direction == 1) probs[i] = lamda1 * unigram[word] + lamda2 * bigram[pre][word] + lamda3 * trigram[prepre + " " + pre][word];
		else probs[i] = lamda1 * unigram[word] + lamda2 * backwardBigram[pre][word] + lamda3 * backwardTrigram[prepre + " " + pre][word];
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
		string cur = sampleWord(prepre, pre, 1, 0.0, 0.5, 0.5);
		if (cur == "<e>") cur = sampleWord("", "<s>", 1, 0.0, 0.5, 0.5);

		cout << cur << " ";
		prepre = pre;
		pre = cur;
	}
	cout << endl;
}

// This function will generate random words.
// GENERATE (10 SHORT WORDS) THEN OUTPUT => generateWords(10, 1, shortWordLenUpperBnd, {""})
// GENERATE (5 WORDS) ON "cat" THEN OUTPUT => generateWords(5, 1, INT_MAX, {"cat"})
void generateWords(int number, int lenLowerBnd, int lenUpperBnd, vector<string> keywords) {
	if (keywords.size() > 1) cout << "You should not have more than one keyword for generating words.";

	if (keywords.size() == 1) {
		if (keywords[0].size() < lenLowerBnd) cout << "Keyword: " << keywords[0] << " is not a LONG word.";
		else if (keywords[0].size() > lenUpperBnd) cout << "Keyword: " << keywords[0] << " is not a SHORT word.";
		else for (int i = 0; i < number; i++) cout << keywords[0] << " ";
		cout << endl;
		return;
	}

	int i = 0;
	while (i < number) {
		string cur = sampleWord("", "", 1, 1, 0.0, 0.0);
		if (isalnum(cur[0]) && cur.size() >= lenLowerBnd && cur.size() <= lenUpperBnd) {
			cout << cur << " ";
			i++;
		}
	}
	cout << endl;
}

// GENERATE (10 SHORT SENTENCES) THEN OUTPUT
void generateSentences(int number, int lenLowerBnd, int lenUpperBnd, vector<string> keywords) {
	if (keywords.size() > 2) {
		cout << "You should not have more than two keyword." << endl;
		return;
	}

	if (keywords.size() == 1) {

	}

	for (int i = 0; i < number; i++) {
		string pre = "<s>", prepre = "", cur = "";
		string output = "";
		int len = 0;
		while (cur != "<e>") {
			cur = sampleWord(prepre, pre, 1, 0.0, 0.5, 0.5);
			if (cur != "<e>") {
				if (isalnum(cur[0])) len++;
				output += cur + " ";
			}
			prepre = pre;
			pre = cur;
		}
		if (len >= lenLowerBnd && len <= lenUpperBnd) {
			cout << output << endl;
			cout << "Length: " << len << endl;
		} else i--;
	}
}

string generatePartOfSentence(string start, string end) {
	unordered_map<string, vector<WordPath>> fromStart, fromEnd;
	queue<WordPath> qFromStart, qFromEnd;
	qFromStart.push(WordPath(start));
	qFromEnd.push(WordPath(end));
	int partOfSentenceLen = 8;

	while (!qFromStart.empty()) {
		WordPath wp = qFromStart.front();
		qFromStart.pop();

		if (wp.len > partOfSentenceLen) break;
		if (wp.pre == "<e>") continue;

		if (bigram[wp.pre].size() == 0) {
			//cout << "1:" << endl;
			for (auto it : unigram) {
				WordPath tmp = wp;
				string word = it.first;
				double prob = unigram[word];
				if (unigram[word] > 0){ // && word != "<e>") {
					tmp.add(word, prob);
					fromStart[word].push_back(tmp);
					qFromStart.push(tmp);
				}
			}
		} else {
			//cout << "2:" << endl;
			for (auto it : bigram[wp.pre]) {
				WordPath tmp = wp;
				string word = it.first;
				double prob = 0.5 * (it.second) + 0.5 * trigram[tmp.prepre + " " + tmp.pre][word];
				if (prob > 0) { // && word != "<e>") {
					tmp.add(word, prob);
					fromStart[word].push_back(tmp);
					qFromStart.push(tmp);
				}
			}
		}
		/*
		for (auto it : unigram) {
			WordPath tmp = wp;
			string word = it.first;
			double prob = 0.0 * unigram[word] + 0.5 * bigram[tmp.pre][word] + 0.5 * trigram[tmp.prepre + " " + tmp.pre][word];
			if (prob > 0 && word != "<e>") {
				tmp.add(word, prob);
				fromStart[word].push_back(tmp);
				qFromStart.push(tmp);
			}
		}
		*/
	}
	cout << "start cleaning queue;" << endl;
	clear(qFromStart);
	cout << "finish qFromStart." << endl;

	while (!qFromEnd.empty()) {
		WordPath wp = qFromEnd.front();
		qFromEnd.pop();

		if (wp.len > partOfSentenceLen) break;
		if (wp.pre == "<s>" || wp.pre == "<e>") continue;

		if (backwardBigram[wp.pre].size() == 0) {
			//cout << "1:" << endl;
			for (auto it : unigram) {
				WordPath tmp = wp;
				string word = it.first;
				double prob = unigram[word];
				if (unigram[word] > 0 && word != "<e>") {
					tmp.addBackward(word, prob);
					fromEnd[word].push_back(tmp);
					qFromEnd.push(tmp);
				}
			}
		} else {
			//cout << "2:" << endl;
			for (auto it : backwardBigram[wp.pre]) {
				WordPath tmp = wp;
				string word = it.first;
				double prob = 0.5 * (it.second) + 0.5 * backwardTrigram[tmp.prepre + " " + tmp.pre][word];
				if (prob > 0 ) { //&& word != "<e>") {
					tmp.addBackward(word, prob);
					fromEnd[word].push_back(tmp);
					qFromEnd.push(tmp);
				}
			}
		}
		/*
		for (auto it : unigram) {
			WordPath tmp = wp;
			string word = it.first;
			double prob = 0.0 * unigram[word] + 0.5 * backwardBigram[tmp.pre][word] + 0.5 * backwardTrigram[tmp.prepre + " " + tmp.pre][word];
			if (prob > 0 && word != "<e>") {
				//cout << prob << " ";
				tmp.addBackward(word, prob);
				fromEnd[word].push_back(tmp);
				qFromEnd.push(tmp);
			}
		}
		*/
	}
	cout << "start cleaning queue;" << endl;
	clear(qFromEnd);
	cout << "finish qFromEnd." << endl;

	vector<string> results;
	vector<double> probs;
	double sum = 0.0;
	int count = 0;

	int countFromStart = 0, countFromEnd = 0;
	for (auto it : fromStart) {
		countFromStart += it.second.size();
		vector<WordPath> vwp = it.second;
		for (int i = 0; i < it.second.size(); i++) cout << vwp[i].totalPath << endl;
	}
	cout << "------" << endl;
	for (auto it : fromEnd) {
		countFromEnd += it.second.size();
		vector<WordPath> vwp = it.second;
		for (int i = 0; i < it.second.size(); i++) cout << vwp[i].totalPath << endl;
	}
	cout << "fromStart.size()= " << fromStart.size() << ", fromEnd.size()= " << fromEnd.size() << endl;
	cout << "countFromStart= " << countFromStart << ", countFromEnd= " << countFromEnd << endl;
	//return "";

	
	for (auto it: fromStart) {
		string word = it.first;
		for (int i = 0; i < fromStart[word].size(); i++) {
			if (fromEnd.count(word) == 0) continue;
			if (fromStart[word][i].pathProb < sum/countFromStart) continue;

			//cout << fromEnd[word].size() << endl;
			for (int j = 0; j < fromEnd[word].size(); j++) {
				if (fromEnd[word][j].pathProb < sum/countFromEnd) continue;

				string sentence = fromStart[word][i].totalPath + "---" + fromEnd[word][j].totalPath;
				double prob = fromStart[word][i].pathProb * fromEnd[word][j].pathProb;
				if (prob < sum/countFromStart) continue;
				results.push_back(sentence);
				probs.push_back(prob);
				//cout << fromStart[word][i].totalPath + "---" + fromEnd[word][j].totalPath << endl;
				sum += fromStart[word][i].pathProb * fromEnd[word][j].pathProb;//probs[probs.size()-1];
				count++;
			}
		}
	}
	cout << sum << endl;
	cout << count << endl;
	
	vector<double> accumProbs(probs.size(),0);
	for (int i = 0; i < probs.size(); i++) accumProbs[i] = (i==0 ? 0.0 : accumProbs[i-1]) + probs[i]/sum;
	//for (int i = 0; i < 100; i++) cout << accumProbs[i] << endl;

	double num = rand()/(double)RAND_MAX;
	//cout << num << " " << endl;
	vector<double>::iterator it;

	if (num == 0) it = upper_bound(accumProbs.begin(), accumProbs.end(), num);
	else it = lower_bound(accumProbs.begin(), accumProbs.end(), num);

	return results[it-accumProbs.begin()];
	
	/*for (auto it : fromStart) {
		string word = it.first;
		cout << word << " : ";
		for (int i = 0; i < fromStart[word].size(); i++) cout << fromStart[word][i].len << " ";
		cout << endl;
		//for (int i = 0; i < fromStart[word].size(); i++) cout << fromStart[word][i].totalPath << " ----> prob: " << fromStart[word][i].pathProb << endl;
	}*/
	
	//end = "freezing-point";
	//for (int i = 0; i < fromStart[end].size(); i++) cout << fromStart[end][i].totalPath << " ----> prob: " << fromStart[end][i].pathProb << endl;
	//for (int i = 0; i < fromEnd[start].size(); i++) cout << fromEnd[start][i].totalPath << " ----> prob: " << fromEnd[start][i].pathProb << endl;

	//if (fromStart[end].size() > 0) return fromStart[end][0].totalPath;
	//else return "There's no path from " + start + " to " + end + ".";

	//if (fromEnd[start].size() > 0) return fromEnd[start][0].totalPath;
	//else return "There's no backward path from " + end + " to " + start + ".";

	
}

void test_generateWords() {
	vector<string> keywords;
	generateWords(10, 1, INT_MAX, keywords);			//output: and few The go wild while A THE here appearance 
	generateWords(10, 1, shortWordLenUpperBnd, keywords); 		//output: of her too at the the The he his of 
	generateWords(10, longWordLenLowerBnd, INT_MAX, keywords);	//output: Accident Perthensis encoding English reputation PROJECT Character Encyclopædia willing English
	cout << endl;

	keywords = {"cat"};
	generateWords(3, longWordLenLowerBnd, INT_MAX, keywords); 	//output: Keyword:cat is not a LONG word.
	generateWords(5, 1, shortWordLenUpperBnd, keywords);		//output: cat cat cat cat cat
	cout << endl;
}

void test_generateSentences() {
	vector<string> keywords;
	generateSentences(2, 1, INT_MAX, keywords);
	cout << endl;
	generateSentences(1, 1, shortSentenceLenUpperBnd, keywords);
}

void test_generatePartOfSentence() {
	cout << generatePartOfSentence("cat", "made") << endl;
}

int main(int argc, char **argv) {
	srand(time(0));
	rand(); // To prevent the psuedo random generator depend on increasing time.

	readUnigram(argv[1]);
	readBigram(argv[2]);
	readTrigram(argv[3]);

	int sentenceLength = atoi(argv[4]);
	//generateWord(sentenceLength);
	//test_sampleWord();
	//test_generateWords();
	//test_generateSentences();
	test_generatePartOfSentence();
	
	return 0;
}