#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <codecvt>
#include "utf8.h"

using namespace std;

// How to use: 
// g++ preprocess.cpp
// ./a.out sample.txt sample.out

// 1) Deal with following punctuations: 
// Make [impartial,"] => [impartial , "] ,where punctuations are separated by space
// Make [word...,] => [word ... ,]
// Make ["While] => [" While]
// Make ["...I..."] => [" ... I ... "]

// 2) (?)Deal with abbreviation: at least first char is capital
// Don't separate [Mr.] into [Mr] and [.]
// Missclassify [Yes.] as abbreviation.

// Go around: the end of the sentence shuold be <lowercase word> <.> <uppercase word>
// Wrong: [I told Mr. Brown today's news.], [This country is call Perth. It has ....], [... so R. L. Stevenson has it.]

// 3) Deal with UTF-8: use UTF8-CPP, a small library that could deal with UTF-8.

// 4) Add middle tag <m> between two senteces.
// e.g. <s> I like to eat apples . <m> But my brother doesn't . <e>


struct Word {
	// ["...I..."] => pre: [" ...], mid: [I], post: [... "], cleanword: [" ... I ... "]
	string pre;
	string mid;
	string post;
	string cleanword;
	Word(): pre(""), mid(""), post(""), cleanword("") {};
	Word(string _pre, string _mid, string _post, string _cleanword): 
		pre(_pre), mid(_mid), post(_post), cleanword(_cleanword) {};
};

string UnicodeToUTF8(unsigned int);
bool isEndOfSentence(string);

// Add <m> tag if w1 is the end of the sentence.
string addMidTag(Word w1, Word w2) {
	if (islower(w1.mid[0]) && isEndOfSentence(w1.post) && isupper(w2.mid[0])) {
		return w1.cleanword + " <m>";
	}
	else return w1.cleanword;
}

// Make word into a clean vesion by separating prior and post punctuations. Should deal with UTF-8.
Word clean_UTF8(string word) {
	string preword = "", postword = "";
	int n = utf8::distance(word.begin(), word.end()), i, j;

	uint32_t cur = 0, prev = 0;
	string::iterator b = word.begin(), e = word.end();
	string::iterator it1, it2; 

	for (i = 0, it1 = b; i < n; i++) {
		cur = utf8::next(it1,e);
		if (isalnum(cur)) {
			utf8::previous(it1,b);
			break;
		}

		if (i == 0 || prev == cur) preword = preword + UnicodeToUTF8(cur);
		else preword = preword + " " + UnicodeToUTF8(cur);
		prev = cur;
	}
	
	for (j = n-1, it2 = e; j > i; j--) {
		cur = utf8::previous(it2,b);
		if (isalnum(cur)) {
			utf8::next(it2,e);
			break;
		}

		if (j == 0 || prev == cur) postword = UnicodeToUTF8(cur) + postword;
		else postword = UnicodeToUTF8(cur) + " " + postword;
		prev = cur;
	}

	string midword(it1, it2);

	string cleanword = preword + " " + midword + " " + postword;
	if (cleanword[0] == ' ') cleanword = cleanword.substr(1);
	if (cleanword[cleanword.size()-1] == ' ') cleanword = cleanword.substr(0, cleanword.size()-1);
	
	Word res = Word(preword, midword, postword, cleanword);
	return res;
}

// Determine if words contain the punctuation which shows the end of a sentence.
bool isEndOfSentence(string words) {
	istringstream iss(words);
	string word;

	while (iss >> word) {
		if (word == "." || word == "!" || word == "?") return true;
	}
	return false;
}

// Convert unicode to UTF8 string
string UnicodeToUTF8(unsigned int codepoint){
	string out;

	if (codepoint <= 0x7f)
		out.append(1, static_cast<char>(codepoint));
	else if (codepoint <= 0x7ff) {
		out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
		out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
	}
	else if (codepoint <= 0xffff) {
		out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
		out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
		out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
	}
	else {
		out.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
		out.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
		out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
		out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
	}
	return out;
}

// Tests for clean_UTF8
void test() {
	cout << clean_UTF8("...,...however...,...").cleanword << endl;
	cout << clean_UTF8("I").cleanword << endl;
	cout << clean_UTF8("HelloWorld").cleanword << endl;
	cout << clean_UTF8(".").cleanword << endl;
	cout << clean_UTF8("....").cleanword << endl;
	cout << clean_UTF8(",,,HelloWorld").cleanword << endl;
	cout << clean_UTF8("HelloWorld””””").cleanword << endl;
	cout << clean_UTF8(",,,I").cleanword << endl;
	cout << clean_UTF8("I””””").cleanword << endl;
	cout << clean_UTF8(",I.").cleanword << endl;
	cout << clean_UTF8(",II.").cleanword << endl;
}

int main(int argc, char **argv) {
	
	ifstream infile(argv[1]);
	ofstream outfile(argv[2], ofstream::out);

	string line;
	int countParagraph = 0;

	while (getline(infile, line)) {
		string paragraph = line;
		while (line.find_first_not_of(" \t\n\v\f\r") != string::npos && getline(infile, line)) {
			paragraph = paragraph + " " + line;
		}
		if (paragraph.find_first_not_of(" \t\n\v\f\r") == string::npos) continue;

		istringstream iss(paragraph);
		string word;
		Word pre("","","","");

		outfile << "<s> ";

		while (iss >> word) {
	    	Word cur = clean_UTF8(word);
	    	if (pre.cleanword != "") outfile << addMidTag(pre, cur) << " ";
			pre = cur;
		}
		if (pre.cleanword != "") {
			outfile << pre.cleanword << " ";
			outfile << "<e>\n";
			countParagraph++;
		}
	}
	
	outfile.close();

	cout << "#paragraph: " << countParagraph << endl;

	return 0;
}
