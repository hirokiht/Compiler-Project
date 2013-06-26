#include <iostream>
#include <string>
#include <list>
#include <map>
#include "lexAnalyzer.hpp"

using namespace std;

#ifndef PRODS
#define PRODS

typedef struct _reduce{
	TOKEN n;
	list<TOKEN> body;
	unsigned int r;
} REDUCE;

typedef struct _state{
	unsigned int label;
	multimap<TOKEN,_state*> transitionS;
	multimap<TOKEN,REDUCE>transitionR;
	bool accept;

	bool operator < (const _state& st) const{
		return label < st.label;
	}
	
	bool operator > (const _state& st) const{
		return label > st.label;
	}

	bool operator == (const _state& st) const{
		return label == st.label;
	}

	bool operator != (const _state& st) const{
		return label != st.label;
	}
} STATE;

typedef struct _production{
	TOKEN head;
	list<TOKEN> body;

	bool operator < (const _production& prod) const{
		return head < prod.head;
	}
	
	bool operator > (const _production& prod) const{
		return head > prod.head;
	}

	bool operator == (const _production& prod) const{
		return head == prod.head;
	}

	bool operator != (const _production& prod) const{
		return head != prod.head;
	}
} PRODUCTION;

inline ostream& operator<<(ostream& os, const _production& prods){
	return os << prods.head << " =>" << prods.body;
}

class parserGenerator{
	protected:
	list<PRODUCTION> production;
	map<TOKEN,bool> nullableTable;
	multimap<TOKEN,TOKEN> firstTable;
	multimap<TOKEN,TOKEN> followTable;
	multimap<TOKEN,TOKEN> follower;
	multimap<PRODUCTION,STATE> prodstate;		//stores the leading states for each productions
	map<unsigned int,STATE*> states;		//map the NFA states which stored in prodstate with state label for the ease of random direct access
	multimap<unsigned int,STATE*> enclosure;	//epsilon enclosed states
	map<STATE*,list<STATE*> > dfa;			//stores generated DFA and it's corresponded NFA states

	public:
	parserGenerator(istream&);
	parserGenerator(string);

	bool add(string);
	bool isTerminal(TOKEN);
	bool isTerminal(string);
	list<TOKEN> getNonTerminals();
	list<PRODUCTION> getProductions(char *);
	list<PRODUCTION> getProductions(string);
	bool nullable(PRODUCTION);
	bool nullable(TOKEN);
	list<TOKEN> first(PRODUCTION);
	list<TOKEN> first(TOKEN);
	void insertFollow(TOKEN,TOKEN);
	void follow(PRODUCTION);
	void follow(TOKEN);
	void init();
	void output(ostream&);
	list<STATE*> get_enclosure(STATE*);
	list<STATE*> get_enclosure(list<STATE*>,STATE*);
	STATE* insertDFA(list<STATE*>);
	map<STATE*,list<STATE*> > getDFA();
	map<unsigned int,STATE*> getParseTable();
	list<PRODUCTION> getProductions();
};
#endif