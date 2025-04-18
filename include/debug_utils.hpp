#pragma once
#include <iostream>
#include <vector>
#include <string>

namespace hut{
	using namespace std;

	template<class  T>
	void printList(const vector<T> &v){
		cout << "[";
		for(int i = 0; i < (int)v.size(); i++){
			cout << v[i];
			if(i < v.size()-1){
				cout << ", ";
			}
		}
		cout << "]"; 
	}

	vector<string> readArgs(int argc, char** argv){
		vector<string> args;
		for(int i = 1; i < argc; i++){
			string s;
			int j = 0;
			while(argv[i][j] != '\0') s.push_back(argv[i][j]), j++;
			args.push_back(s);
		}
		return args;
	}
}
