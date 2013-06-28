#include <iostream>
#include <string>
#include "lexAnalyzer.hpp"

const string lexAnalyzer::keywords[] = {"abstract","assert","boolean","break","byte","case","continue","enum","for","instanceof","new","return","switch","transient","catch","default","extends","goto","int","package","short","synchronized","try","char","do","final","if","interface","private","static","this","void","class","double","finally","implements","long","protected","strictfp","throw","volatile","const","else","float","import","native","public","super","throws","while"};
const char lexAnalyzer::specialSymbols[] = {'{','}','[',']','(',')',';',','};
const string lexAnalyzer::operators[] = {"++","--","++","--","+","-","~","!","+","-","*","/","%","<<",">>",">>>","&","^","|","<",">","<=",">=","instanceof","==","!=","&&","||","?",":","=","+=","-=","*=","/=","%=","&=","^=","|=","<<=",">>=",">>>="};

lexAnalyzer::lexAnalyzer(string codes){
	for(size_t i = 0 ; i < codes.length() ; i++)							//reading string
		if(codes[i] == '/' && i+1 < codes.length() && (codes[i+1] == '/' || codes[i+1] == '*')){//comment removal
			if(codes[i+1] == '*')
				while(++i < codes.length() && (codes[i-1] != '*' || codes[i] != '/'));
			else	while(++i < codes.length() && codes[i+1] != '\n');
		}else if(codes[i] != '\r')									//convert dos-like into unix-like "txt file"
			code += codes[i];
	init();
}

lexAnalyzer::lexAnalyzer(istream& input){
	input.seekg(0);				//rewind just in case
	input.clear();				//clear any errors that may occur previously
	input.sync();				//sync inputs as a precaution
	for(int ch = input.get() ; input.good() ; ch = input.get())					//reading file
		if(ch == '/' && (input.peek() == '/' || input.peek() == '*'))				//comment removal
			if(input.peek() == '*'){							
				while((input.get() != '*' || input.peek() != '/') && input.good());	
				input.seekg(1,istream::cur);						//to skip last '/'
			}else	while(input.peek() != '\n' && input.good())				//skipping until newline or error
					input.seekg(1,istream::cur);					//skipping comments
		else if(ch != '\r')									//convert dos-like into unix-line "txt file"
			code += ch;
	init();
}

void lexAnalyzer::init(){
	for(size_t i = 0 ; i < code.length() ; i++){
		if((i == 0 || (i > 0 && isWhiteSpace(code[i-1]))) && !isWhiteSpace(code[i])){
			string tok = "";
			for(bool str = false ; i < code.length() && (!isWhiteSpace(code[i]) || (i+1 < code.length() && code[i-1] == '\'' && code[i+1] == '\'') || str) ; i++){
				if(code[i] == '"')
					str = !str;
				tok += code[i];
			}
			string type = checkType(tok);
	//		cout << "Processing token: " << tok << endl;	//debug use only, checking input tokens
			if(!type.compare("Error")){
				cerr << "ERROR: Unrecognized token: '" << tok << "'" << endl;
				valid = false;
				return;
			}else if(!type.compare("Special Symbol") || !type.compare("Keyword") || !type.compare("Operator")){
				toks += tok;
				tokens.push_back(new TOKEN((TOKEN){tok,tok,true}));
			}else{
				toks += type;
				tokens.push_back(new TOKEN((TOKEN){type,tok,true}));
			}
		}
		toks += code[i];	//rest is all white space character of all shapes and sizes
	}
	valid = true;
}

bool lexAnalyzer::isWhiteSpace(char ch){
	return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r')? true : false;
}

bool lexAnalyzer::isKeyword(string str){
	for(unsigned char i = 0 ; i < 50 ; i++)
		if(!str.compare(keywords[i]))
			return true;
	return false;
}

bool lexAnalyzer::isOperator(string str){
	for(unsigned char i = 0 ; i < 42; i++)
		if(!str.compare(operators[i]))
			return true;
	return false;
}

bool lexAnalyzer::isInt(string str){
	if(str.length() > 1){
		if((str[0] < '0' | str[0] > '9') && str[0] != '+' && str[0] != '-' )
			return false;
		for(unsigned int i = 1 ; i < str.length() ; i++)
			if(str[i] < '0' | str[i] > '9')
				return false;
	}else if(str[0] < '0' | str[0] > '9')
		return false;
	return true;
}

