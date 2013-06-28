#include <iostream>
#include <stack>
#include "semanticAnalyzer.hpp"

semanticAnalyzer::semanticAnalyzer(NODE* rt){
	valid = true;
	root = rt;
	if(root->childs.size())
		for(unsigned int i = 0 ; i < root->childs.size() ; i++)
			init(root->childs[i]);
}

semanticAnalyzer::semanticAnalyzer(SLRparser* slr){
	valid = true;
	root = slr->parseTree2();
	if(root->childs.size())
		for(unsigned int i = 0 ; i < root->childs.size() ; i++)
			init(root->childs[i]);
}

void semanticAnalyzer::init(NODE* nod){
	if(!nod->tok->name.compare("VarDeclaration")){
		SYMBOL sym = {"",NONE,false,NULL};
		for(unsigned int ii = 0 ; ii < nod->childs.size() ; ii++)
			if(!nod->childs[ii]->tok->name.compare("TypeSpecifier"))
				if(nod->childs[ii]->childs.size() == 1)
					if(!nod->childs[ii]->childs[0]->tok->name.compare("int"))
						sym.T = INT;
					else if(!nod->childs[ii]->childs[0]->tok->name.compare("float"))
						sym.T = FLOAT;
					else if(!nod->childs[ii]->childs[0]->tok->name.compare("boolean"))
						sym.T = BOOLEAN;
					else if(!nod->childs[ii]->childs[0]->tok->name.compare("byte"))
						sym.T = BYTE;
					else if(!nod->childs[ii]->childs[0]->tok->name.compare("short"))
						sym.T = SHORT;
					else if(!nod->childs[ii]->childs[0]->tok->name.compare("long"))
						sym.T = LONG;
					else if(!nod->childs[ii]->childs[0]->tok->name.compare("double"))
						sym.T = DOUBLE;
					else if(!nod->childs[ii]->childs[0]->tok->name.compare("char"))
						sym.T = CHAR;
					else if(!nod->childs[ii]->childs[0]->tok->name.compare("string"))
						sym.T = STRING;
					else{
						cerr << "Unrecognized type:" << nod->childs[ii]->childs[0]->tok->name << endl;
						valid = false;
						return;
					}
				else{
					cerr << "TypeSpecifier has more than one type!!!" << endl;
					valid = false;
					return;
				}
			else if(!nod->childs[ii]->tok->name.compare("Identifier"))
				sym.name = nod->childs[ii]->tok->val;
		if(!sym.name.length() || sym.T == NONE){
			cerr << "Invalid variable declaration!!!" << endl;
			valid = false;
			return;
		}
		for(NODE* scope = nod ; scope != NULL ; scope = scope->parent)
			if(scope->childs[0]->tok->isTerminal && scope->childs[0]->tok->name.length() == 1 && scope->childs[0]->tok->name[0] == '{'){
				sym.scope = scope;
				symbolTable.push_back(sym);
				break;
			}
	}else if(!nod->tok->name.compare("Expression")){
		if(nod->childs.size() == 1 && !nod->childs[0]->tok->name.compare("SimpleExpression"))
			init(nod->childs[0]);
		else if(nod->childs.size() == 3 && !nod->childs[0]->tok->name.compare("Identifier") && !nod->childs[1]->tok->name.compare("=") && !nod->childs[2]->tok->name.compare("Expression") && nod->childs[0]->tok->isTerminal && nod->childs[1]->tok->isTerminal){
			SYMBOL* sp = NULL;
			for(unsigned int s = 0 ; s < symbolTable.size() && sp == NULL ; s++)
				if(!symbolTable[s].name.compare(nod->childs[0]->tok->val) && isDescendant(nod->childs[0],symbolTable[s].scope))
					sp = &symbolTable[s];
			if(sp == NULL){
				cerr << "Variable " << nod->childs[0]->tok->val << " not found!" << endl;
				valid = false;
				return;
			}
			sp->initialized = true;
			init(nod->childs[2]);
			temp.push_back((QUADRUPLE){nod->childs[1]->tok->name,temp.size(),0,sp});
		}else{
			cerr << "Invalid Expression!" << endl;
			valid = false;
			return;
		}
	}else if(!nod->tok->name.compare("SimpleExpression")){
		if(nod->childs.size() == 1 && !nod->childs[0]->tok->name.compare("AdditiveExpression")){
			init(nod->childs[0]);
		}else if(nod->childs.size() == 3 && !nod->childs[0]->tok->name.compare("AdditiveExpression") && !nod->childs[1]->tok->name.compare("Relop") && !nod->childs[2]->tok->name.compare("AdditiveExpression") && nod->childs[1]->childs.size() == 1 && nod->childs[1]->childs[0]->tok->isTerminal){
			init(nod->childs[0]);
			unsigned int arg1 = temp.size();
			init(nod->childs[2]);
			temp.push_back((QUADRUPLE){nod->childs[1]->childs[0]->tok->name,arg1,temp.size(),NULL});
		}else{
			cerr << "Invalid Simple Expression!" << endl;
			valid = false;
			return;
		}
	}else if(!nod->tok->name.compare("AdditiveExpression")){
		if(nod->childs.size() == 1 && !nod->childs[0]->tok->name.compare("Term"))
			init(nod->childs[0]);
		else if(nod->childs.size() == 3 && !nod->childs[0]->tok->name.compare("AdditiveExpression") && !nod->childs[1]->tok->name.compare("Addop") && !nod->childs[2]->tok->name.compare("Term") && nod->childs[1]->childs.size() == 1 && nod->childs[1]->childs[0]->tok->isTerminal){
			init(nod->childs[0]);
			unsigned int arg1 = temp.size();
			init(nod->childs[2]);
			temp.push_back((QUADRUPLE){nod->childs[1]->childs[0]->tok->name,arg1,temp.size(),NULL});
		}else{
			cerr << "Invalid Additive Expression!" << endl;
			valid = false;
			return;
		}
	}else if(!nod->tok->name.compare("Term")){
		if(nod->childs.size() == 1 && !nod->childs[0]->tok->name.compare("Factor"))
			init(nod->childs[0]);
		else if(nod->childs.size() == 3 && !nod->childs[0]->tok->name.compare("Term") && !nod->childs[1]->tok->name.compare("Mulop") && !nod->childs[2]->tok->name.compare("Factor") && nod->childs[1]->childs.size() == 1 && nod->childs[1]->childs[0]->tok->isTerminal){
			init(nod->childs[0]);
			unsigned int arg1 = temp.size();
			init(nod->childs[2]);
			temp.push_back((QUADRUPLE){nod->childs[1]->childs[0]->tok->name,arg1,temp.size(),NULL});
		}else{
			cerr << "Invalid Term!" << endl;
			valid = false;
			return;
		}
	}else if(!nod->tok->name.compare("Factor")){
		if(nod->childs.size() == 3 && nod->childs[0]->tok->isTerminal && nod->childs[0]->tok->name.length() == 1 && nod->childs[0]->tok->name[0] == '(' && nod->childs[2]->tok->isTerminal && nod->childs[2]->tok->name.length() == 1 && nod->childs[2]->tok->name[0] == ')' && !nod->childs[1]->tok->name.compare("Expression"))
			init(nod->childs[1]);
		else if(nod->childs.size() == 1 && nod->childs[0]->tok->isTerminal && !nod->childs[0]->tok->name.compare("Identifier")){
			SYMBOL* sp = NULL;
			for(unsigned int s = 0 ; s < symbolTable.size() && sp == NULL ; s++)
				if(!symbolTable[s].name.compare(nod->childs[0]->tok->val) && isDescendant(nod->childs[0],symbolTable[s].scope))
					sp = &symbolTable[s];
			if(sp == NULL){
				cerr << "Variable " << nod->childs[0]->tok->val << " not found!" << endl;
				valid = false;
				return;
			}
			if(!sp->initialized){
				cerr << "Variable " << nod->childs[0]->tok->val << " has not initialized!" << endl;
				valid = false;
				return;
			}
			temp.push_back((QUADRUPLE){"=",0,0,sp});
		}else if(nod->childs.size() == 1 && nod->childs[0]->tok->isTerminal && !nod->childs[0]->tok->name.compare("Int"))
			temp.push_back((QUADRUPLE){"=",0,0,new SYMBOL((SYMBOL){nod->childs[0]->tok->val,INT,true,NULL})});
		else{
			cerr << "Invalid Factor!" << endl;
			valid = false;
			return;
		}
	}else if(!nod->tok->name.compare("Statement")){
		if(nod->childs.size() == 1)
			init(nod->childs[0]);
		else if((nod->childs.size() == 5 || nod->childs.size() == 7) && !nod->childs[0]->tok->name.compare("if") && nod->childs[1]->tok->name.length() == 1 && nod->childs[1]->tok->name[0] == '(' && nod->childs[1]->tok->isTerminal && !nod->childs[2]->tok->name.compare("SimpleExpression") && nod->childs[3]->tok->name.length() == 1 && nod->childs[3]->tok->name[0] == ')' && nod->childs[3]->tok->isTerminal && !nod->childs[4]->tok->name.compare("Statement")){
			unsigned int n = temp.size();
			init(nod->childs[2]);
			unsigned int m = temp.size();
			init(nod->childs[4]);
			//cout << "n: " << n << "\tm: " << m << "\tf: " << temp.size() << endl;
			ifstack.push(temp.size()-m);
			if(nod->childs.size() == 5)
				elsestack.push(0);
			else{
				unsigned int f = temp.size();
				init(nod->childs[6]);
				elsestack.push(temp.size()-f);
			}
		}else{
			cerr << "Invalid Statement!" << endl;
			valid = false;
			return;
		}
	}else	for(unsigned int i = 0 ; i < nod->childs.size() ; i++)
		init(nod->childs[i]);
}

