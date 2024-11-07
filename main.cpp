#include<iostream>
#include<algorithm>
#include<string>
#include"volemachine.h"
#include<fstream>
#include <vector>
#include <cctype>

using namespace std;

// GLOBAL FUNCTIONS
string dec_to_base(int value, int base)
{
    int val = value;
    string new_value;
    while (val)
    {
        int rem = val % base;
        val /= base;
        char di = (rem < 10 ? rem + '0' : 'A' + rem - 10);
        new_value.push_back(di);
    }
    if (new_value.empty()) new_value.push_back('0');
    reverse(all(new_value));
    return new_value;
}
int base_to_dec(string value, int base)
{
    reverse(all(value));
    int pos{ 1 }, num{};
    for (auto& c : value)
    {
        int di = (isalpha(c) ? c - 'A' + 10 : c - '0');
        num += di * pos;
        pos *= base;
    }
    return num;
}
int base_to_dec(char c)
{
    int di = (isalpha(c) ? c - 'A' + 10 : c - '0');
    return di;
}

// CLASS MEMORY
Memory::Memory(int val) : value{ val }
{

}
void Memory::set_value(int val)
{
    value = val;
}
int Memory::get_value() const
{
    return value;
}
string Memory::bi_value() const
{
    string bi_val = dec_to_base(value, 2);
    while (bi_val.size() < 8) bi_val = '0' + bi_val;
    return bi_val;
}
string Memory::hex_value() const
{
    string hex_val = dec_to_base(value, 16);
    while (hex_val.size() < 2) hex_val = '0' + hex_val;
    return hex_val;
}
int Memory::twos_comp_value() const
{
    string bi_val = bi_value();
    int num{};
    bool rev{};
    if (bi_val[0] == '1')
    {
        int sz = bi_val.size();
        for (int i{ sz - 1 }; i >= 0; i--)
        {
            if (rev) bi_val[i] = !(bi_val[i] - '0') + '0';
            if (bi_val[i] - '0' != 0) rev = true;
        }
        return base_to_dec(bi_val, 2) * -1;
    }
    return value;
}
double Memory::float_value() const
{
    string bi_val = bi_value(), man = bi_val.substr(4, 4);
    double num{};
    int dot_idx = base_to_dec(bi_val.substr(1, 3), 2) - 4;
    while (dot_idx < 0) man.insert(0, "0"), dot_idx++;
    int exp = dot_idx - 1;
    for (each c : man)
    {
        num += (c - '0') * pow(2, exp);
        exp--;
    }
    if (bi_val[0] - '0' != 0) num *= -1;
    return num;
}

// CLASS REGISTER
bool Register::operator==(const Memory& rhs) const
{
    return value == rhs.get_value();
}
bool Register::operator!=(const Memory& rhs) const
{
    return value != rhs.get_value();
}
Register Register::operator=(const Memory& rhs)
{
    value = rhs.get_value();
    return *this;
}
Register Register::operator=(const int rhs)
{
    value = rhs;
    return *this;
}
Register Register::operator++()
{
    value++;
    return *this;
}
Register Register::operator+=(const Register& rhs)
{
    value += rhs.value;
    return *this;
}
Register Register::operator+=(const int rhs)
{
    value += rhs;
    return *this;
}

// CLASS ALU
string ALU::float_to_bi(double d)
{
    string val;
    for (int i = 0; i < 8; i++)
    {
        d *= 2;
        if (d >= 1) val.push_back('1'), d--;
        else val.push_back('0');
    }
    return val;
}

int ALU::add_int(int val1, int val2)
{
    int sum = val1 + val2;
    string bi_sum = dec_to_base(sum, 2);
    if (bi_sum.size() > 8) bi_sum = bi_sum.substr(bi_sum.size() - 8, 8);
    return base_to_dec(bi_sum, 2);
}

