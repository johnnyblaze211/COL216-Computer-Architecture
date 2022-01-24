/*code from assignment3 */

#include<iostream>
#include<string>
#include<vector>
#include<tuple>
#include<unordered_map>
#include<fstream>
#include<cmath>
#include<stdexcept>
#include<set>


#define MAX_INT_MEMORY 262144
#define INSTR_MEM_SIZE 32768
#define I2B 4
#define xx cout<<"xxx\n";

using namespace std;


struct instruction1{
    string ins_str;
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
set<int> mem;


int row_buffer_index = -1;
int buffer_activate_count = 0;
string line;
int PC = 0;
int ROW_ACCESS_DELAY, COL_ACCESS_DELAY;
int reg_file[32] = {0};
int clock_cycles = 0;
int DRAM[1024][256];
int DRAM_row_buffer[256];
int data_segment[MAX_INT_MEMORY] = {0};
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

      if(v1[0] == "add"){
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


    else return 1;

    ins.ins_str = s;

    instr.push_back(ins);

    if(instr.size() >= INSTR_MEM_SIZE) {cout<<"Overflow: Instruction Segment memory exceeded\n"; return 2;}
    return 0;

}

void print_reg(){
    char hex_string[20];
    sprintf(hex_string, "%X", PC);
    cout<<"\nPC\t"<<hex_string<<"\n";
    for(int i = 0;i < 32;i++){
        sprintf(hex_string, "%X", reg_file[i]);
        cout<<hex_string<<" ";
    }
    cout<<"\n";
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

/*code from assignment3 ends here, partially present in the remaining portion*/

int process(bool flag, int reg1, int reg1_val, int reg2, int addr1, int addr1_val, int end_clock, bool lw_bool)
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

            if(!flag){
              clock_cycles++;
              cout<<"Cycle "<<clock_cycles<<":"<<ins.ins_str<<"\n";
              cout<<"Register R"<<ins.dst_reg<<"="<<reg_file[ins.dst_reg]<<"\n";
              instr_count[PC/I2B]++;
              PC = PC + 4;
              return process(false, -1, -1, -1, -1, -1, -1, false);
            }

            else if(ins.dst_reg == reg1 || ins.dst_reg == reg2 || ins.src1_reg == reg1 || ins.src1_reg == reg2 || ins.src2_reg == reg1 || ins.src2_reg == reg2){
              clock_cycles++;
              cout<<"Cycle "<<clock_cycles<<"-"<<end_clock<<": \n";
              clock_cycles = end_clock;
              if(lw_bool){
                cout<<"Loaded Register R"<<reg1<<"="<<reg1_val<<" from DRAM"<<"\n";
              }
              else{
                cout<<"Updated Address "<<addr1<<"="<<addr1_val<<"\n";
                mem.insert(addr1);
              }
              clock_cycles++;
              cout<<"Cycle "<<clock_cycles<<": "<<ins.ins_str<<"\n";
              cout<<"Register R"<<ins.dst_reg<<"="<<reg_file[ins.dst_reg]<<"\n";
              instr_count[PC/I2B]++;
              PC = PC + 4;
              return process(false, -1, -1, -1, -1, -1, -1, false);

            }

            else{
              clock_cycles++;
              cout<<"Cycle "<<clock_cycles<<": "<<ins.ins_str<<"\n";
              if(lw_bool){
                cout<<"Loading Register R"<<reg1<<"="<<reg1_val<<" from DRAM and loaded register R"<<ins.dst_reg<<"="<<reg_file[ins.dst_reg]<<" from register file\n";
              }
              else{
                cout<<"Updating Address "<<addr1<<"="<<addr1_val<<" from DRAM and loaded register R"<<ins.dst_reg<<"="<<reg_file[ins.dst_reg]<<" from register file\n";
                mem.insert(addr1);
              }
              instr_count[PC/I2B]++;
              PC = PC + 4;
              if(clock_cycles == end_clock)
                return process(false, -1, -1, -1, -1, -1, -1, false);
              else
                return process(flag, reg1, reg1_val, reg2, addr1, addr1_val, end_clock, lw_bool);
            }

        }

