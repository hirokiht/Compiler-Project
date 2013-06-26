#include <iostream>
#include <string>
#include <list>
#include <map>
#include "parserGenerator.hpp"

using namespace std;

parserGenerator::parserGenerator (istream& in){		//constructor for reading input stream into object
	in.seekg(0);				//rewind just in case
	in.clear();				//clear any errors that may occur previously
	in.sync();				//sync inputs as a precaution
	while(in.good()){
		string p = "";
		for(char ch = in.get() ; in.good() ; ch = in.get())	//getline like
			if(ch == '\n')
				break;
			else if(ch != '\r')
				p += ch;
		add(p);
	}
	init();
}

parserGenerator::parserGenerator (string in){		//constructor for reading input stream into object
	while(!in.empty()){
		if(in.find('\n') == string::npos){
			add(in);
			break;
		}
		add(in.substr(0,in.find('\n')));
		in = in.substr(in.find('\n')+1);
	}
	init();
}

bool parserGenerator::add(string p){
	TOKEN head = (p.find(' ') == string::npos)? (TOKEN){p,p,false} : (TOKEN){p.substr(0,p.find(' ')),p.substr(0,p.find(' ')),false};
	list<TOKEN> body;
	if(p.find(' ') != string::npos && !p.substr(p.find(' ')+1).empty())
		for(string str = p.substr(p.find(' ')+1) ; !str.empty() ; str = (str.find(' ') == string::npos)? "" : str.substr(str.find(' ')+1)){
			string token = (str.find(' ') == string::npos)? str : str.substr(0,str.find(' '));
			//bool terminal = isTerminal(token) && token.compare(p.substr(0,p.find(' ')));	//must not equal to head if terminal
			body.push_back((TOKEN){token,token,isTerminal(token)});// .isTerminal = terminal});	//substituted due to post-processing is handling it
		}
	production.push_back((PRODUCTION){head,body});
	for(list<PRODUCTION>::iterator it = production.begin() ; it != production.end() ; it++)		//post-processing for non-terminalize any previous body that has this head token
		for(list<TOKEN>::iterator itt = it->body.begin() ; itt != it->body.end() ; itt++)
			if(itt->isTerminal && itt->name == head.name)
				itt->isTerminal = false;
	return !p.empty();
}

bool parserGenerator::isTerminal(TOKEN tok){
	return isTerminal(tok.name);
}

bool parserGenerator::isTerminal(string head){
	for(list<PRODUCTION>::iterator it = production.begin() ; it != production.end() ; it++)
		if(!it->head.name.compare(head))
			return false;
	return true;
}

list<TOKEN> parserGenerator::getNonTerminals(){
	list<TOKEN> result;
	for(list<PRODUCTION>::iterator it = production.begin() ; it != production.end() ; it++){
		bool exist = false;
		for(list<TOKEN>::iterator itt = result.begin() ; itt != result.end() ; itt++)
			if(*itt == it->head){
				exist = true;
				break;
			}
		if(!exist)
			result.push_back(it->head);
	}
	return result;
}

list<PRODUCTION> parserGenerator::getProductions(string head){
	return getProductions((char *)head.c_str());
}

list<PRODUCTION> parserGenerator::getProductions(char * head){
	list<PRODUCTION> result;
	for(list<PRODUCTION>::iterator it = production.begin() ; it != production.end() ; it++)
		if(!it->head.name.compare(head))
			result.push_back(*it);
	return result;
}

bool parserGenerator::nullable(PRODUCTION prod){
	//cout << "processing production: " << prod << endl;
	if(prod.body.empty())
		return true;
	if(prod.body.size() == 1 && prod.body.front().name == prod.head.name){
		cerr << "Invalid grammar, having same head and body in production! " << prod << endl;
		return false;
	}
	for(list<TOKEN>::iterator it = prod.body.begin() ; it != prod.body.end() ; it++)
		if(it->name != prod.head.name && !nullable(*it))	//AND relation, if one of the tokens in body is not nullable the production is not nullable
			return false;
	return true;
}

bool parserGenerator::nullable(TOKEN tok){
	//cout << "processing token: " << tok << endl;
	if(tok.isTerminal)
		return false;
	list<PRODUCTION> prods = getProductions(tok.name);
	for(list<PRODUCTION>::iterator it = prods.begin() ; it != prods.end() ; it++)
		if(nullable(*it))		//OR relation, if one of the productions is nullable, then the production is nullable
			return true;
	return false;
}

