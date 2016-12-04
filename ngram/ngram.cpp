#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <queue>
#include <cstdlib>
#include <climits>
#include <cmath>
#include <random>
#include <algorithm>
#include <unordered_map>

using namespace std;

// How to use: ./ngram <unigram_file> <bigram_file> <backward_bigram_file> <trigram_file> <backward_trigram_file>
// g++ -std=c++11 ngram.cpp -o ngram
// ./ngram sample_uni.out sample_bi.out sample_bbi.out sample_tri.out sample_btri.out

// parameters:
// 1) lamda1, lamda2 and lamda3: for different dependency on unigram/bigram/trigram models.
// 2) 

// TODO: this is just a naive way of n gram.
// 1) deal with 2 keyword (maybe more in the future)
// 2) don't output <s> <e> tag when generating sentences

//double lamda1 = 0.0, lamda2 = 0.5, lamda3 = 1 - lamda1 - lamda2;

int shortWordLenUpperBnd = 4, longWordLenLowerBnd = 12;
int shortSentenceLenUpperBnd = 7, longSentenceLenLowerBnd = 20, maxSentenceLen = 50;
int shortParagraphLenUpperBnd = 4, longParagraphLenLowerBnd = 7, maxParagraphLen = 15;

unordered_map<string, double> unigram;
unordered_map<string, unordered_map<string, double>> bigram, trigram, backwardBigram, backwardTrigram;

int num = -1;
string type = "", length = "";
vector<string> keywords;

// util
template <typename T>
string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out << std::setprecision(n) << a_value;
    return out.str();
}

int sentenceLen(string str);

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
	SubSentence() {};
	SubSentence(vector<double> _accumProbs, vector<string> _sentences): accumProbs(_accumProbs), sentences(_sentences) {}

	bool empty() {
		return (sentences.size() == 0);
	}

	string getRandSubSentence(bool byProb) {
		if (sentences.size() == 0) return "";
		if (byProb) {
			// Generate a sentence by probabilities
			double num = rand()/(double)RAND_MAX;
			vector<double>::iterator it;

			if (num == 0) it = upper_bound(accumProbs.begin(), accumProbs.end(), num);
			else it = lower_bound(accumProbs.begin(), accumProbs.end(), num);

			return sentences[it-accumProbs.begin()];
		} else {
			double num = rand() % sentences.size();
			return sentences[num];
		}
	}

	void countSentences(int lenLowerBnd, int lenUpperBnd) {
		vector<string> newresults;
		int count = 0;
		for (string s: sentences) {
			int len = sentenceLen(s);
			if (len >= lenLowerBnd && len <= lenUpperBnd) {
				newresults.push_back(s);
				count++;
				if (count < 10) cout << s << endl;
			}
		}
		cout << count << " sentences meet constraint." << endl;
		cout << "Total " << sentences.size() << " sentences." << endl;
	}

	void printSubSentenceWithProb() {
		int count = 0;
		for (int i = 0; i < sentences.size(); i++) {
			double prob = accumProbs[i] - (i==0 ? 0.0 : accumProbs[i-1]);
			if (prob == 0) continue;
			cout << setw(50) << sentences[i] << " " << to_string_with_precision(prob,10) << endl;
			count++;
		}
		cout << "#subsent = " << count << endl; 
	}
};

struct SubParagraph {
	vector<string> vecParagraph;
	string paragraph;
	int numSentence;

	SubParagraph(string _paragraph): paragraph(_paragraph) {
		initialVecParagraph(_paragraph);
	}

	void initialVecParagraph(string str) {
		istringstream iss(str);
		string cur;
		while (iss >> cur) {
			if (cur != "<s>" && cur != "<e>") {
				vecParagraph.push_back(cur);
			} else numSentence++;
		}
	}

	string getSubParagraph(int pos1, int pos2) {
		string result = "";
		if (pos1 >= 0 && pos1 < vecParagraph.size()) result = vecParagraph[pos1];
		for (int i = pos1+1; i <= pos2; i++) result += " " + vecParagraph[i]; 
		return result;
	}
};

/**
 * Declare functions
 */

void generateSentence(int);
string sampleWord(string prepre, string pre, int direction, double lamda1, double lamda2, double lamda3);

SubSentence generatePartOfSentence(string start, string end, int partOfSentenceLen = maxSentenceLen);
string mergeString(string, string);

void clearTag(string &str);
SubParagraph generatePartOfSentenceFromKeyword(string keyword, int lenLowerBnd, int lenUpperBnd, int direction);
string generatePartOfSentence2(string start, string end, int lenLowerBnd, int lenUpperBnd);