double ALU::add_float(double val1, double val2)
{
    double sum = val1 + val2;
    int exp{ 4 };
    string bi_val, man;
    if (sum < 0) sum *= -1, bi_val.push_back('1');
    else bi_val.push_back('0');

    man = dec_to_base(sum, 2);
    exp = min(static_cast<int>(exp + man.size()), 7);

    sum -= static_cast<int>(sum);
    string tmp = float_to_bi(sum);
    if (exp == 4)
    {
        while (exp && tmp[0] == '0')
        {
            exp--;
            tmp.erase(0, 1);
        }
    }
    man += tmp;
    bi_val += dec_to_base(exp, 2) + man.substr(0, 4);
    return base_to_dec(bi_val, 2);
}

// CLASS CU
void CU::load(string& ins)
{
    int register_idx = base_to_dec(ins[1]), memory_cell_idx = base_to_dec(ins.substr(2, 2), 16);
    R[register_idx] = M[memory_cell_idx];
}

void CU::load_reg(string& ins)
{
    int register_idx = base_to_dec(ins[1]), value = base_to_dec(ins.substr(2, 2), 16);
    R[register_idx] = value;
}

void CU::store(string& ins)
{
    int register_idx = base_to_dec(ins[1]), memory_cell_idx = base_to_dec(ins.substr(2, 2), 16);
    M[memory_cell_idx] = R[register_idx];
    if (memory_cell_idx == 0) screen.push_back(M[memory_cell_idx].get_value());
}

void CU::move(string& ins)
{
    int register_idx_1 = base_to_dec(ins[2]), register_idx_2 = base_to_dec(ins[3]);
    R[register_idx_2] = R[register_idx_1];
}

void CU::add_twos_compliment(string& ins)
{
    int r[3];
    for (int i{}; i < 3; i++) r[i] = base_to_dec(ins[i + 1]);
    R[r[0]] = AU.add_int(R[r[1]].get_value(), R[r[2]].get_value());
}

void CU::add_floating_point(string& ins)
{
    int r[3], exp{ 4 };
    for (int i{}; i < 3; i++) r[i] = base_to_dec(ins[i + 1]);
    R[r[0]] = AU.add_float(R[r[1]].float_value(), R[r[2]].float_value());
}

void CU::jump(string& ins)
{
    int register_idx = base_to_dec(ins[1]), memory_cell_idx = base_to_dec(ins.substr(2, 2), 16);
    if (R[register_idx] == R[0]) PCtr = memory_cell_idx;
}

bool CU::halt()
{
    if (PCtr.get_value() > 254 || is_halt)
    {
        is_halt = true;
        return false;
    }
    string ins = M[PCtr.get_value()].hex_value();
    ++PCtr;
    ins += M[PCtr.get_value()].hex_value();
    if (PCtr.get_value() < 255) ++PCtr;
    InsR = base_to_dec(ins, 16);
    switch (ins[0])
    {
        case '1':
        {
            load(ins);
            return true;
        }
        case '2':
        {
            load_reg(ins);
            return true;
        }
        case '3':
        {
            store(ins);
            return true;
        }
        case '4':
        {
            move(ins);
            return true;
        }
        case '5':
        {
            add_twos_compliment(ins);
            return true;
        }
        case '6':
        {
            add_floating_point(ins);
            return true;
        }
        case 'B':
        {
            jump(ins);
            return true;
        }
    }
    if (ins == "C000")
    {
        is_halt = true;
        return true;
    }
    return false;
}

void Machine::reset()
{
    for (int i{}; i < rCount; i++)
    {
        R[i] = 0;
    }
    for (int i{}; i < mSize; i++)
    {
        M[i] = 0;
    }
    PCtr = 0;
    is_halt = false;
    screen.clear();
}

bool Machine::valid_value(string& ins)
{
    for (each c : ins)
    {
        if (c < '0' || c > 'F')
        {
            return false;
        }
    }
    return true;
}

// CLASS MACHINE
Machine::Machine(int memory_size, int register_count) : mSize(memory_size), rCount(register_count)
{
    M = new Memory[memory_size];
    R = new Register[register_count];
    PCtr = 0;
    is_halt = false;
}

