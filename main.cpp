#include <bits/stdc++.h>
#include <chrono>
#include <thread>

#include "debug_utils.hpp"


using namespace std;

int k = 0;

void in_k(){
	cin >> k;
	cout << "[thread 2] k is " << k << endl;
}

int main(int argc, char** argv){

	hut::printList(hut::readArgs(argc, argv));
	cout << endl;

	auto th = thread(in_k);

	int num = 0;
	int dir = 1;
	while(!k){
		cout << "[thread 1] k is";
		for(int i = 0; i < num; i++) cout << " ";
		cout <<"zero" << endl;
		num+=dir;
		if(num == 10) dir*=-1;
		if(num == 0) dir *=-1;
	}

	th.join();

	return 0;
}