SubParagraph generatePartOfParagraphFromKeyword(string keyword, int numSentence, int direction);
string generatePartOfParagraph(string start, string end, int numSentence);
int numSentenceByLength();

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
			if ((word1 == "." || word1 == "?" || word1 == "!") && word2 != "<e>") continue;
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
			if ((word2 == "." || word2 == "?" || word2 == "!") && word1 != "<e>") continue;
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
	if (keywords.size() > 1) {
		cout << "You should not have more than one keyword for generating words." << endl;
		return;
	}

	if (keywords.size() == 1) {
		if (keywords[0].size() < lenLowerBnd) cout << "Keyword: " << keywords[0] << " is not a LONG word.";
		else if (keywords[0].size() > lenUpperBnd) cout << "Keyword: " << keywords[0] << " is not a SHORT word.";
		else for (int i = 0; i < number; i++) cout << keywords[0] << " ";
		cout << endl;
		return;
	}

	int i = 0;
	vector<string> results;
	for (auto it : unigram) {
		string word = it.first;
		if (isalnum(word[0]) && word.size() >= lenLowerBnd && word.size() <= lenUpperBnd && word != "<s>" && word != "<e>") {
			results.push_back(word);
		}
	}

	if (results.size() == 0) {
		cout << "No word meets the critiria. Try to release the constraint." << endl;
		return;
	}

	for (int i = 0; i < number; i++) {
		int num = rand() % results.size();
		cout << results[num] << " ";
	}
	cout << endl;
}

/**
 * For SENTENCE/SENTENCES
 *
 * This function will generate random sentences.
 * GENERATE (10 SHORT SENTENCES) BY PROBABILITY THEN OUTPUT => generateSentences(10, 1, shortSentenceLenUpperBnd, {""}, 1)
 * GENERATE (10 SHORT SENTENCES) THEN OUTPUT => generateSentences(10, 1, shortSentenceLenUpperBnd, {""}, 0)
 */