        else if(ins.name == "addi"){

            reg_file[ins.dst_reg] = reg_file[ins.src1_reg] + ins.ival;

            if(reg_file[ins.src1_reg]>0 && ins.ival && reg_file[ins.dst_reg]<0)
                {cout<<"Overflow: Arithmetic Overflow Error\n"; return 2;}
            else if(reg_file[ins.src1_reg]<0 && ins.ival<0 && reg_file[ins.dst_reg]>0)
                {cout<<"Overflow: Arithmetic Overflow Error\n"; return 2;}

            if(!flag){
              clock_cycles++;
              cout<<"Cycle "<<clock_cycles<<":"<<ins.ins_str<<"\n";
              cout<<"Register R"<<ins.dst_reg<<"="<<reg_file[ins.dst_reg]<<"\n";
              instr_count[PC/I2B]++;
              PC = PC + 4;
              return process(false, -1, -1, -1, -1, -1, -1, false);
            }

            else if(ins.dst_reg == reg1 || ins.dst_reg == reg2 || ins.src1_reg == reg1 || ins.src1_reg == reg2){
              clock_cycles++;
              cout<<"Cycle "<<clock_cycles<<"-"<<end_clock<<": \n";
              clock_cycles = end_clock;
              if(lw_bool){
                cout<<"Loaded Register R"<<reg1<<"="<<reg1_val<<" from DRAM"<<"\n";
              }
              else{
                cout<<"Updated Address "<<addr1<<"="<<addr1_val<<"\n";
                mem.insert(addr1);
              }
              clock_cycles++;
              cout<<"Cycle "<<clock_cycles<<": "<<ins.ins_str<<"\n";
              cout<<"Loaded Register R"<<ins.dst_reg<<"="<<reg_file[ins.dst_reg]<<"\n";
              instr_count[PC/I2B]++;
              PC = PC + 4;
              return process(false, -1, -1, -1, -1, -1, -1, false);

            }

            else{
              clock_cycles++;
              cout<<"Cycle "<<clock_cycles<<": "<<ins.ins_str<<"\n";
              if(lw_bool){
                cout<<"Loading Register R"<<reg1<<"="<<reg1_val<<" from DRAM and loaded register R"<<ins.dst_reg<<"="<<reg_file[ins.dst_reg]<<" from register file\n";
              }
              else{
                cout<<"Updating Address "<<addr1<<"="<<addr1_val<<" from DRAM and loaded register R"<<ins.dst_reg<<"="<<reg_file[ins.dst_reg]<<" from register file\n";
                mem.insert(addr1);
              }
              instr_count[PC/I2B]++;
              PC = PC + 4;
              if(clock_cycles == end_clock)
                return process(false, -1, -1, -1, -1, -1, -1, false);
              else
                return process(flag, reg1, reg1_val, reg2, addr1, addr1_val, end_clock, lw_bool);
            }

        }

        else if(ins.name == "lw"){
            int addr = reg_file[ins.addr_reg] + ins.offset;
            if(addr%I2B != 0){cout<<"Unaligned addressing not permitted\n"; return 2;}
            if(addr/I2B >= MAX_INT_MEMORY || addr/I2B < 0){cout<<"Error: Bad Memory Address\n"; return 2;}

            instr_count[PC/I2B]++;
            PC = PC + 4;

            if(flag){
              clock_cycles++;
              cout<<"Cycle "<<clock_cycles<<"-"<<end_clock<<": \n";
              clock_cycles = end_clock;
              if(lw_bool){
                cout<<"Loaded Register R"<<reg1<<"="<<reg1_val<<" from DRAM\n";
              }
              else{
                cout<<"Updated Address "<<addr1<<"="<<addr1_val<<"\n";
                mem.insert(addr1);
              }
            }

            clock_cycles++;
            cout<<"Cycle "<<clock_cycles<<": "<<ins.ins_str<<"\n";
            cout<<"DRAM request issued\n";


            int int_addr = addr/I2B;
            if(int_addr/256 == row_buffer_index){
              reg_file[ins.dst_reg] = DRAM_row_buffer[int_addr % 256];

              return process(true, ins.dst_reg, DRAM_row_buffer[int_addr%256], ins.addr_reg, -420, -420, clock_cycles+COL_ACCESS_DELAY, true);
            }

            else if(row_buffer_index == -1){
              buffer_activate_count++;
              for(int i = 0;i<256;i++){
                DRAM_row_buffer[i] = DRAM[int_addr/256][i];
              }
              reg_file[ins.dst_reg] = DRAM_row_buffer[int_addr%256];
              row_buffer_index = int_addr/256;

              return process(true, ins.dst_reg, DRAM_row_buffer[int_addr%256], ins.addr_reg, -420, -420, clock_cycles+ROW_ACCESS_DELAY+COL_ACCESS_DELAY, true);
            }

            else{
              buffer_activate_count++;
              for(int i = 0; i < 256; i++){
                DRAM[row_buffer_index][i] = DRAM_row_buffer[i];
              }
              for(int i = 0;i<256;i++){
                DRAM_row_buffer[i] = DRAM[int_addr/256][i];
              }
              reg_file[ins.dst_reg] = DRAM_row_buffer[int_addr%256];

              row_buffer_index = int_addr/256;

              return process(true, ins.dst_reg, DRAM_row_buffer[int_addr%256], ins.addr_reg, -420, -420, clock_cycles+2*ROW_ACCESS_DELAY+COL_ACCESS_DELAY, true);
            }
        }