int Machine::registerCount()
{
    return rCount;
}

int Machine::memorySize()
{
    return mSize;
}

string Machine::sceen_content()
{
    return screen;
}

Memory& Machine::atM(int index)
{
    return M[index];
}

Register& Machine::atR(int index)
{
    return R[index];
}

string Machine::PC()
{
    return PCtr.hex_value();
}

string Machine::IR()
{
    return InsR.hex_value();
}

bool Machine::halted()
{
    return is_halt;
}

Machine::~Machine()
{
    delete[] M;
    delete[] R;
}

int main()
{
    Machine machine;
    int op;
    string ls[] = {
            "Load a file program",
            "Enter one 4-bit instruction",
            "Display register",
            "Display memory",
            "Run until halt",
            "Run one cycle",
    };
    while (1)
    {
        int list_size = (machine.halted() ? 4 : 6);
        cout << "====================================\n";
        for (int i{ 1 }; i <= list_size; i++)
        {
            cout << i << "-" << ls[i - 1] << endl;
        }
        cout << "0-Exit\n";
        cout << "====================================\n";
        cout << "Response: ";
        cin >> op;
        if (op < 0 || op > list_size) continue;

        switch (op)
        {
            case 0:
            {
                return 0;
            }
            case 1:
            {
                machine.reset();
                string file_name, ins, content;
                char c;
                int idx{};
                cout << "Filename: ";
                cin >> file_name;
                ifstream fs(file_name);
                while (1)
                {
                    string hex_idx;
                    cout << "Starting at (hex index): ";
                    cin >> hex_idx;
                    idx = base_to_dec(hex_idx, 16);
                    if (idx >= 0 && idx < 256) break;
                }
                while (fs >> c)
                {
                    content.push_back(c);
                }
                for (int i{}; i + 1 < content.size(); i += 2)
                {
                    ins.push_back(content[i]), ins.push_back(content[i + 1]);
                    machine.atM(idx).set_value(base_to_dec(ins, 16));
                    idx++;
                    ins.clear();
                }
                break;
            }
            case 2:
            {
                machine.reset();  // Reset the machine as before

                string ins;
                char c;
                int idx{};

                cout << "Enter the instruction (in hex): ";
                cin >> ins;  // Read the instruction (hexadecimal)

                while (1)
                {
                    string hex_idx;
                    cout << "Starting at (hex index): ";
                    cin >> hex_idx;
                    idx = base_to_dec(hex_idx, 16);  // Convert hex to decimal
                    if (idx >= 0 && idx < 256) break;
                }

                // Assuming the instruction is in pairs of hex digits
                for (int i = 0; i < ins.size(); i += 2)
                {

                    string instruction_pair = ins.substr(i, 2);
                    machine.atM(idx).set_value(base_to_dec(instruction_pair, 16));
                    idx++;  // Move to the next memory index
                }
                break;
            }
            case 3:
            {
                for (int i{}; i < machine.registerCount(); i++)
                {
                    cout << 'R' << dec_to_base(i, 16) << ": " << machine.atR(i).hex_value() << endl;
                }
                cout << "PC: " << machine.PC() << " IR: " << machine.IR() << endl;
                break;
            }
            case 4:
            {
                cout << "  ";
                for (int i{}; i < 16; i++)
                {
                    char c = (i < 10 ? i + '0' : i - 10 + 'A');
                    cout << c << "  ";
                }
                cout << endl;
                for (int i{}; i < 16; i++)
                {
                    char c = (i < 10 ? i + '0' : i - 10 + 'A');
                    cout << c << " ";
                    for (int j{}; j < 16; j++)
                    {
                        cout << machine.atM(i * 16 + j).hex_value() << " ";
                    }
                    cout << endl;
                }
                break;
            }
            case 5:
            {
                while (!machine.halted())
                {
                    machine.halt();
                }
                break;
            }
            case 6:
            {
                machine.halt();
                break;
            }
        }
    }
}