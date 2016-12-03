#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <codecvt>
#include <set>
#include <locale>
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

// 2) (?) Deal with abbreviation: at least first char is capital
// Don't separate [Mr.] into [Mr] and [.]
// Missclassify [Yes.] as abbreviation.
// New solution: Check against common abbreviations. 
// Problem: This will fail for some abbreviations we haven't hardcoded.

// 3) Deal with UTF-8: use UTF8-CPP, a small library that could deal with UTF-8.

// TODO:
// *1) remove unnecessary parts of text: 
// e.g. in 53537-0.txt, remove line 1 - 116(begin), line 5735 - 6254 (end)
// e.g. remove [Illustration: PERTH FROM THE SLOPES OF KINNOULL HILL], and combine the paragraph
// e.g. subtitles

// *2) append indented paragraph to its previous paragraph

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
bool isAllUpperWord(string);
bool isEndOfSentence(string);
bool isAbbreviation(string);

// Add <m> tag if w1 is the end of the sentence.
string addMidTag(Word w1, Word w2) {
	if (!isAbbreviation(w1.mid) && isEndOfSentence(w1.post) && isupper(w2.mid[0])) {
		return w1.cleanword + " <m>";
	}
	return w1.cleanword;
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

	if ((isAbbreviation(midword) || isAllUpperWord(midword)) && postword.size() > 0 && postword[0] == '.'){
		midword += ".";
		if (postword.size() > 1 && postword[1] == ' ') postword = postword.substr(2);
		else postword = postword.substr(1);
	}

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

// Common abbreviations 
const string tmp[] = {
	"Dr",
	"Mr",
	"Mrs",
	"Ms",
	"rd",
	"st",
	"etc",
	"Gen",
	"Hon",
	"Prof",
	"Rev",
	"Sr",
	"Jr",
	"St",
	"ave",
	"Dept",
	"Inc",
	"Mt",
	"No",
};
const set<string> ABBREVIATIONS(tmp, tmp + sizeof(tmp) / sizeof(tmp[0]));

// Checks if a word is in the list of common abbreviations
bool isAbbreviation(string word) {
	//transform(word.begin(), word.end(), word.begin(), ::tolower);
	if (isupper(word[0]) && word.size() == 1) return true;

	return ABBREVIATIONS.find(word) != ABBREVIATIONS.end();
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

bool isAllUpperWord(string str) {
	// all are A-Z a-z
	string result = "";
	int n = utf8::distance(str.begin(), str.end()), i;

	uint32_t cur = 0;
	string::iterator b = str.begin(), e = str.end();
	string::iterator it;

	for (i = 0, it = b; i < n; i++) {
		cur = utf8::next(it,e);
		if (!isupper(cur)) return false;
	}
	return true;
}

string getLastChar(string paragraph) {
	string::iterator it, b = paragraph.begin();
	uint32_t cur = 0;

	for (it = paragraph.end(); it != paragraph.begin(); ) {
		cur = utf8::previous(it, b);
		if (UnicodeToUTF8(cur) != " ") return UnicodeToUTF8(cur);
	}
	return "";
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
	cout << clean_UTF8("_etc._").cleanword << endl;
	cout << clean_UTF8("etc...").cleanword << endl;
	cout << clean_UTF8(",,,etc.").cleanword << endl;

	cout << isAllUpperWord("HelloWorld””””Iam") << endl;
	cout << isAllUpperWord("11AT") << endl;
	cout << isAllUpperWord("AT") << endl;
}

int main(int argc, char **argv) {
	
	if (argc != 3) {
		cerr << "Usage: ./preprocess <input_file> <output_file>" << endl;
		return 0;
	}

	ifstream infile(argv[1]);
	ofstream outfile(argv[2], ofstream::out);

	string line;
	int countParagraph = 0;

	while (getline(infile, line)) {
		int countLine = 0;
		string paragraph = line;
		while (line.find_first_not_of(" \t\n\v\f\r") != string::npos && getline(infile, line)) {
			paragraph = paragraph + " " + line;
			countLine++;
		}
		if (paragraph.find_first_not_of(" \t\n\v\f\r") == string::npos) continue;

		// If the number of lines in the paragraph is 1, then don't parse the paragraph.
		if (countLine <= 2) continue;

		// If the last char in the paragraph is invalid, then don't parse the paragraph.
		string lastChar = getLastChar(paragraph);
		if (lastChar != "." && lastChar != "?" && lastChar != "!" && lastChar != "”") continue;

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

	test();
	return 0;
}