bool semanticAnalyzer::isDescendant(NODE* potential, NODE* root){
	for(NODE* p = potential ; p != NULL ; p = p->parent)
		if(p == root)
			return true;
	return false;
}

void semanticAnalyzer::printQuadruples(ostream& out){
	if(!valid){
		cerr << "Invalid semantic! Unable to generate quadruples!" << endl;
		return;
	}
	for(unsigned int i = 0 ; i < temp.size() ; i++){
		if(!temp[i].arg1 && !temp[i].arg2)
			out << temp[i].op << '\t' << temp[i].sym->name << "\t\tt" << i << endl;
		else if(temp[i].sym == NULL && temp[i].arg2)
			out << temp[i].op << "\tt" << temp[i].arg1-1 << "\tt" << temp[i].arg2-1 << "\tt" << i << endl;
		else if(temp[i].sym == NULL)
			out << temp[i].op << "\tt" << temp[i].arg1-1 << "\t\tt" << i << endl;
		else if(temp[i].arg2)
			out << temp[i].op << "\tt" << temp[i].arg1-1 << "\tt" << temp[i].arg2-1 << "\t" << temp[i].sym->name << endl;
		else	out << temp[i].op << "\tt" << temp[i].arg1-1 << "\t\t" << temp[i].sym->name << endl;
	}
}

