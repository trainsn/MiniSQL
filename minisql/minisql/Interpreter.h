//
//Interpreter.h
//
//by ÕÅ±Ìë… on 16/6/2
//
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include<string>
#include<vector>
#include"API.H"
using namespace std;

class Interpreter
{
	public:
		API *ap;
		string fileName;
		int interpreter(string s);
		
		string getWord(string s,int*st);
		Interpreter(){};
		~Interpreter(){};		
};

#endif