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

/**
 * Define Class
 */

// Record the path from word1 to word2, and the probability of the path
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

// Record possible paths (not all) from word1 to word2, and their accumulative probabilities
struct SubSentence {
	vector<double> accumProbs;
	vector<string> sentences;
	SubSentence(vector<double> _accumProbs, vector<string> _sentences): accumProbs(_accumProbs), sentences(_sentences) {}
};

/**
 * Declare functions
 */

void generateSentence(int);
string sampleWord(string prepre, string pre, int direction, double lamda1, double lamda2, double lamda3);

SubSentence generatePartOfSentence(string, string);
string mergeString(string, string);


void clear(std::queue<WordPath> &q)
{
	std::queue<WordPath> empty;
	std::swap( q, empty );
}

/**
 * Read unigram, bigram, backwardBigram, trigram, backwardTrigram files
 */

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
			//if (word2 == "<e>" && isalnum(word1[0])) continue;
			bigram[word1][word2] = prob;
		}
	}
}

void readBackwardBigram(char *filename) {
	ifstream infile(filename);
	string line;

	while (getline(infile, line)) {	
		istringstream iss(line);
		string word1, word2;
		double prob;

		while (iss >> word1 >> word2 >> prob) {
			backwardBigram[word1][word2] = prob;
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
			//if (word3 == "<e>" && isalnum(word2[0])) continue;
			trigram[word1 + " " + word2][word3] = prob;
		}
	}
}

void readBackwardTrigram(char *filename) {
	ifstream infile(filename);
	string line;

	while (getline(infile, line)) {	
		istringstream iss(line);
		string word1, word2, word3;
		double prob;

		while (iss >> word1 >> word2 >> word3 >> prob) {
			backwardTrigram[word1 + " " + word2][word3] = prob;
		}
	}
}

/**
 * Trivial example for generating words.
 */

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