void generateSentences(int number, int lenLowerBnd, int lenUpperBnd, vector<string> keywords, bool byProb) {
	if (keywords.size() > 2) {
		cout << "You should not have more than two keyword." << endl;
		return;
	}

	if (keywords.size() == 2) {
		//TODO
		if (lenUpperBnd > 20) { // This method is not good at generating short sentences.
			for (int i = 0; i < number; i++) {
				if (rand()%2) swap(keywords[0], keywords[1]); // change keyword position
				string part2 = generatePartOfSentence2(keywords[0], keywords[1], 1, lenUpperBnd);
				
				int dir = 2 * (rand()%2) - 1;
				if (dir == 1) swap(keywords[0], keywords[1]);
				string part1 = generatePartOfSentenceFromKeyword(keywords[0], 1, max(lenUpperBnd-sentenceLen(part2),0), dir).paragraph;
				string part3 = generatePartOfSentenceFromKeyword(keywords[1], lenLowerBnd-sentenceLen(part2)-sentenceLen(part1), max(lenUpperBnd-sentenceLen(part2)-sentenceLen(part1),0), -dir).paragraph;

				if (part1 == "" || part2 == "" || part3 == "") {
					i--;
					continue;
				}

				string output = (dir==-1) ? mergeString(mergeString(part1, part2), part3) : mergeString(mergeString(part3, part2), part1);
				int len = sentenceLen(output);
				if (len >= lenLowerBnd && len <= lenUpperBnd) {
					cout << output << endl; 
					//cout << len << endl;
				} else i--;
			}
			
		} else {
			int numSen1 = rand()%(number+1), numSen2 = number - numSen1;

			SubSentence subsent1 = generatePartOfSentence("<s>", keywords[0], lenUpperBnd),
						subsent2 = generatePartOfSentence(keywords[0], keywords[1], lenUpperBnd),
						subsent3 = generatePartOfSentence(keywords[1], "<e>", lenUpperBnd);

			if (subsent1.empty() || subsent2.empty() || subsent3.empty()) {
				cout << "Cannot generate sentences with " << keywords[0] << " and " << keywords[1] << "." << endl;
				return;
			}

			for (int i = 0; i < numSen1; i++) {
				string str = mergeString(subsent1.getRandSubSentence(byProb), subsent2.getRandSubSentence(byProb));
				string output = mergeString(str, subsent3.getRandSubSentence(byProb));
				
				int len = sentenceLen(output);
				if (len >= lenLowerBnd && len <= lenUpperBnd) {
					clearTag(output);
					cout << output << endl;
				} else i--;
			}

			swap(keywords[0], keywords[1]);

			subsent1 = generatePartOfSentence("<s>", keywords[0], lenUpperBnd),
			subsent2 = generatePartOfSentence(keywords[0], keywords[1], lenUpperBnd),
			subsent3 = generatePartOfSentence(keywords[1], "<e>", lenUpperBnd);

			if (subsent1.empty() || subsent2.empty() || subsent3.empty()) {
				cout << "Cannot generate sentences with " << keywords[0] << " and " << keywords[1] << "." << endl;
				return;
			}

			for (int i = 0; i < numSen2; i++) {
				string str = mergeString(subsent1.getRandSubSentence(byProb), subsent2.getRandSubSentence(byProb));
				string output = mergeString(str, subsent3.getRandSubSentence(byProb));
				
				int len = sentenceLen(output);
				if (len >= lenLowerBnd && len <= lenUpperBnd) {
					clearTag(output);
					cout << output << endl;
				} else i--;
			}
		}
		return;
	}

	if (keywords.size() == 1) {
		for (int i = 0; i < number; i++) {
			int dir = 2 * (rand()%2) - 1;
			string part1 = generatePartOfSentenceFromKeyword(keywords[0], 1, lenUpperBnd, dir).paragraph;
			string part2 = generatePartOfSentenceFromKeyword(keywords[0], 1, lenUpperBnd-sentenceLen(part1), -dir).paragraph;
			
			if (part1 == "" || part2 == "") {
				i--;
				continue;
			}

			string output = (dir==-1) ? mergeString(part1, part2) : mergeString(part2, part1);
			int len = sentenceLen(output);
			if (len >= lenLowerBnd && len <= lenUpperBnd) cout << output << endl;
			else i--;
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
			//cout << "Length: " << len << endl;
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

int sentenceLen(string str) {
	istringstream iss(str);
	string word;
	int len = 0;
	while (iss >> word) {
		if (word != "<s>" && word != "<e>" && isalnum(word[0])) len++;
	}
	return len;
}

void clearTag(string &str) {
	istringstream iss(str);
	string word, result;
	while (iss >> word) {
		if (word != "<s>" && word != "<e>") result += word + " ";
	}
	str = result;
}

int getIntFromUniformDist(int lower, int upper) {
	int range = upper - lower + 1;
	return rand() % range + lower; 
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
			  double &sum, int &count, bool hashIsStart, int lenUpperBnd) {
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

				if (sentenceLen(sentence) > lenUpperBnd) continue;

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

SubSentence generatePartOfSentence(string start, string end, int lenUpperBnd) {
	unordered_map<string, vector<WordPath>> fromStart, fromEnd;
	queue<WordPath> qFromStart, qFromEnd;
	fromStart[start] = {WordPath(start)};
	fromEnd[end] = {WordPath(end)};
	qFromStart.push(WordPath(start));
	qFromEnd.push(WordPath(end));
	int partOfSentenceLen = 40;
	int numSentences = 1000;
	int iter = 0, maxIter = 1000;

	int turn = 0;
	//vector<string> results;
	//vector<double> probs;
	unordered_map<string, double> results;
	double sum = 0.0;
	int count = 0;

	while (count < numSentences && (!qFromStart.empty() || !qFromEnd.empty()) && iter < maxIter) {
		//cout << "count = " << count << endl;
		if (turn == 0) {
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
							findPath(results, fromEnd, tmp, sum, count, 0, lenUpperBnd);
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
							findPath(results, fromEnd, tmp, sum, count, 0, lenUpperBnd);
						}
					}
				}
			}
		} else {
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
							findPath(results, fromStart, tmp, sum, count, 1, lenUpperBnd);
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
							findPath(results, fromStart, tmp, sum, count, 1, lenUpperBnd);
						}
					}
				}
			}
		}
		turn = !turn;
		iter++;
	}

	if (results.size() == 0) return SubSentence();
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

	//cout << sentences[it-accumProbs.begin()] << endl;
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
	//generateSentences(2, 1, INT_MAX, keywords, 0);
	//cout << endl;
	//generateSentences(1, 1, shortSentenceLenUpperBnd, keywords, 0);
	keywords = {"you", "king"};
	generateSentences(10, 1, shortSentenceLenUpperBnd, keywords, 1);

	keywords = {"<s>", "<s>"};
	generateSentences(10, 1, shortSentenceLenUpperBnd, keywords, 1);
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

	SubSentence subsent1 = generatePartOfSentence("king", "land");
	SubSentence subsent2 = generatePartOfSentence("cat", "run");
	//subsent1.printSubSentenceWithProb();
}

