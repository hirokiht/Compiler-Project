#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include "SLRparser.hpp"

using namespace std;

enum type {NONE,BOOLEAN,BYTE,SHORT,INT,LONG,FLOAT,DOUBLE,CHAR,STRING};

typedef struct _symbol{
	string name;
	type T;
	bool initialized;
	NODE* scope;
} SYMBOL;

typedef struct _quadruple{
	string op;
	unsigned int arg1;
	unsigned int arg2;
	SYMBOL* sym;
} QUADRUPLE;

inline ostream& operator<<(ostream& os, const QUADRUPLE qr){
	return os << "op:" << qr.op << '\t' << qr.arg1 << '\t' << qr.arg2 << '\t' << (qr.sym==NULL? "" : qr.sym->name);
}

class semanticAnalyzer{
	protected:
	NODE* root;						//tree's root
	vector<SYMBOL> symbolTable;
	vector<QUADRUPLE> temp;
	queue<unsigned int> ifstack;
	queue<unsigned int> elsestack;
	bool valid;
	void init(NODE*);
	
	public:
	semanticAnalyzer(NODE*);
	semanticAnalyzer(SLRparser*);
	
	bool isDescendant(NODE*,NODE*);
	void printQuadruples(ostream&);
	void printTM(ostream&);
	char getFreeReg(unsigned char*);
};