bool lexAnalyzer::isFloat(string str){
	if(str.length() < 1)
		return false;
	if(str.find('.') != str.rfind('.'))	//more than one .
		return false;
	if(str.length() > 1){
		if(str.find('.') == str.length()-1)//filter str that ends with .
			return false;
		for(unsigned int i = (str[0] == '+' || str[0] == '-')? 1 : 0 ; i < str.find('.') ; i++)	//process numbers before .
			if(str[i] < '0' || str[i] > '9')
				return false;
		for(unsigned int i = str.find('.')+1 ; i < str.length() ; i++)		//process numbers after .
				if(str[i] < '0' || str[i] > '9')
					return false;
		return true;
	}else if(str[0] >= '0' && str[0] <= '9')
		return true;
	else	return false;
}

bool lexAnalyzer::isChar(string str){
	if(str.length() == 4 && (!str.compare("'\\\''") || !str.compare("'\\\\'")) || !str.compare("'\\\"'"))
		return true;
	else if(str.length() != 3 || str[0] != '\'' || str[2] != '\'')
		return false;
	else	return true;
}

bool lexAnalyzer::isString(string str){
	if(str.length() < 3 || str[0] != '"' || str[str.length()-1] != '"')
		return false;
	else	return true;
}

bool lexAnalyzer::isSpecialSymbol(char c){
	for(unsigned int i = 0 ; i < sizeof(specialSymbols)/sizeof(specialSymbols[0]) ; i++)
		if(specialSymbols[i] == c)
			return true;
	return false;
}

bool lexAnalyzer::isSpecialSymbol(string str){
	if(str.length() != 1)
		return false;
	return isSpecialSymbol(str[0]);
}

bool lexAnalyzer::isIdentifier(string str){
	if(str.length() < 1)
		return false;
	if((str[0] < 'a' || str[0] > 'z') && (str[0] < 'A' || str[0] > 'Z') && str[0] != '$' && str[0] != '_')
		return false;
	for(unsigned int i = 1 ; i < str.length() ; i++)
		if( (str[i] < 'a' || str[i] > 'z') && (str[i] < 'A' || str[i] > 'Z') && (str[i] < '0' || str[i] > '9') && str[i] != '$' && str[i] != '_' && str[i] != '.')
			return false;
	if(str[str.length()-1] == '.')	//if last character is . then false
		return false;
	return true;
}

bool lexAnalyzer::isValid(){
	return valid;
}

string lexAnalyzer::checkType(string str){
	if(str.length() < 1)
		return "Error";
	else if(isInt(str))
		return "Int";
	else if(isSpecialSymbol(str))
		return "Special Symbol";
	else if(isOperator(str))
		return "Operator";
	else if(str.length() == 1)
		return isIdentifier(str)? "Identifier" : "Error";
	else	return isKeyword(str)? "Keyword" : isChar(str)? "Char" : isString(str)? "String" : isFloat(str)? "Float" : isIdentifier(str)? "Identifier" : "Error";
}

string lexAnalyzer::dumpTokens(){
	return toks;
}

list<TOKEN*> lexAnalyzer::getTokenList(){
	return tokens;
}

void lexAnalyzer::symbolTable(ostream& out){
	unsigned int scope = 0, maxIDlen = 0;
	TOKEN* lasttok = *tokens.begin();
	for(list<TOKEN*>::iterator it = tokens.begin() ; it != tokens.end() ; it++)
		if((!(*it)->name.compare("Identifier") || !(*it)->name.compare("id")) && (*it)->val.length() > maxIDlen)
			maxIDlen = (*it)->val.length();
	for(list<TOKEN*>::iterator it = ++tokens.begin() ; it != tokens.end() ; it++){
		if((*it)->name.length() == 1 && (*it)->name[0] == '{')
			scope++;
		if((!(*it)->name.compare("Identifier") || !(*it)->name.compare("id")) && !checkType(lasttok->val).compare("Keyword")){
			out << "id " << (*it)->val << '\t';
			 for(int tabs = maxIDlen-(*it)->val.length() ; tabs > 0 ; tabs -= 8)
				out << '\t';
			 out << lasttok->val << '\t' << scope << endl;
		}
		lasttok = *it;
	}
}