/**
 * ANOTHER WAY
 */
SubParagraph generatePartOfSentenceFromKeyword(string keyword, int lenLowerBnd, int lenUpperBnd, int direction) {
	if (lenUpperBnd < 1) return SubParagraph("");

	string result = keyword;
	string cur = "", pre = keyword, prepre = "";
	int len = 1;
	int iter = 0, maxIter = 100;
	bool reset = false;

	while (iter < maxIter) {
		
		if (len > lenUpperBnd) { //reset
			len = 1;
			result = keyword;
			pre = keyword;
			prepre = "";
			iter++;
		}

		cur = sampleWord(prepre, pre, direction, 0.0, 0.5, 0.5);
		if (direction == 1) {
			if (cur == "<s>" || cur == "<e>") {
				//if (len >= lenLowerBnd && len <= lenUpperBnd) break;
				//else reset = true;
				break;
			}
			if (cur != "<s>" && cur != "<e>") {
				if (isalnum(cur[0])) len++;
				result = result + " " + cur;
			}
		} else {
			if (cur == "<s>" || cur == "<e>") {
				//if (len >= lenLowerBnd && len <= lenUpperBnd) break;
				//else reset = true;
				break;
			}
			if (cur != "<s>" && cur != "<e>") {
				if (isalnum(cur[0])) len++;
				result = cur + " " + result;
			}
		}
		
		prepre = pre;
		pre = cur;
	}

	if (iter >= maxIter) return SubParagraph("");
	//cout << "res: " << result << endl;

	return SubParagraph(result);
}


string generatePartOfSentence2(string start, string end, int lenLowerBnd, int lenUpperBnd) {
	vector<string> results;
	int iter = 0, maxIter = 100;

	while (results.size() == 0 && iter < maxIter) {
		iter++;

		SubParagraph fromStart = generatePartOfSentenceFromKeyword(start, lenLowerBnd, lenUpperBnd, 1);
		SubParagraph fromEnd = generatePartOfSentenceFromKeyword(end, lenLowerBnd-sentenceLen(fromStart.paragraph), lenUpperBnd-sentenceLen(fromStart.paragraph), -1);
		unordered_map<string, vector<int>> wordsFromStart, wordsFromEnd;

		for (int i = 0; i < fromStart.vecParagraph.size(); i++) {
			string cur = fromStart.vecParagraph[i];
			wordsFromStart[cur].push_back(i);
		}

		for (int i = fromEnd.vecParagraph.size()-1; i >= 0; i--) {
			string cur = fromEnd.vecParagraph[i];
			wordsFromEnd[cur].push_back(i);
		}

		for (auto it : wordsFromStart) {
			string word = it.first;
			vector<int> pos1 = it.second;
			if (wordsFromEnd.count(word) == 0) continue;

			for (int i = 0; i < pos1.size(); i++) {
				vector<int> pos2 = wordsFromEnd[word];
				for (int j = 0; j < pos2.size(); j++) {
					string part1 = fromStart.getSubParagraph(0, pos1[i]), 
						   part2 = fromEnd.getSubParagraph(pos2[j], fromEnd.vecParagraph.size()-1);
					//cout << "PART1: " << part1 << endl << "PART2: " << part2 << endl;
					results.push_back(mergeString(part1, part2));
				}
			}
		}
	}

	if (results.size() == 0 && iter >= maxIter) return "";
	int num = rand() % results.size();
	return results[num];
}


/**
 * For PARAGRAPH/PARAGRAPHS
 *
 * GENERATE (10 PARAGRAPHS) THEN OUTPUT => generateSentences(10, 1, INT_MAX, {""}, 0)
 * GENERATE (10 SHORT SENTENCES) THEN OUTPUT => generateSentences(10, 1, shortSentenceLenUpperBnd, {""}, 0)
 */