list<TOKEN> parserGenerator::first(PRODUCTION prod){
	list<TOKEN> result;
	//cout << "processing production: " << prod << endl;
	if(prod.body.empty())
		return result;
	if(prod.body.size() == 1 && prod.body.front().name == prod.head.name){
		cerr << "Invalid grammar, having same head and body in production! " << prod << endl;
		return result;
	}
	for(list<TOKEN>::iterator it = prod.body.begin() ; it != prod.body.end() ; it++)
		if(it->name == prod.head.name && !nullable(*it))
			return result;
		else if(it->name == prod.head.name)
			continue;
		else if(!nullable(*it)){
			list<TOKEN> toks = first(*it);
			result.splice(result.end(), toks);
			return result;
		}else{
			list<TOKEN> toks = first(*it);
			result.splice(result.end(), toks);
		}
	return result;	//if all nullable
}

list<TOKEN> parserGenerator::first(TOKEN tok){
	list<TOKEN> result;
	if(isTerminal(tok)){
		result.push_back(tok);
		return result;
	}
	list<PRODUCTION> prods = getProductions(tok.name);
	for(list<PRODUCTION>::iterator it = prods.begin() ; it != prods.end() ; it++){
		list<TOKEN> toks = first(*it);
		result.splice(result.end(),toks);
	}
	result.sort();
	result.unique();
	return result;
}

void parserGenerator::insertFollow(TOKEN to, TOKEN from){
	bool exists = false;
	for(multimap<TOKEN,TOKEN>::iterator it = followTable.equal_range(to).first ; it != followTable.equal_range(to).second && !exists ; it++)
		if(it->second == from)
			exists = true;
	if(!exists)
		followTable.insert(pair<TOKEN,TOKEN>(to,from));
	if(follower.find(to) != follower.end())
		for(multimap<TOKEN,TOKEN>::iterator it = follower.equal_range(to).first ; it != follower.equal_range(to).second ; it++)
			insertFollow(it->second,from);
}

void parserGenerator::follow(PRODUCTION prod){	//does the heavy lifting
	for(list<TOKEN>::iterator it = prod.body.begin() ; it != --prod.body.end() ; it++){	//process the first belong to follow stuff
		list<TOKEN>::iterator nxt = it;
		list<TOKEN> nxtfirst = first(*++nxt);
		while(nullable(*nxt)){
			list<TOKEN> nxtnxtfirst = first(*++nxt);
			nxtfirst.splice(nxtfirst.begin(),nxtnxtfirst);
		}
		for(list<TOKEN>::iterator itt = nxtfirst.begin() ; itt != nxtfirst.end() ; itt++){
			bool exists = false;							//check if not exists previously then insert it
			for(multimap<TOKEN,TOKEN>::iterator ittt = followTable.equal_range(*it).first ; ittt != followTable.equal_range(*it).second && !exists ; ittt++)
				if(ittt->second == *itt)
					exists = true;
			if(!exists && !isTerminal(*it))
				insertFollow(*it,*itt);
		}
	}
}

void parserGenerator::follow(TOKEN tok){		//mainly for tracking production uses
	if(tok.isTerminal)
		return;
	list<PRODUCTION> prods = getProductions(tok.name);
	for(list<PRODUCTION>::iterator it = prods.begin() ; it != prods.end() ; it++)
		follow(*it);
}

void parserGenerator::output(ostream& out){
	list<TOKEN> nT = getNonTerminals();
	unsigned int maxlength = nT.front().name.length();
	for(list<TOKEN>::iterator it = ++nT.begin() ; it != nT.end() ; it++)
		if(it->name.length() > maxlength)
			maxlength = it->name.length();
	out << "nullable" << endl;
	for(list<TOKEN>::iterator it = nT.begin() ; it != nT.end() ; it++){
		out << it->name;
		for(unsigned int i = 0 ; i < (maxlength-it->name.length())/8.0+0.5 ; i++)
			out << '\t';
		out << (nullableTable.find(*it)->second? "true" : "false") << endl;
	}
	out << endl << "first" << endl;
	for(list<TOKEN>::iterator it = nT.begin() ; it != nT.end() ; it++){
		out << it->name;
		for(unsigned int i = 0 ; i < (maxlength-it->name.length())/8.0-0.5 ; i++)
			out << '\t';
		for(multimap<TOKEN,TOKEN>::iterator itt = firstTable.equal_range(*it).first ; itt != firstTable.equal_range(*it).second ; itt++)
			out << '\t' << itt->second.name << (itt->second.name.length() > 8 ? "" : "\t");
		out << endl;
	}
	out << endl << "follow" << endl;
	for(list<TOKEN>::iterator it = nT.begin() ; it != nT.end() ; it++){
		out << it->name;
		for(unsigned int i = 0 ; i < (maxlength-it->name.length())/8.0-0.5 ; i++)
			out << '\t';
		for(multimap<TOKEN,TOKEN>::iterator itt = followTable.equal_range(*it).first ; itt != followTable.equal_range(*it).second ; itt++)
			out << '\t' << itt->second.name << (itt->second.name.length() > 8 ? "" : "\t");
		out << endl;
	}
}

