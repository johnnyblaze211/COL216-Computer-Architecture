#include<iostream>
#include<string>
#include<vector>
#include<tuple>
#include<unordered_map>
#include<fstream>
#include<cmath>
#include<stdexcept>

#define MAX_INT_MEMORY 262144
#define INSTR_MEM_SIZE 32768
#define I2B 4
#define ADD 0
#define SUB 1
#define MUL 2
#define SLT 3
#define ADDI 4
#define LW 5
#define SW 6
#define BEQ 7
#define BNE 8
#define J 9
#define xx cout<<"xxx\n";

using namespace std;


struct instruction1{
    string name;
    int dst_reg;
    int src1_reg;
    int src2_reg;
    int addr_reg;
    int addr_val;
    int offset;
    int ival;
};

vector<instruction1> instr;
int PC = 0;
int reg_file[32] = {0};
int clock_cycles;
int data_segment[MAX_INT_MEMORY] = {0};
bool instr_bool[INSTR_MEM_SIZE] = {false};
int instr_count[INSTR_MEM_SIZE] = {0};

vector<string> tokenize(string s){
    vector<string> v;
    int i = 0;
    if(s.empty()) {/*v.push_back("empty");*/ return v;}
    bool done = false;

    int i2 = 0;
    while(i2<s.length() && (s.at(i2) == ' ' || s.at(i2) == '\t')){i2++;}
    //if(i2 == s.length()) {v.push_back("empty"); return v;}
    //else if(s.at(i2) == '#') {v.push_back("empty"); return v;}
    while(i<s.length()){
        int start = -1;
        while(i<s.length() && ((s.at(i) == ' ')||(s.at(i) == '\t'))) i++;
        if(i<s.length()) start = i;
        while(i<s.length() && !((s.at(i) == ' ')||(s.at(i) == '\t')||(s.at(i)=='#')||s.at(i) == ',')) i++;
        if(i<s.length()) {
            if(s.at(i) == '#') {
                if(s.substr(start, i - start) == "")  break;
                else {v.push_back(s.substr(start, i - start)); break;}
            }
            else if(s.at(i) == ','){
                if(s.substr(start, i - start) == "") {v.push_back(","); i++; continue;}
                else{
                    v.push_back(s.substr(start, i - start));
                    v.push_back(",");
                    i++;
                    continue;
                }
            }
        }
        if(start!=-1) v.push_back(s.substr(start, i - start));

    }

    
    return v;
}