string sampleWord(string prepre, string pre, int direction, double lamda1, double lamda2, double lamda3) {
	// prepre: the string that is two words before
	// pre: the string that is one word before
	// direction: +1 means forward, -1 means backward
	
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

/**
 * For WORD/WORDS
 *
 * This function will generate random words.
 * GENERATE (10 SHORT WORDS) THEN OUTPUT => generateWords(10, 1, shortWordLenUpperBnd, {""})
 * GENERATE (5 WORDS) ON "cat" THEN OUTPUT => generateWords(5, 1, INT_MAX, {"cat"})
 */

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

/**
 * For SENTENCE/SENTENCES
 *
 * This function will generate random sentences.
 * GENERATE (10 SHORT SENTENCES) THEN OUTPUT => generateSentences(10, 1, shortSentenceLenUpperBnd, {""})
 */

void generateSentences(int number, int lenLowerBnd, int lenUpperBnd, vector<string> keywords) {
	if (keywords.size() > 2) {
		cout << "You should not have more than two keyword." << endl;
		return;
	}

	if (keywords.size() == 2) {
		//TODO
	}

	if (keywords.size() == 1) {
		SubSentence subsent1 = generatePartOfSentence("<s>", keywords[0]);
		SubSentence subsent2 = generatePartOfSentence(keywords[0], "<e>");
		for (int i = 0; i < number; i++) {
			int num1 = rand() / subsent1.sentences.size();
			int num2 = rand() / subsent2.sentences.size();
			cout << mergeString(subsent1.sentences[num1], subsent2.sentences[num2]) << endl;
		}
		return;
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

string mergeString(string s1, string s2) {
	size_t pos1 = s1.find_last_of(" ");
	size_t pos2 = s2.find_first_of(" ");
	if (pos1 != string::npos) return s1.substr(0, pos1) + " " + s2;
	else return s1 + " " + s2.substr(pos2+1);
}

string mergeWordPath(WordPath wp1, WordPath wp2) {
	return mergeString(wp1.totalPath, wp2.totalPath);
}

/*
string mergeWordPath(WordPath wp1, WordPath wp2) {
	size_t pos1 = wp1.totalPath.find_last_of(" ");
	size_t pos2 = wp2.totalPath.find_first_of(" ");
	if (pos1 != string::npos) return wp1.totalPath + "---" + wp2.totalPath; //wp1.totalPath.substr(0, pos1) + " " + wp2.totalPath;//
	else return wp1.totalPath + "---" + wp2.totalPath; //wp1.totalPath + " " + wp2.totalPath.substr(pos2+1);//
}
*/

void findPath(unordered_map<string, double> &results, unordered_map<string, vector<WordPath>> &hash, WordPath wp,
			  double &sum, int &count, bool hashIsStart) {
	string word = wp.pre;
	if (hash.count(word) != 0) {
		vector<WordPath> vwp = hash[word];
		for (int i = 0; i < vwp.size(); i++) {
				string sentence;
				if (hashIsStart) {
					sentence = mergeWordPath(vwp[i], wp);
				} else { 
					sentence = mergeWordPath(wp, vwp[i]);
				}

				double prob = vwp[i].pathProb * wp.pathProb;
				//results.push_back(sentence);
				//probs.push_back(prob);
				if (results.count(sentence) != 0) {
					sum = sum - results[sentence];
					results[sentence] = max(results[sentence], prob);
					sum += results[sentence];
				} else {
					sum += prob;
					results[sentence] = prob;
					count++;
				}
			}
	}
}

SubSentence generatePartOfSentence(string start, string end) {
	unordered_map<string, vector<WordPath>> fromStart, fromEnd;
	queue<WordPath> qFromStart, qFromEnd;
	fromStart[start] = {WordPath(start)};
	fromEnd[end] = {WordPath(end)};
	qFromStart.push(WordPath(start));
	qFromEnd.push(WordPath(end));
	int partOfSentenceLen = 20;
	int numSentences = 100000;

	int turn = 0;
	//vector<string> results;
	//vector<double> probs;
	unordered_map<string, double> results;
	double sum = 0.0;
	int count = 0;

	while (count < numSentences && (!qFromStart.empty() || !qFromEnd.empty())) {
		//cout << "count = " << count << endl;
		switch (turn) {
			case 0 :
				//cout << "case 0" << endl;
				if (!qFromStart.empty()) {
					WordPath wp = qFromStart.front();
					qFromStart.pop();

					if (wp.len > partOfSentenceLen) break;
					if (wp.pre == "<e>") continue;

					if (bigram[wp.pre].size() == 0) {
						//cout << "1:" << endl;
						for (auto it : unigram) {
							if (it.first == "<s>") continue;

							WordPath tmp = wp;
							string word = it.first;
							double prob = unigram[word];
							
							if (unigram[word] > 0 && word != "<e>") {
								tmp.add(word, prob);
								fromStart[word].push_back(tmp);
								qFromStart.push(tmp);
								findPath(results, fromEnd, tmp, sum, count, 0);
							}
						}
					} else {
						//cout << "2:" << endl;
						for (auto it : bigram[wp.pre]) {
							WordPath tmp = wp;
							string word = it.first;
							double prob = 0.5 * (it.second) + 0.5 * trigram[tmp.prepre + " " + tmp.pre][word];
							if (prob > 0 && word != "<e>") {
								tmp.add(word, prob);
								fromStart[word].push_back(tmp);
								qFromStart.push(tmp);
								findPath(results, fromEnd, tmp, sum, count, 0);
							}
						}
					}
				}
				break;

			case 1:
				//cout << "case 1" << endl;
				if (!qFromEnd.empty()) {
					WordPath wp = qFromEnd.front();
					qFromEnd.pop();

					if (wp.len > partOfSentenceLen) break;
					if (wp.pre == "<s>") continue;

					if (backwardBigram[wp.pre].size() == 0) {
						//cout << "1:" << endl;
						for (auto it : unigram) {
							if (it.first == "<e>") continue;

							WordPath tmp = wp;
							string word = it.first;
							double prob = unigram[word];

							if (unigram[word] > 0 && word != "<e>" && word != "<s>") {
								tmp.addBackward(word, prob);
								fromEnd[word].push_back(tmp);
								qFromEnd.push(tmp);
								findPath(results, fromStart, tmp, sum, count, 1);
							}
						}
					} else {
						//cout << "2:" << endl;
						for (auto it : backwardBigram[wp.pre]) {
							WordPath tmp = wp;
							string word = it.first;
							double prob = 0.5 * (it.second) + 0.5 * backwardTrigram[tmp.prepre + " " + tmp.pre][word];
							if (prob > 0 && word != "<e>" && word != "<s>") {
								tmp.addBackward(word, prob);
								fromEnd[word].push_back(tmp);
								qFromEnd.push(tmp);
								findPath(results, fromStart, tmp, sum, count, 1);
							}
						}
					}
				}
				break;
			
			default:
				break;
		}
		turn = !turn;
	}

	//cout << "start cleaning queue;" << endl;
	//clear(qFromStart);
	//cout << "finish qFromStart." << endl;

	
	//cout << "start cleaning queue;" << endl;
	//clear(qFromEnd);
	//cout << "finish qFromEnd." << endl;

	

	int countFromStart = 0, countFromEnd = 0;
	for (auto it : fromStart) {
		countFromStart += it.second.size();
		vector<WordPath> vwp = it.second;
		//for (int i = 0; i < it.second.size(); i++) cout << vwp[i].totalPath << endl;
	}
	//cout << "------" << endl;
	for (auto it : fromEnd) {
		countFromEnd += it.second.size();
		vector<WordPath> vwp = it.second;
		//for (int i = 0; i < it.second.size(); i++) cout << vwp[i].totalPath << endl;
	}
	//cout << "fromStart.size()= " << fromStart.size() << ", fromEnd.size()= " << fromEnd.size() << endl;
	//cout << "countFromStart= " << countFromStart << ", countFromEnd= " << countFromEnd << endl;
	//return "";

	int i = 0;
	vector<double> accumProbs(results.size(),0);
	vector<string> sentences(results.size(), "");
	for (auto it : results) {
		accumProbs[i] = (i==0 ? 0.0 : accumProbs[i-1]) + it.second/sum;
		sentences[i] = it.first;
		//cout << sentences[i] << ": " << accumProbs[i] << endl;
		i++;
	}
	//for (int i = 0; i < probs.size(); i++) accumProbs[i] = (i==0 ? 0.0 : accumProbs[i-1]) + probs[i]/sum;
	//for (int i = 0; i < 100; i++) cout << accumProbs[i] << endl;

	double num = rand()/(double)RAND_MAX;
	//cout << num << " " << endl;
	vector<double>::iterator it;

	if (num == 0) it = upper_bound(accumProbs.begin(), accumProbs.end(), num);
	else it = lower_bound(accumProbs.begin(), accumProbs.end(), num);

	cout << sentences[it-accumProbs.begin()] << endl;
	//int randPos = rand() % sentences.size();
	//cout << sentences[randPos] << endl;

	return SubSentence(accumProbs, sentences);
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
	//generateSentences(2, 1, INT_MAX, keywords);
	//cout << endl;
	//generateSentences(1, 1, shortSentenceLenUpperBnd, keywords);
	keywords = {"a"};
	generateSentences(10, 1, shortSentenceLenUpperBnd, keywords);
}

void test_generatePartOfSentence() {
	/* Test: mergeWordPath
	WordPath wp1("cat"), wp2("dog");
	wp1.add("likes", 0.3); wp2.addBackward("likes",0.2);
	cout << mergeWordPath(wp1,wp2) << endl;
	*/
	generatePartOfSentence("<s>", "dubious");
	generatePartOfSentence("dubious", "made");
	generatePartOfSentence("made", "<e>");

	generatePartOfSentence("<s>", "<e>");	
	//cout << generatePartOfSentence("<s>", "made") << endl;
}

int main(int argc, char **argv) {
	srand(time(0));
	rand(); // To prevent the psuedo random generator depend on increasing time.

	if (argc != 6) cout << "Usage: ./ngram <unigram_file> <bigram_file> <backward_bigram_file> <trigram_file> <backward_trigram_file>" << endl;

	readUnigram(argv[1]);
	readBigram(argv[2]);
	readBackwardBigram(argv[3]);
	readTrigram(argv[4]);
	readBackwardTrigram(argv[5]);

	//int sentenceLength = atoi(argv[6]);
	//generateWord(sentenceLength);
	//test_sampleWord();
	//test_generateWords();
	test_generateSentences();
	//test_generatePartOfSentence();
	
	return 0;
}