void parserGenerator::init(){
	list<TOKEN> nT = getNonTerminals();
	list<PRODUCTION> prods = getProductions(nT.front().name);
	if(prods.size() != 1){
		cerr << "Invalid Start token, more than 1 productions with same token detected!" << endl;
		return;
	}
	if(prods.front().body.size() != 1){
		cerr << "Invalid Start token, production contain more than 1 start token!" << endl;
		return;
	}
	for(list<TOKEN>::iterator it = nT.begin() ; it != nT.end() ; it++){	//follower
		prods = getProductions(it->name);
		for(list<PRODUCTION>::iterator itt = prods.begin() ; itt != prods.end() ; itt++){
			if(itt->body.size() && !isTerminal(itt->body.back()) && itt->body.back() != itt->head){
				bool exists = false;
				for(multimap<TOKEN,TOKEN>::iterator ittt = follower.equal_range(itt->head).first ; ittt != follower.equal_range(itt->head).second && !exists; ittt++)
					if(ittt->second == itt->body.back())
						exists = true;
				if(!exists)
					follower.insert(pair<TOKEN,TOKEN>(itt->head,itt->body.back()));
			}
		}
	}
	insertFollow(*(++nT.begin()),(TOKEN){"$","$",true});	//insert $ into second production (first is: start production ' )
	for(list<TOKEN>::iterator it = nT.begin() ; it != nT.end() ; it++){
		nullableTable.insert(pair<TOKEN,bool>(*it,nullable(*it)));
		list<TOKEN> toks = first(*it);
		for(list<TOKEN>::iterator itt = toks.begin() ; itt != toks.end() ; itt++)
			firstTable.insert(pair<TOKEN,TOKEN>(*it,*itt));
		follow(*it);
	}
	unsigned int i = 0;
	for(list<PRODUCTION>::iterator it = production.begin() ; it != production.end() ; it++){		//leading states "production" generated
		STATE* state = new STATE;
		state->label = i++;
		state->accept = it->body.size()? false : true;
		STATE* curr = state;
	//	cout << "NFA: " << *it << " is " << (state->accept? "accept" : "reject") << endl;
		for(list<TOKEN>::iterator itt = it->body.begin() ; itt != it->body.end() ; itt++){
			curr->transitionS.insert(pair<TOKEN,STATE*>(*itt,new STATE));
			(--curr->transitionS.end())->second->label = i++;
			(--curr->transitionS.end())->second->accept = itt == --it->body.end()? true : false;
			curr = (--curr->transitionS.end())->second;
		}
		prodstate.insert(pair<PRODUCTION,STATE>(*it,*state));
	}
	for(multimap<PRODUCTION,STATE>::iterator it = prodstate.begin() ; it != prodstate.end() ; it++){		//leading states "production" generated
		states.insert(pair<unsigned int,STATE*>(it->second.label,&it->second));
		for(STATE* curr = &it->second ; curr->transitionS.size() ; curr = curr->transitionS.begin()->second)	//traces the leading states
			for(multimap<TOKEN,STATE*>::iterator itt = curr->transitionS.begin() ; itt != curr->transitionS.end() ; itt++)	//traces the production in the state
				states.insert(pair<unsigned int,STATE*>(itt->second->label,itt->second));	//insert the nfas into "states" for tracking purpose
	}
	/*for(multimap<PRODUCTION,STATE>::iterator it = prodstate.begin() ; it != prodstate.end() ; it++){		//leading states "production" generated
		cout << it->first << endl << "(" << it->second.label << ") => " << endl;
		for(STATE* curr = &it->second ; curr->transitionS.size() ; curr = curr->transitionS.begin()->second)	//traces the leading states
			for(multimap<TOKEN,STATE*>::iterator itt = curr->transitionS.begin() ; itt != curr->transitionS.end() ; itt++)
				cout << "\t\t" << itt->first << "(" << curr->label << ") => " << itt->second->label << endl;
		if(!it->second.transitionS.size())
			cout << "\t\t(empty)" << endl;
		cout << endl;
	}*/
	for(multimap<PRODUCTION,STATE>::iterator it = prodstate.begin() ; it != prodstate.end() ; it++)		//leading states "production" generated
		for(STATE* curr = &it->second ; curr->transitionS.size() ; curr = (--curr->transitionS.end())->second){	//traces the leading states????
			for(multimap<TOKEN,STATE*>::iterator itt = curr->transitionS.begin() ; itt != curr->transitionS.end() ; itt++)
				if(!isTerminal(itt->first))
					for(multimap<PRODUCTION,STATE>::iterator ittt = prodstate.begin() ; ittt != prodstate.end() ; ittt++)
						if(itt->first == ittt->first.head)
							curr->transitionS.insert(pair<TOKEN,STATE*>((TOKEN){"","",true},&ittt->second));
			if(curr->transitionS.begin()->second == NULL)
				break;
		}
	list<STATE*> s0_epsilon;
	for(unsigned int l = 0 ; l < i ; l++){
		list<STATE*> encs = get_enclosure(states.find(l)->second);
		if(l == 0)
			s0_epsilon = encs;
		for(list<STATE*>::iterator it = encs.begin() ; it != encs.end() ; it++)
			enclosure.insert(pair<unsigned int,STATE*>(l,*it));
	}
	/*for(unsigned int l = 0 ; l < i ; l++){
		cout << "enclosure(" << l << ") = {" ;
		for(multimap<unsigned int,STATE*>::iterator it = enclosure.equal_range(l).first ; it != enclosure.equal_range(l).second ; it++)
			cout << "\t" << it->second->label;
		cout << "\t}" << endl;
	}*/
	STATE* dfa_start = insertDFA(s0_epsilon);
}

