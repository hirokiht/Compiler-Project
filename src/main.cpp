#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include "semanticAnalyzer.hpp"

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
	ofstream ofs1 ("output1.txt"), ofs2 ("output2.txt"), ofs3("output3.tm");
	if(!ofs1.is_open() || !ofs2.is_open() || !ofs3.is_open()){
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
	cout << "Parsed successfully! Performing semantic analysis..." << endl;
	semanticAnalyzer* sa = new semanticAnalyzer(slr);
	//cout << *slr->parseTree2();
	sa->printQuadruples(ofs2);
	cout << "Generating TM code..." << endl;
	sa->printTM(ofs3);
	return 0;
}