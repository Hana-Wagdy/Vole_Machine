
// IMPROTANT NOTES FOR GRADING TUT:
// please notice the following features of out programme
// 1. register doesnt change to you chose respone to halt
// 2. there is two options halt till run and halt one cycle
// if one of them isnt nessecairy please ignore it
// thank you so much

// Maya Mohamed salah 20230311   s2
// Hana wagdy nagy 20230459      s1
// Marian ahmed abelwahab ali 20230777

// git hub repo link :

//https://github.com/Hana-Wagdy/Vole_Machine

#ifndef VOLEMACHINE_H
#define VOLEMACHINE_H

#include <iostream>
#include <vector>
#include <string>
#include<algorithm>
#define all(v) v.begin(), v.end()
#define each auto &
using namespace std;

string dec_to_base(int value, int base);
int base_to_dec(string value, int base);
int base_to_dec(char c);

class Memory {
private:
    int size_;

public:
    int value{ 0 };

    Memory(int val = 0);
    void set_value(int val);
    int get_value() const;
    string bi_value() const;
    string hex_value() const;
    int twos_comp_value() const;
    double float_value() const;
};

class Register : public Memory {
private:
    vector<int> cells_;
    int size_;

public:
    bool operator==(const Memory& rhs) const;
    bool operator!=(const Memory& rhs) const;
    Register operator=(const Memory& rhs);
    Register operator=(const int rhs);
    Register operator++();
    Register operator+=(const Register& rhs);
    Register operator+=(const int rhs);
};

class ALU {
public:

    string float_to_bi(double d);
    int add_int(int val1, int val2);
    double add_float(double val1, double val2);
};

class CU  {
protected:
    ALU AU;
    Memory* M;
    Register* R, PCtr, InsR;
    bool is_halt;
    string screen;

    // DECODE
public:
    void load(string& ins);                //instruction 1
    void load_reg(string& ins);            //instruction 2
    void store(string& ins);               //instruction 3
    void move(string& ins);                //instruction 4
    void add_twos_compliment(string& ins); //instruction 5
    void add_floating_point(string& ins);  //instruction 6
    void jump(string& ins);                //instruction B
    bool halt();                           //instruction C
};

class Machine : public CU {

    int rCount, mSize;
public:
    Machine(int memory_size = 256, int register_count = 16);
    bool valid_value(string& ins);
    int registerCount();
    int memorySize();
    string sceen_content();
    Memory& atM(int index);
    Register& atR(int index);
    void reset();
    string PC();
    string IR();
    bool halted();
    ~Machine();
};

#endif // VOLE_MACHINE_H
