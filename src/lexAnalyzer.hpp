#include <iostream>
#include <list>
#include <string>

using namespace std;

typedef struct _token{
	string name;
	string val;
	bool isTerminal;

	bool operator < (const _token& tok) const{
		if(name.compare(tok.name))
			return name.compare(tok.name) < 0;
		else if(val.compare(tok.val))
			return val.compare(tok.val) < 0;
		else	return isTerminal < tok.isTerminal;
	}
	
	bool operator > (const _token& tok) const{
		if(name.compare(tok.name))
			return name.compare(tok.name) > 0;
		else if(val.compare(tok.val))
			return val.compare(tok.val) > 0;
		else	return isTerminal > tok.isTerminal;
	}

	bool operator == (const _token& tok) const{
		return !name.compare(tok.name) && !val.compare(tok.val) && isTerminal == tok.isTerminal;
	}

	bool operator != (const _token& tok) const{
		return name.compare(tok.name) || val.compare(tok.val) || isTerminal != tok.isTerminal;
	}
} TOKEN;

inline ostream& operator<<(ostream& os, const _token& tok){
	return os << tok.name << (tok.isTerminal? "<T>" : "") << (tok.name.compare(tok.val)? '['+tok.val+']' : "");
}

inline ostream& operator<<(ostream& os, const list<TOKEN>& toks){
	list<TOKEN> tok = toks;
	for(list<TOKEN>::iterator it = tok.begin() ; it != tok.end() ; it++)
		os << ' ' << *it;
	return os;
}

class lexAnalyzer{
	private:
	static const string keywords[];
	static const char specialSymbols[];
	static const string operators[];
	string code;
	string toks;
	list<TOKEN*> tokens;
	bool valid;
	
	public:
	lexAnalyzer(string);
	lexAnalyzer(istream&);
	void init();
	static bool isWhiteSpace(char);
	static bool isKeyword(string);
	static bool isOperator(string);
	static bool isInt(string);
	static bool isFloat(string);
	static bool isChar(string);
	static bool isString(string);
	static bool isSpecialSymbol(char);
	static bool isSpecialSymbol(string);
	static bool isIdentifier(string);
	bool isValid();
	static string checkType(string);
	string dumpTokens();
	list<TOKEN*> getTokenList();
	void symbolTable(ostream&);
};