void generateParagraphs(int number, int lenLowerBnd, int lenUpperBnd, vector<string> keywords, bool byProb) {
	random_device rd;
	mt19937 gen(rd());

	normal_distribution<double> d1(3,1), d2(1.5,0.5);
	int numSentence = 1;
	

	if (keywords.size() > 2) {
		cout << "You should not have more than two keyword." << endl;
		return;
	}

	if (keywords.size() == 2) {
		int maxIter = 10000, iter = 0;
		for (int i = 0; i < number; i++) {
			numSentence = numSentenceByLength();

			if (iter > maxIter) {
				cout <<  "Cannot generate paragraph with " << keywords[0] << " and " << keywords[1] << "." << endl;
				break;
			}

			int numSentences1 = max(round(d2(gen)),1.0), numSentences2 = max(round(d2(gen)),2.0), 
				numSentences3 = max(round(numSentence - numSentences1 - numSentences2),1.0);
			string part1 = generatePartOfParagraphFromKeyword(keywords[0], numSentences1, -1).paragraph;
			string part2 = generatePartOfParagraph(keywords[0], keywords[1], numSentences2);
			string part3 = generatePartOfParagraphFromKeyword(keywords[1], numSentences3, 1).paragraph;
			if (part1 == "" || part2 == "" || part3 == "") {
				i--;
				continue;
			}
			string tmp = mergeString(part1, part2);
			string output = mergeString(tmp, part3);
			clearTag(output);
			cout << output << endl << endl;

			iter++;
		}
		return;
	}

	

	if (keywords.size() == 1) {
		for (int i = 0; i < number; i++) {
			numSentence = numSentenceByLength();

			int numSentences1 = max(round(d1(gen)),1.0);
			int numSentences2 = max(round(numSentence-numSentences1),1.0);
			SubParagraph part1 = generatePartOfParagraphFromKeyword(keywords[0], numSentences1, -1);
			SubParagraph part2 = generatePartOfParagraphFromKeyword(keywords[0], numSentences2, 1);
			string output = mergeString(part1.paragraph, part2.paragraph);
			clearTag(output);
			cout << output << endl << endl;
		}
		return;
	}

	for (int i = 0; i < number; i++) {
		numSentence = numSentenceByLength();
		//SubParagraph pg = generatePartOfParagraphFromKeyword("<s>", numSentence, 1);
		string output = generatePartOfParagraphFromKeyword("<s>", numSentence, 1).paragraph;
		clearTag(output);
		cout << output << endl << endl;
	}
}

int numSentenceByLength() {
	random_device rd;
	mt19937 gen(rd());
	normal_distribution<double> d(6,1.5);
	int numSentence;

	if (length == "SHORT") numSentence = getIntFromUniformDist(1, shortParagraphLenUpperBnd);
	else if (length == "LONG") numSentence = getIntFromUniformDist(longParagraphLenLowerBnd, maxParagraphLen);
	else numSentence = max(round(d(gen)),1.0);

	return numSentence;
}

// direction = +1: forward
//           = -1: backward
SubParagraph generatePartOfParagraphFromKeyword(string keyword, int numSentence, int direction) {
	string result = keyword;
	string cur = "", pre = keyword, prepre = "";
	int count = 0;
	while (count < numSentence) {
		cur = sampleWord(prepre, pre, direction, 0.0, 0.5, 0.5);
		if (direction == 1) {
			result = result + " " + cur;
			if (cur == "<e>") count++;
		} else {
			result = cur + " " + result;
			if (cur == "<e>" || cur == "<s>") {
				count++;
				cur = "<e>";
			}
		}
		
		prepre = pre;
		pre = cur;
	}

	return SubParagraph(result);
}

string generatePartOfParagraph(string start, string end, int numSentence) {
	SubParagraph fromStart = generatePartOfParagraphFromKeyword(start, numSentence, 1);
	SubParagraph fromEnd = generatePartOfParagraphFromKeyword(end, numSentence, -1);
	unordered_map<string, vector<int>> wordsFromStart, wordsFromEnd;
	int posOfFirstTagFromStart = -1, posOfLastTagFromEnd = -1;

	vector<string> results;

	for (int i = 0; i < fromStart.vecParagraph.size(); i++) {
		string cur = fromStart.vecParagraph[i];
		if (cur == "<e>" || cur == "<s>") posOfFirstTagFromStart = i;
		if (cur != "<e>" && cur != "<s>" && posOfFirstTagFromStart != -1) {
			wordsFromStart[cur].push_back(i);
		}
	}

	for (int i = fromEnd.vecParagraph.size()-1; i >= 0; i--) {
		string cur = fromEnd.vecParagraph[i];
		if (cur == "<e>" || cur == "<s>") posOfLastTagFromEnd = i;
		if (cur != "<e>" && cur != "<s>" && posOfLastTagFromEnd != -1) {
			wordsFromEnd[cur].push_back(i);
		}
	}

	for (auto it : wordsFromStart) {
		string word = it.first;
		vector<int> pos1 = it.second;
		if (wordsFromEnd.count(word) == 0) continue;

		for (int i = 0; i < pos1.size(); i++) {
			vector<int> pos2 = wordsFromEnd[word];
			for (int j = 0; j < pos2.size(); j++) {
				string part1 = fromStart.getSubParagraph(0, pos1[i]), 
					   part2 = fromEnd.getSubParagraph(pos2[j], fromEnd.vecParagraph.size()-1);
				//cout << "PART1: " << part1 << endl << "PART2: " << part2 << endl;
				results.push_back(mergeString(part1, part2));
			}
		}
	}
	//cout << "Results size = " << results.size() << endl;

	if (results.size() == 0) return "";
	int num = rand() % results.size();
	return results[num];
}