        else if(ins.name == "sw"){
            int addr = reg_file[ins.addr_reg] + ins.offset;
            if(addr%I2B != 0){cout<<"Unaligned addressing not permitted\n"; return 2;}
            if(addr/I2B >= MAX_INT_MEMORY || addr/I2B < 0){cout<<"Error: Bad Memory Address\n"; return 2;}
            instr_count[PC/I2B]++;
            PC = PC + 4;

            if(flag){
              clock_cycles++;
              cout<<"Cycle "<<clock_cycles<<"-"<<end_clock<<": \n";
              clock_cycles = end_clock;
              if(lw_bool){
                cout<<"Loaded Register R"<<reg1<<"="<<reg1_val<<" from DRAM\n";
              }
              else{
                cout<<"Updated Address "<<addr1<<"="<<addr1_val<<"from DRAM\n";
                mem.insert(addr1);
              }
            }

            clock_cycles++;
            cout<<"Cycle "<<clock_cycles<<": "<<ins.ins_str<<"\n";
            cout<<"DRAM request issued\n";

            int int_addr = addr/I2B;
            if(int_addr / 256 == row_buffer_index){
              DRAM_row_buffer[int_addr%256] = reg_file[ins.dst_reg];

              return process(true, ins.dst_reg, -420, ins.addr_reg, addr, reg_file[ins.dst_reg], clock_cycles+COL_ACCESS_DELAY, false);
            }

            else if(row_buffer_index == -1){
              buffer_activate_count++;
              for(int i = 0;i<256;i++){
                DRAM_row_buffer[i] = DRAM[int_addr/256][i];
              }
              DRAM_row_buffer[int_addr%256] = reg_file[ins.dst_reg];

              row_buffer_index = int_addr/256;

              return process(true, ins.dst_reg, -420, ins.addr_reg, addr, reg_file[ins.dst_reg], clock_cycles+ROW_ACCESS_DELAY+COL_ACCESS_DELAY, false);
            }

            else{
              buffer_activate_count++;
              for(int i = 0; i < 256; i++){
                DRAM[row_buffer_index][i] = DRAM_row_buffer[i];
              }
              for(int i = 0;i<256;i++){
                DRAM_row_buffer[i] = DRAM[int_addr/256][i];
              }
              DRAM_row_buffer[int_addr%256] = reg_file[ins.dst_reg];

              row_buffer_index = int_addr/256;

              return process(true, ins.dst_reg, -420, ins.addr_reg, addr, reg_file[ins.dst_reg], clock_cycles+2*ROW_ACCESS_DELAY+COL_ACCESS_DELAY, false);
            }

        }

        else if(ins.name == "check"){
            if(flag){
              clock_cycles++;
              cout<<"Cycle "<<clock_cycles<<"-"<<end_clock<<": \n";
              clock_cycles = end_clock;
              if(lw_bool){
                cout<<"Loaded Register R"<<reg1<<"="<<reg1_val<<" from DRAM"<<"\n";
              }
              else{
                cout<<"Updated Address "<<addr1<<"="<<addr1_val<<"\n";
                mem.insert(addr1);

              }
            }
            return 0;
        }

        else {
            {cout<<"Error: Invalid operation "<<ins.name<<": "<<ins.ins_str; return 2;}
        }
    }

}








int main(int argc, char * argv[]){

    if(argc != 4){
        cout<<"3 CommandLine arguments required\n"; return 1;
    }
    ifstream infile (argv[1]);

    if(!isNum(argv[2]) || s_to_i(argv[2]) <= 0) {cout<<"ROW_ACCESS_DELAY must be a positive integer\n"; return 1;}
    if(!isNum(argv[3]) || s_to_i(argv[3]) <= 0) {cout<<"COL_ACCESS_DELAY must be a positive integer\n"; return 1;}

    ROW_ACCESS_DELAY = s_to_i(argv[2]);
    COL_ACCESS_DELAY = s_to_i(argv[3]);

    if(infile.is_open())
    {
        int ln = 0;
        while(getline(infile, line)){
            if(line.empty()) {ln++;continue;}
            ln++;

            int n1_prev = instr.size();
            int err_code = parse(line);
            if(instr.size() != n1_prev) continue;
            if(err_code == -1||err_code == 1) {cout<<"Abort: Syntax error at line "<<ln<<"\n"; return 1;}
            if(err_code > 1) {return 1;}


        }



        if(instr.size() == 0) {cout<<"Abort: Empty Input file\n"; return 1;}

        struct instruction1 ins_temp;
        ins_temp.name = "check";
        instr.push_back(ins_temp);



        //print_instr();
        //print_reg();



        int err_code = process(false, -1, -1, -1, -1, -1, -1, false);
        if (err_code!= 0) {return 1;}
        else{
            if (buffer_activate_count > 0) cout<<"\n\nRow Buffer Activation Count = "<<buffer_activate_count<<"\n";

            if(clock_cycles>0)cout<<"\nClock Cycles: "<<clock_cycles<<"\n";
            else cout<<"Clock Cycles: "<<0<<"\n";
            cout<<endl;
            cout<<"Register file:\n";
            print_reg();
            if(mem.size()>0)cout<<"\nUpdated memory values: \n";
            else cout<<"\nNo memory values updated\n";
            for(auto i:mem){
              if(i/1024 == row_buffer_index) cout<<"Address "<<i<<"-"<<i+3<<" = "<<DRAM_row_buffer[(i/4)%256]<<"\n";
              else cout<<"Address "<<i<<"-"<<i+3<<" = "<<DRAM[i/1024][(i%1024)/4]<<"\n";
            }

        }
    }

    else {cout<<"Abort: File not found\n"; return 1;}

    infile.close();





  return 0;
}
