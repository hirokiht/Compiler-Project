#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include <stack>
#include "SLRparser.hpp"

SLRparser::SLRparser(map<unsigned int,STATE*> dfa){
	parseTable = dfa;
	valid = false;
}

SLRparser::SLRparser(map<unsigned int,STATE*>dfa, list<TOKEN*> toks){
	parseTable = dfa;
	tokens = toks;
	valid = parse();
}

bool SLRparser::parse(){
	return parse(tokens);
}

bool SLRparser::parse(list<TOKEN*> tokens){
	//cout << "Total number of tokens: " << tokens.size() << endl;
	stack<STATE*> pool;
	stack<string> vals;
	pool.push(parseTable.find(0)->second);
	for(TOKEN* token = tokens.front() ; pool.size() ; token = tokens.size() ? tokens.front() : new TOKEN((TOKEN){"$","$",true})){
		STATE* cur = pool.top();
	//	cout << "Processing state: " << cur->label << endl;
	//	cout << "Processing token: " << token << endl;
		bool found = false;
		for(map<TOKEN,STATE*>::iterator it = cur->transitionS.begin() ; it != cur->transitionS.end() && !found ; it++)
			if(!it->first.name.compare(token->name)){
				if(!token->name.compare("Identifier") || !token->name.compare("id") || !token->name.compare("Int") || !token->name.compare("Char") || !token->name.compare("Float"))
					vals.push(token->val);
				tokens.pop_front();
	//			cout << "Action taken for " << it->first << ": " << "Go to state" << it->second->label << endl;
	//			cout << "Action taken for " << token << ": " << "Go to state" << it->second->label << endl;
				pool.push(it->second);
				found = true;
			}
		if(found)
			continue;
		for(map<TOKEN,REDUCE>::iterator it = cur->transitionR.begin() ; it != cur->transitionR.end() && !found; it++)
			if(!it->first.name.compare(token->name)){
				for(unsigned int i = 0 ; i < it->second.r ; i++)
					pool.pop();
				cur = pool.top();
				if(it->second.n == (TOKEN){"","",true})
					return true;
				else if(cur->transitionS.find(it->second.n) == cur->transitionS.end()){
					cerr << "Parse Table Error : " << it->second.n << " Not found!!!" << endl;
					return false;
				}
	//			cout << "Action taken for " << it->first << ": " << "Reduce" << it->second.r << "(" << it->second.n << ")" << endl;
	//			cout << "Action taken for " << token << ": " << "Reduce" << it->second.r << "(" << it->second.n << "=>" << it->second.body << ")" << endl;
				for(list<TOKEN>::iterator itt = it->second.body.begin() ; itt != it->second.body.end() ; itt++)
					if(!itt->name.compare("Identifier") || !itt->name.compare("id") || !itt->name.compare("Int") || !itt->name.compare("Char") || !itt->name.compare("Float")){
						itt->val = vals.top();
						vals.pop();
					}
				STATE* nxt = cur->transitionS.find(it->second.n)->second;
				pool.push(nxt);
	//			cout << "pushing " << nxt->label << endl;
				found = true;
				reduce.push_back((PRODUCTION){it->second.n,it->second.body});
	//			cout << reduce.back() << endl;
			}
		if(!found){
			cerr << *token << " not found in " << cur->label << "!" << endl;
			return false;
		}
	}
	return false;
}

string SLRparser::output(){
	ostringstream out;
	for(list<PRODUCTION>::iterator it = reduce.begin() ; it != reduce.end() ; it++){
		out << it->head.name;
		for(list<TOKEN>::iterator itt = it->body.begin() ; itt != it->body.end() ; itt++)
			out << '\t' << itt->name;
		out << endl;
	}
	return out.str();
}

string SLRparser::parseTree(PRODUCTION* start){	// start = NULL
	parseTree2();
	ostringstream out;
	out << (start==NULL? root : new NODE((NODE){NULL,vector<NODE*>(1,root),&start->head,0}));
	return out.str();
}

NODE* SLRparser::parseTree2(){
	if(!valid){
		cerr << "Syntax Incorrect! Unable to produce parse tree!" << endl;
		return NULL;
	}
	list<PRODUCTION*> result;				//store the list of productions
	stack<PRODUCTION*> prods;				//tmp stack for procesing parseTree
	stack<list<TOKEN>::reverse_iterator> rits;		//refer to the production's body
	prods.push(&*reduce.rbegin());
	rits.push(prods.top()->body.rbegin());
	stack<list<PRODUCTION*>::iterator> its;
	its.push(result.begin());
	for(list<PRODUCTION>::reverse_iterator it = reduce.rbegin() ; it != reduce.rend() ; ){
		result.insert(its.top(),prods.top());
		while(prods.size()){
			if(rits.top() == prods.top()->body.rbegin())
				its.push(its.top()--);
			for( ; prods.size() && rits.top() == prods.top()->body.rend() ; its.pop(), rits.pop(), prods.pop());		//finishes a production
			if(prods.size() && rits.top()->isTerminal)
				rits.top()++;
			else if(prods.size()){
				rits.top()++;
				it++;
				prods.push(&*it);
				rits.push(prods.top()->body.rbegin());
				break;
			}else	it++;
		}
	}
	return root = treegen(&result);
}

NODE* SLRparser::treegen(list<PRODUCTION*>* result, NODE* parent, unsigned int level){
	PRODUCTION* prod = result->front();
	result->pop_front();
	NODE* root = new NODE((NODE){parent,vector<NODE*>(),&prod->head,level});
	for(list<TOKEN>::iterator it = prod->body.begin() ; it != prod->body.end() ; it++)
		if(it->isTerminal)
			root->childs.push_back(new NODE((NODE){root,vector<NODE*>(),&*it,level+1}));
		else if(*it == result->front()->head)
			root->childs.push_back(treegen(result,root,level+1));
		else{
			cerr << "parse tree generation error!" << endl;
			return NULL;
		}
	return root;
}

bool SLRparser::isValid(){
	return valid;
}