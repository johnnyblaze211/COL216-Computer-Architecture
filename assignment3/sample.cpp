#include <iostream>
#include<cmath>
using namespace std;

int s_to_i(string s){
    int n = 0;
    for(int i = 0;i<s.length();i++){
        if(!(s.at(i) < 48 || s.at(i)>57)){
            n = n + (s.at(i) - '0')*pow(10, s.length() - 1 - i);
        }

        else return -1;
    }

    return n;
}


int reg_n(string s){
	int n;
    if(s.length() <=1 || s.at(0) != '$') return -1;

    else if(s.length() > 3) return -1;

    else{

        int n1 = s.at(1) - '0';
        n = n1;
        if (n1 > 9) return -1;
        if(s.length() == 3) {
            n1 = s.at(2) - '0';
            if(n1 > 9) return -1;
            n = 10 * n + n1;
        }

        if(n>31) return -1;
        
        else return n;
        
    }
}

int* reg_val_offset(string s){
    
    static int val_offset_arr[2] = {0,0};
    string lparen = "(";
    size_t pos = s.find(lparen);
    int result = 0;
    int offset = 0;
    if(pos==string::npos){
        result = reg_n(s);
    }

    else{
        if(s.at(s.length() - 1) == ')'){
            string s2 = s.substr(pos, s.length());
            result = reg_n(s2);

           if(result!=-1 && pos!=0){
                string s3 = s.substr(0, pos);
                int offset = s_to_i(s3);
            }

        }
        else result = -1;

    }

    val_offset_arr[0] = result;
    val_offset_arr[1] = offset;

    return val_offset_arr;

    
}

int main() {
	string s = "($31)";
	cout<<reg_val_offset(s)[0];
	cin.ignore();
	cout<<"yay";
	cin.ignore();
	return 0;
}