void semanticAnalyzer::printTM(ostream& out){
	unsigned char reg = 0;
	unsigned int line = 0;
	stack<int> regadds;
	for(unsigned int i = 0 ; i < temp.size() ; i++){
		if(!temp[i].arg1 && !temp[i].arg2){
			regadds.push(getFreeReg(&reg));
			if(temp[i].sym->scope == NULL){
				out << line++ << ": LDC " << regadds.top() << ',' << temp[i].sym->name << ",0" << endl;
			}else for(unsigned int s = 0 ; s < symbolTable.size() ; s++)
				if(!symbolTable[s].name.compare(temp[i].sym->name) && isDescendant(temp[i].sym->scope,symbolTable[s].scope)){
					out << line++ << ": LD " << regadds.top() << ',' << s << "(0)" << endl;
					break;
				}
		}else if(temp[i].sym == NULL && temp[i].arg2)
			if(temp[i].op.length() == 1 && (temp[i].op[0] == '+' || temp[i].op[0] == '-' || temp[i].op[0] == '*' || temp[i].op[0] == '/')){
				int arg2 = regadds.top();
				regadds.pop();
				int arg1 = regadds.top();
				regadds.pop();
				out << line++ << ": " << (temp[i].op[0]=='+'? "ADD" : temp[i].op[0]=='-'? "SUB" : temp[i].op[0]=='*'? "MUL" : temp[i].op[0]=='/'? "DIV" : "") << ' ' << arg1  << ',' << arg1 << ',' << arg2 << endl;
				regadds.push(arg1);
				reg &= ~(1 << arg2-1);
			}else if((temp[i].op.length() == 1  || (temp[i].op.length() == 2 && temp[i].op[1] == '=' )) && (temp[i].op[0] == '<' || temp[i].op[0] == '>' || ((temp[i].op[0] == '!' || temp[i].op[0] == '=') && temp[i].op.length() == 2))){
				int arg2 = regadds.top();
				regadds.pop();
				int arg1 = regadds.top();
				regadds.pop();
				out << line++ << ": " << "SUB " << arg1  << ',' << arg1 << ',' << arg2 << endl;
				//cout << "doing " << arg1 << temp[i].op << arg2 << " on following " << ifstack.front() << endl;
				if(temp[i].op.length() == 1 && temp[i].op[0] == '<')
					out << line++ << ": JGE ";
				else if(temp[i].op.length() == 1 && temp[i].op[0] == '>')
					out << line++ << ": JLE ";
				else if(temp[i].op.length() == 2 && temp[i].op[0] == '<' && temp[i].op[1] == '=')
					out << line++ << ": JGT ";
				else if(temp[i].op.length() == 2 && temp[i].op[0] == '>' && temp[i].op[1] == '=')
					out << line++ << ": JLT ";
				else if(temp[i].op.length() == 2 && temp[i].op[0] == '=' && temp[i].op[1] == '=')
					out << line++ << ": JNE ";
				else if(temp[i].op.length() == 2 && temp[i].op[0] == '!' && temp[i].op[1] == '=')
					out << line++ << ": JEQ ";
				else{
					cerr << "Invalid condition! " << temp[i].op[0] << temp[i].op[1] << temp[i].op.length() << endl;
					return;
				}
			}else{
				cerr << "Unrecognized operator (" << temp[i].op << ")! Abort!" << endl;
				return;
			}
		else if(temp[i].sym == NULL){
			cerr << "Invalid action!" << endl;
			return;
		}else if(temp[i].arg2){
			cerr << "Invalid action!" << endl;
			return;
		}else for(unsigned int s = 0 ; s < symbolTable.size() ; s++)
			if(!symbolTable[s].name.compare(temp[i].sym->name) && isDescendant(temp[i].sym->scope,symbolTable[s].scope)){
				out << line++ << ": ST " << regadds.top() << ','  << s << "(0)" << endl;
				if(i+1 == temp.size())
					out << line++ << ": OUT " <<  regadds.top() << ",0,0" << endl;
				regadds.pop();
				reg = 0;
			}
	}
	out << line++ << ": HALT 1,0,0" << endl;
}

char semanticAnalyzer::getFreeReg(unsigned char* reg){
	for(unsigned char a = 0 ; a < 8 ; a++)
		if(!(*reg & (1<<a))){
			*reg |= 1 << a;
			return a+1;
		}
	return -1;
}