int s_to_i(string s){
    int n = 0;
    bool neg = false;
    if(s.length() > 1 && s.at(0) == '-'){neg = true; s = s.substr(1, s.length());}
    for(int i = 0;i<s.length();i++){
        if(!(s.at(i) < 48 || s.at(i)>57)){
            n = n + (s.at(i) - '0')*pow(10, s.length() - 1 - i);
        }

        else return -1;
    }

    if(neg) return (n*(-1));
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

bool isNum(string s){
    if(s.length() > 1) if(s.at(0) - '0' == 0) return false;
    for (int i = 0; i < s.length(); i++)
        {if(i==0 && s.length() > 1 && s.at(i) == '-') continue;
        if (isdigit(s[i]) == false)
            return false;}
 
    return true;
}

int* reg_val_offset(string s){
	
    static int val_offset_arr[2] = {0,0};
    if (isNum(s)){
        val_offset_arr[0] = stoi(s);
        val_offset_arr[1] = 0;
        return val_offset_arr;
    }
    
    string lparen = "(";
    size_t pos = s.find(lparen);
    int result = 0;
    int offset = 0;
    


    if(pos==string::npos){
    	
        result = reg_n(s);
    }

    else{
        if(s.at(s.length() - 1) == ')'){
        	if(pos==s.length() - 1){val_offset_arr[0] = -1; val_offset_arr[1] = 0; return val_offset_arr;}
            string s2 = s.substr(pos+1, s.length() - 1 - pos - 1);
            
        	
            result = reg_n(s2);
  

           if(result!=-1 && pos!=0){
                string s3 = s.substr(0, pos);
                //cout<<s3<<"\n";
                offset = s_to_i(s3);
            }

        }
        else result = -1;

    }

    val_offset_arr[0] = result;
    val_offset_arr[1] = offset;

    cout<<"result "<<result<<" offset "<<offset<<"\n\n";

    return val_offset_arr;

    
}







int parse(string s){

    

    struct instruction1 ins = instruction1();
    ins.src1_reg = 0;
    ins.src2_reg = 0;
    ins.dst_reg = 0;
    ins.offset = 0;
    ins.ival = 0;
    ins.addr_reg = 0;
    ins.addr_val = 0;
    
    vector<string> v1 = tokenize(s);
    if(v1.size() > 1){
        
        if(v1[0] == "j"){
            if(v1.size() != 2) return 1;
            if(!isNum(v1[1])) return 1;
            else{
                ins.addr_val = stoi(v1[1]);
                ins.name = "j";
            }   
        }

        else if(v1[0] == "add"||v1[0] == "sub"||v1[0] == "mul"||v1[0] == "slt"){
            if(v1.size()!=6) return 1;
            if(v1[2] !="," || v1[4]!=",") return 1;
            int r_dst = reg_n(v1[1]);
            if(r_dst == 0) {cout<<"Abort: Cannot assign to zero register\n"; return 3;}
            int r_src1 = reg_n(v1[3]);
            int r_src2 = reg_n(v1[5]);

            if(r_dst == -1 || r_src1 == -1 || r_src2 == -1) return 1;
            else{
                ins.name = v1[0];
                ins.dst_reg = r_dst;
                ins.src1_reg = r_src1;
                ins.src2_reg = r_src2;
            }

        }

        else if(v1[0] == "beq" || v1[0] == "bne"){
            if(v1.size()!=6) return 1;
            if(v1[2] !="," || v1[4]!=",") return 1;
            int r_src1 = reg_n(v1[1]);
            int r_src2 = reg_n(v1[3]);
            if(!(isNum(v1[5]))) return 1;

            if(r_src1 == -1 || r_src2 == -1) return 1;

            else{
                ins.src1_reg = r_src1;
                ins.src2_reg = r_src2;
                ins.addr_val = s_to_i(v1[5]);
                ins.name = v1[0];
            }
        }

        else if(v1[0] == "lw" || v1[0] == "sw"){
            if(v1.size()!=4) return 1;
            if(v1[2]!=",") return 1;

            int r_dst = reg_n(v1[1]);
            if(r_dst == 0) {cout<<"Abort: Cannot assign to zero register\n"; return 3;}
            int* val_offset_arr = reg_val_offset(v1[3]);
            if(val_offset_arr[0] == -1 || val_offset_arr[1] == -1) return 1;
            else{
                ins.dst_reg = r_dst;
                ins.addr_reg = val_offset_arr[0];
                ins.offset = val_offset_arr[1];
                ins.name = v1[0];
            }


        }

        else if(v1[0] == "addi"){
            if(v1.size()!=6) return 1;
            if(v1[2] != "," || v1[4] != ",") return 1;

            int r_dst = reg_n(v1[1]);
            if(r_dst == 0) {cout<<"Abort: Cannot assign to zero register\n"; return 3;}
            int r_src1 = reg_n(v1[3]);
            bool i_val_bool = isNum(v1[5]);
            int ival = s_to_i(v1[5]);

            if(r_dst == -1 || r_src1 == -1 || i_val_bool == false) return 1;
            else {
                ins.dst_reg = r_dst;
                ins.src1_reg = r_src1;
                ins.ival = ival;
                ins.name = v1[0];
            }

        }

        else return 1;
    }

    else if(v1.size() == 0){
        return -2;
    }

    /*else if(v1.size() == 1 && v1[0] == "empty"){
        return -2;
    }*/

    else return 1;

    instr.push_back(ins);

    if(instr.size() >= INSTR_MEM_SIZE) {cout<<"Overflow: Instruction Segment memory exceeded\n"; return 2;}
    return 0;

}

void print_reg(){
    char hex_string[20];
    sprintf(hex_string, "%X", PC);
    cout<<"\nPC\t"<<hex_string<<"\n\n";
    for(int i = 0;i < 32;i++){
        sprintf(hex_string, "%X", reg_file[i]);
        cout<<"R"<<i<<"\t"<<hex_string<<"\n";
    }
    cin.ignore();
}

void print_instr(){
    for(int i = 0;i<instr.size();i++){
        cout<<"instr.name "<<instr[i].name<<"\n";
        cout<<"instr.src1_reg "<<instr[i].src1_reg<<"\n";
        cout<<"instr.src2_reg "<<instr[i].src2_reg<<"\n";
        cout<<"instr.dst_reg "<<instr[i].dst_reg<<"\n";
        cout<<"instr.ival "<<instr[i].ival<<"\n";
        cout<<"instr.addr_reg "<<instr[i].addr_reg<<"\n";
        cout<<"instr.offset "<<instr[i].offset<<"\n";
        cout<<"addr_val "<<instr[i].addr_val<<"\n\n";
    }
}

int process()
{
    
    struct instruction1 ins;
    if(PC/I2B >=32768){
        
        cout<<"Abort: Instruction address out of bounds\n";
        return 1;
        
    }


    else if(PC/I2B<32768 && PC/I2B>=instr.size()) {return 0;}

    else{
        
        ins = instr[PC/I2B];
        if(ins.name == "add"){
            reg_file[ins.dst_reg] = reg_file[ins.src1_reg] + reg_file[ins.src2_reg];
            if(reg_file[ins.src1_reg]>0 && reg_file[ins.src2_reg]>0 && reg_file[ins.dst_reg]<0)
                {cout<<"Overflow: Arithmetic Overflow Error\n"; return 2;}
            else if(reg_file[ins.src1_reg]<0 && reg_file[ins.src2_reg]<0 && reg_file[ins.dst_reg]>0)
                {cout<<"Overflow: Arithmetic Overflow Error\n"; return 2;}

            instr_count[PC/I2B]++;
            PC = PC + 4;
        }

        else if(ins.name == "sub"){
            reg_file[ins.dst_reg] = reg_file[ins.src1_reg] - reg_file[ins.src2_reg];
            if(reg_file[ins.dst_reg]>0 && reg_file[ins.src2_reg]>0 && reg_file[ins.src1_reg]<0)
                {cout<<"Overflow: Arithmetic Overflow Error\n"; return 2;}
            else if(reg_file[ins.dst_reg]<0 && reg_file[ins.src2_reg]<0 && reg_file[ins.src1_reg]>0)
                {cout<<"Overflow: Arithmetic Overflow Error\n"; return 2;}
            instr_count[PC/I2B]++;
            PC = PC + 4;
        }

        else if(ins.name == "mul"){
            reg_file[ins.dst_reg] = reg_file[ins.src1_reg] * reg_file[ins.src2_reg];
            if(reg_file[ins.src1_reg]!=0 && reg_file[ins.src2_reg]!=0){
                if(reg_file[ins.src1_reg] != reg_file[ins.dst_reg]/reg_file[ins.src2_reg]){
                    cout<<"Overflow: Arithmetic Overflow Error\n"; return 2;
                }
            }
            
            instr_count[PC/I2B]++;
            PC = PC + 4;
        }

        else if(ins.name == "slt"){
            reg_file[ins.dst_reg] = (reg_file[ins.src1_reg] < reg_file[ins.src2_reg]) ? 1 : 0;
            instr_count[PC/I2B]++;
            PC = PC + 4;

        }

        else if(ins.name == "addi"){
            
            reg_file[ins.dst_reg] = reg_file[ins.src1_reg] + ins.ival;
            
            if(reg_file[ins.src1_reg]>0 && ins.ival && reg_file[ins.dst_reg]<0)
                {cout<<"Overflow: Arithmetic Overflow Error\n"; return 2;}
            else if(reg_file[ins.src1_reg]<0 && ins.ival<0 && reg_file[ins.dst_reg]>0)
                {cout<<"Overflow: Arithmetic Overflow Error\n"; return 2;}
            instr_count[PC/I2B]++;
            PC = PC + 4;
        }

        else if(ins.name == "lw"){
            int addr = reg_file[ins.addr_reg] + ins.offset;
            if(addr%I2B != 0){cout<<"Unaligned addressing not permitted\n"; return 2;}
            if(addr/I2B >= MAX_INT_MEMORY || addr/I2B < INSTR_MEM_SIZE){cout<<"Error: Bad Memory Address\n"; return 2;}
                
            reg_file[ins.dst_reg] = data_segment[addr/I2B];
            instr_count[PC/I2B]++;
            PC = PC + 4;
        }

        else if(ins.name == "sw"){
            int addr = reg_file[ins.addr_reg] + ins.offset;
            if(addr%I2B != 0){cout<<"Unaligned addressing not permitted\n"; return 2;}
            if(addr/I2B >= MAX_INT_MEMORY || addr/I2B <INSTR_MEM_SIZE){cout<<"Error: Bad Memory Address\n"; return 2;}
            data_segment[addr/I2B] = reg_file[ins.dst_reg];
            instr_count[PC/I2B]++;
            PC = PC + 4;
        }

        else if(ins.name == "beq"){
            int addr = ins.addr_val;
            if(addr%I2B != 0){cout<<"Unaligned addressing not permitted\n"; return 2;}
            if(reg_file[ins.src1_reg] == reg_file[ins.src2_reg]){
                if(addr/I2B >= INSTR_MEM_SIZE || addr/I2B <0){cout<<"Error: Bad Instruction Address\n"; return 2;}
                instr_count[PC/I2B]++;
                PC = addr;
            }
            else {instr_count[PC/I2B]++;PC = PC + 4;}
        }

        else if(ins.name == "bne"){
            int addr = ins.addr_val;
            if(addr%I2B != 0){cout<<"Unaligned addressing not permitted\n"; return 2;}
            if(reg_file[ins.src1_reg] != reg_file[ins.src2_reg]){
                if(addr/I2B >= INSTR_MEM_SIZE || addr/I2B < 0){cout<<"Error: Bad Instruction Address\n"; return 2;}
                instr_count[PC/I2B]++;
                PC = addr;
                //cout<<"\n\n\n\n"<<addr<<"\n\n\n\n";
            }
            else {instr_count[PC/I2B]++;PC = PC + 4;}
        }

        else if(ins.name == "j"){
            int addr = ins.addr_val;
            if(addr%I2B != 0){cout<<"Unaligned addressing not permitted\n"; return 2;}
            if(addr/I2B >= INSTR_MEM_SIZE || addr/I2B<0){cout<<"Error: Bad Instruction Address\n"; return 2;}
            else{

                instr_count[PC/I2B]++;
                PC = addr;
            }
        }

        else {
            {cout<<"Error: Invalid operation "<<ins.name<<": instruction "<<PC/I2B<<"\n"; return 2;}
        }

        


        clock_cycles++;
        print_reg();
    }
    


    
    return process();
    
}








int main(int argc, char * argv[]){

    string line;

    if(argc > 2||argc == 0){
        cout<<"Single input file required\n";
    }
    ifstream infile (argv[1]);
    if(infile.is_open())
    {   
        int ln = 0;
        while(getline(infile, line)){
            if(line.empty()) {ln++;continue;}
            ln++;
            int n1_prev = instr.size();
            int err_code = parse(line);
            if(instr.size() != n1_prev) continue;
            if(err_code == -1 || err_code == 1) {cout<<"Abort: Syntax error at line "<<ln<<"\n"; return 1;}
            if(err_code > 1) {return 1;}
            
            
        }



        if(instr.size() == 0) {cout<<"Abort: Empty Input file\n"; return 1;}



        //print_instr();
        print_reg();
        

        
        int err_code = process();
        if (err_code!= 0) {return 1;}
        else{
            if(instr.size() > 0)cout<<"\n\nInstruction\t\t\tCount\n";
            for(int i = 0;i < instr.size();i++){
                cout<<"Instruction address "<<4*i<<"\t\t"<<instr_count[i]<<"\n";
            }

            if(clock_cycles>0)cout<<"\nClock Cycles: "<<clock_cycles<<"\n\n";
            else cout<<"Clock Cycles: "<<0<<"\n";

        }
    }

    else {cout<<"Abort: File not found\n"; return 1;}

    
    

  
  return 0;
}
