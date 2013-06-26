#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <list>
#include "SLRparser.hpp"

using namespace std;

int main(int argc, char ** argv){
	if(argc < 3){
		cerr << "Received " << argc-1 << " arguments, kindly input in the following format:" << endl << "<program name> <grammer filename> <java source code filename>" << endl;
		return 1;
	}
	ifstream ifs (argv[1]), ifs2 (argv[2]);
	if(!ifs.is_open() || !ifs2.is_open()){
		cerr << "Unable to open the " << (ifs.is_open()? "grammer" : "java") << " file(" << argv[1] << "), kindly check the filename or permission!" << endl;
		return 2;
	}
	ofstream ofs1 ("output1.txt"), ofs2 ("output2.txt");
	if(!ofs1.is_open() || !ofs2.is_open()){
		cerr << "Unable to open the output files for saving..." << endl;
		cerr << "Output(s) will be printed on default stream (typically screen)!" << endl;
	}
	parserGenerator * p = new parserGenerator(ifs);
	lexAnalyzer* lex = new lexAnalyzer(ifs2);
	if(!lex->isValid()){
		cerr << "Incorrect Lexcial! Program Aborted!" << endl;
		return 3;
	}else	cout << "Lexical is Correct! Proceed to Syntax Analysis!" << endl;
	lex->symbolTable(ofs1);
	SLRparser* slr = new SLRparser(p->getParseTable(),lex->getTokenList());
	if(!slr->isValid()){
		cerr << "Syntax Incorrect!!!" << endl;
		return 4;
	}
	cout << "Parsed successfully! " << endl;
	ofs2 << slr->parseTree();
	cout << slr->parseTree2();
	cout << *slr->parseTree2();
	return 0;
}