#include <iostream>
#include <string>
#include <list>
#include <map>
#include <vector>
#include "parserGenerator.hpp"

using namespace std;

typedef struct _node{
	_node* parent;
	vector<_node*> childs;
	TOKEN* tok;
	unsigned int level;
} NODE;

inline ostream& operator<<(ostream& os, const NODE& nod){
	os << *nod.tok;
	for(unsigned int c = 0 ; c < nod.childs.size() ; c++)
		os << ' ' << *nod.childs[c]->tok;
	return os << endl;
}

inline ostream& operator<<(ostream& os, const NODE* nod){
	os << *nod->tok << endl;
	for(unsigned int c = 0 ; c < nod->childs.size() ; c++){
		for(unsigned int i = 0 ; i < nod->level ; i++)
			os << "  ";
		os << '|' << (c+1 == nod->childs.size()? "__" : "--") << nod->childs[c];
	}
	return os;
}

class SLRparser{
	protected:
	map<unsigned int,STATE*> parseTable;			//stores generated DFA and it's corresponded NFA states
	list<TOKEN*> tokens;
	list<PRODUCTION> reduce;
	NODE* root;						//tree's root
	bool valid;
	
	public:
	SLRparser(map<unsigned int,STATE*>);
	SLRparser(map<unsigned int,STATE*>,list<TOKEN*>);
	
	bool parse();
	bool parse(list<TOKEN*>);
	string output();
	string parseTree(PRODUCTION* = NULL);
	NODE* parseTree2();
	bool isValid();
};