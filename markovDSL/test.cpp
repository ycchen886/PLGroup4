#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char **argv) {

	int num;
	int count = 0;
	string type = "", length = "";
	vector<string> keywords;

	for (int i = 0; i < argc; i++) {
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
			
			while (i+1 < argc && argv[i+1][0] != '-') {
				string tmp(argv[i+1]);
				keywords.push_back(tmp);
				i++;
				count++;
			}
		}
	}


	cout << "Testing!" << endl;

	cout << "Num = " << num << endl;
	cout << "Type = " << type << endl;
	cout << "Length = " << length << endl;
	cout << "You gave " << count << " keywords" << endl;
}