void test_generateParagraphs() {
	vector<string> keywords;
	//generateParagraphs(3, 1, 1, keywords, 0);

	keywords = {"war"};
	//generateParagraphs(5, 1, 1, keywords, 0);

	//cout << generatePartOfParagraph("war", "English", 3) << endl;

	keywords = {"here", "home"};
	generateParagraphs(5, 1, 1, keywords, 0);

}

int main(int argc, char **argv) {
	srand(time(0));
	rand(); // To prevent the psuedo random generator depend on increasing time.

	if (argc < 6) {
		cout << "Usage: ./ngram [unigram_file] [bigram_file] [backward_bigram_file] [trigram_file] [backward_trigram_file] ";
		cout << "-num [number] -type [WORD/SENTENCE/PARAGRAPH] [-length [SHORT/LONG]] [-keywords [list of keywords]]" << endl;
		cout << "E.g. ./ngram 1.uni 1.bi 1.bbi 1.tri 1.btri -num 3 -type PARAGRAPH -length LONG -keywords home king" << endl;
		return 0;
	}

	readUnigram(argv[1]);
	readBigram(argv[2]);
	readBackwardBigram(argv[3]);
	readTrigram(argv[4]);
	readBackwardTrigram(argv[5]);

	for (int i = 6; i < argc; i++) {
		string arg(argv[i]);
		if (arg == "-num") {
			num = atoi(argv[i+1]);
		} else if (arg == "-type") {
			string tmp(argv[i+1]);
			type = tmp;
		} else if (arg == "-length") {
			string tmp(argv[i+1]);
			length = tmp;
		} else if (arg == "-keywords") {
			int count = 0;
			while (i+1 < argc && argv[i+1][0] != '-') {
				string tmp(argv[i+1]);
				keywords.push_back(tmp);
				i++;
				count++;
			}
		}
	}

	if (num < 0) {
		cout << "There should be -num [number] in your arguments." << endl;
		return 0;
	}

	if (type == "") {
		cout << "There should be -type [WORD/SENTENCE/PARAGRAPH] in your arguments." << endl;
		return 0;
	}

	//Not support more than 2 keywords
	if (keywords.size() > 2) {
		cout << "You should not have more than two keywords." << endl;
		return 0;
	}

	if (type == "WORD") {
		if (length == "SHORT") generateWords(num, 1, shortWordLenUpperBnd, keywords);
		else if (length == "LONG") generateWords(num, longWordLenLowerBnd, INT_MAX, keywords);
		else generateWords(num, 1, INT_MAX, keywords);
	} else if (type == "SENTENCE") {
		if (length == "SHORT") generateSentences(num, 1, shortSentenceLenUpperBnd, keywords, 0);
		else if (length == "LONG") generateSentences(num, longSentenceLenLowerBnd, maxSentenceLen, keywords, 0);
		else generateSentences(num, 1, maxSentenceLen, keywords, 0);
	} else if (type == "PARAGRAPH") {
		if (length == "SHORT") generateParagraphs(num, 1, shortParagraphLenUpperBnd, keywords, 0);
		else if (length == "LONG") generateParagraphs(num, longParagraphLenLowerBnd, maxParagraphLen, keywords, 0);
		else generateParagraphs(num, 1, maxParagraphLen, keywords, 0);
	} else {
		cout << "Undefined type. Acceptable types are WORD, SENTENCE, PARAGRAPH." << endl;
	}


	//int sentenceLength = atoi(argv[6]);
	//generateWord(sentenceLength);
	//test_sampleWord();
	//test_generateWords();
	//test_generateSentences();
	//test_generatePartOfSentence();
	//test_generateParagraphs();

	return 0;
}