list<STATE*> parserGenerator::get_enclosure(STATE* state){
	list<STATE*> result;
	return get_enclosure(result,state);
}

list<STATE*> parserGenerator::get_enclosure(list<STATE*> result, STATE* state){
	result.push_back(state);
	for(multimap<TOKEN,STATE*>::iterator it = state->transitionS.begin() ; it != state->transitionS.end() ; it++)
		if(it->first == (TOKEN){"","",true}){
			bool exists = false;
			for(list<STATE*>::iterator itt = result.begin() ; itt != result.end() && !exists; itt++)
				if((*itt)->label == it->second->label)
					exists = true;
			if(!exists)
				result = get_enclosure(result,it->second);
		}
	return result;
}

STATE* parserGenerator::insertDFA(list<STATE*> nfa_states){
	STATE* newDFA = NULL;
	for(map<STATE*,list<STATE*> >::iterator itt = dfa.begin() ; itt != dfa.end() && newDFA == NULL ; itt++){	//searches if it already exists, if not then continue to create and insert it
		bool valid = false;
		for(list<STATE*>::iterator it = nfa_states.begin() ; it != nfa_states.end(); it++){
			bool exists = false;
			for(list<STATE*>::iterator ittt = itt->second.begin() ; ittt != itt->second.end() && !exists ; ittt++)				
				if(*it == *ittt)
					exists = true;
			if(!exists)
				break;
			else if(it == --nfa_states.end())
				valid = true;
		}
		if(valid)
			newDFA = itt->first;
	}
	if(newDFA != NULL)
		return newDFA;
	else	newDFA = new STATE;
	newDFA->label = dfa.size();
	newDFA->accept = false;
/*	for(list<STATE*>::iterator it = nfa_states.begin() ; it != nfa_states.end() ; it++){
		cout << (*it)->label << ((*it)->accept? "accept" : "reject") << ":" << endl;
		for(multimap<TOKEN,STATE*>::iterator itt = (*it)->transitionS.begin() ; itt != (*it)->transitionS.end() ; itt++)
			cout << "\tfrom " << itt->first << " to " << itt->second->label << endl;
	} */
	STATE* accepting = NULL;
	for(list<STATE*>::iterator it = nfa_states.begin() ; it != nfa_states.end() && !newDFA->accept; it++)
		if((*it)->accept){
			newDFA->accept = true;
			accepting = *it;
		}
	dfa.insert(pair<STATE*,list<STATE*> >(newDFA,nfa_states));
	/*cout << "inserting " << newDFA->label << " (" <<( newDFA->accept? "accept" : "reject" )<< ") with ";
	for(list<STATE*>::iterator it = nfa_states.begin() ; it != nfa_states.end() ; it++)
		cout << '\t' << (*it)->label;
	cout << endl;*/
	//start processing transitionS for newDFA***************************
	multimap<TOKEN,STATE*> tmp;	//record next going nfa state label
	list<TOKEN> keys;		//keys for tmp
	for(list<STATE*>::iterator it = nfa_states.begin() ; it != nfa_states.end() ; it++)
		for(multimap<TOKEN,STATE*>::iterator itt = (*it)->transitionS.begin() ; itt != (*it)->transitionS.end() ; itt++)
			if(itt->first != (TOKEN){"","",true}){
				tmp.insert(pair<TOKEN,STATE*>(itt->first,itt->second));
				bool exists = false;
				for(list<TOKEN>::iterator ittt = keys.begin() ; ittt != keys.end() && !exists; ittt++)
					if(*ittt == itt->first)
						exists = true;
				if(!exists)
					keys.push_back(itt->first);
			}
	for(list<TOKEN>::iterator it = keys.begin() ; it != keys.end() ; it++){
		list<STATE*> new_nfa_states;
		for(multimap<TOKEN,STATE*>::iterator itt = tmp.equal_range(*it).first ; itt != tmp.equal_range(*it).second ; itt++)
			new_nfa_states.push_back(itt->second);
		for(list<STATE*>::iterator itt = new_nfa_states.begin() ; itt != new_nfa_states.end() ; itt++)		//expand the nfa states with epsilon enclosure
			for(multimap<unsigned int,STATE*>::iterator ittt = enclosure.equal_range((*itt)->label).first ; ittt != enclosure.equal_range((*itt)->label).second ; ittt++)
				if(ittt->second != *itt)
					new_nfa_states.push_back(ittt->second);
		new_nfa_states.sort();				//sort based on memory address (pointer)
		new_nfa_states.unique();			//remove duplicates
		STATE* new_dfa_state = insertDFA(new_nfa_states);
		newDFA->transitionS.insert(pair<TOKEN,STATE*>(*it,new_dfa_state));
	//	cout << "inserting transition in " << newDFA->label << " from " << *it << " to (" << new_dfa_state->label << ")" << endl;
	}
	//end processing transitionS for newDFA****************************
	//start processing states to reduce to*****************************
	if(accepting == NULL)
		return newDFA;	//skip processing if no accepting states are found in nfa_states
	const PRODUCTION* prod = NULL;
	/*for(multimap<PRODUCTION,STATE>::iterator it = prodstate.begin() ; it != prodstate.end() && prod == NULL ; it++)
		for(STATE* curr = &it->second ; curr->transitionS.size() && prod == NULL ; curr = (--curr->transitionS.end())->second)
			for(multimap<TOKEN,STATE*>::iterator itt = curr->transitionS.begin() ; itt != curr->transitionS.end() ; itt++)
				cout << "Comparing " << itt->second->label << " with " << accepting->label << " from " << it->first << endl; */
	for(multimap<PRODUCTION,STATE>::iterator it = prodstate.begin() ; it != prodstate.end() && prod == NULL ; it++)
		for(STATE* curr = &it->second ;  prod == NULL ; curr = (--curr->transitionS.end())->second){
			if(curr == accepting)
				prod = &it->first;
			if(!curr->transitionS.size())
				break;
		}
	if(!followTable.count(prod->head)){
		REDUCE acpt;	//dummy reduce to represent accept
		acpt.n = (TOKEN){"","",true};
		acpt.r = 0;
		newDFA->transitionR.insert(pair<TOKEN,REDUCE>((TOKEN){"$","$",true},acpt));
	}
	for(multimap<TOKEN,TOKEN>::iterator it = followTable.equal_range(prod->head).first ; it != followTable.equal_range(prod->head).second ; it++)
		newDFA->transitionR.insert(pair<TOKEN,REDUCE>(it->second,(REDUCE){prod->head,prod->body,prod->body.size()}));
	/*for(multimap<TOKEN,TOKEN>::iterator it = followTable.equal_range(prod->head).first ; it != followTable.equal_range(prod->head).second ; it++)
		cout << "Inserting " << *prod << " into " << newDFA->label << ',' << it->second << " accepting is" << accepting->label << endl;*/
	//end processing states to reduce to*****************************
	return newDFA;
}

map<STATE*,list<STATE*> > parserGenerator::getDFA(){
	return dfa;
}

map<unsigned int,STATE*> parserGenerator::getParseTable(){
	map<unsigned int,STATE*> parsetable;
	for(map<STATE*,list<STATE*> >::iterator it = dfa.begin() ; it != dfa.end() ; it++)
		parsetable.insert(pair<unsigned int,STATE*>(it->first->label,it->first));
	return parsetable;
}

list<PRODUCTION> parserGenerator::getProductions(){
	return production;
}