#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <array>
#include <cassert>
using namespace std;
#endif /* __PROGTEST__ */

class CRecord
{
    fstream file;
    vector<string> commands;
    vector<string> contacts;
    int total_commands;
    int total_contacts;

public:
    CRecord(const string file_name)
    {
        this->file.open(file_name, ios::in);
        this->total_commands = 0;
        this->total_contacts = 0;
    }

    bool is_number(string contact)
    {
        for (int i = 0; contact[i] != '\0'; i++)
        {
            if (!isdigit(contact[i]))
                return 0;
        }
        return 1;
    }

    bool check_line(string line, string &new_line)
    {
        int cnt = 0, total = 0;
        istringstream ss(line);
        string word;

        while (ss >> word)
        {
            if (cnt == 2)
            {
                if (word.length() != 9 || word[0] == '0' || !is_number(word))
                    return 0;
                cnt = 0;
                new_line += word;
            }
            else
            {
                new_line += word + " ";
            }
            total++;
            cnt++;
        }

        if (total % 3 != 0)
            return 0;
        return 1;
    }

    bool get_commands()
    {
        string current_line;
        bool line = false;

        if (this->file.is_open())
        {
            while (getline(this->file, current_line))
            {
                string new_line;

                if (current_line.empty())
                    line = true;

                if (!line)
                {
                    if (!this->check_line(current_line, new_line))
                        return 0;
                    this->contacts.push_back(new_line);
                    this->total_contacts++;
                }

                if (!current_line.empty() && line)
                {
                    this->commands.push_back(current_line);
                    this->total_commands++;
                }
            }
        }

        if (!line)
            return 0;
        return 1;
    }

    void check_list(ostream &out)
    {
        for (int i = 0; i < this->total_commands; i++)
        {
            int founded = 0;
            for (int j = 0; j < this->total_contacts; j++)
            {
                if (this->contacts[j].find(this->commands[i] + " ") != this->contacts[j].npos)
                {
                    out << this->contacts[j] << endl;
                    founded++;
                }
            }
            out << "-> " << founded << endl;
        }
    }

    ~CRecord()
    {
        this->file.close();
    }
};

bool report(const string &file_name, ostream &out)
{
    bool check = true;
    CRecord *record = new CRecord(file_name);

    check = record->get_commands();
    if (!check)
    {
        delete record;
        return false;
    }

    record->check_list(out);

    delete record;
    return true;
}

#ifndef __PROGTEST__
int main()
{
    ostringstream oss;
    oss.str("");

    assert(report("tests/test1_in.txt", oss) == false);
    oss.str("");

    assert(report("tests/test0_in.txt", oss) == true);
    assert(oss.str() ==
           "John Christescu 258452362\n"
           "John Harmson 861647702\n"
           "-> 2\n"
           "-> 0\n"
           "Josh Dakhov 264112084\n"
           "Dakhov Speechley 865216101\n"
           "-> 2\n"
           "John Harmson 861647702\n"
           "-> 1\n");
    oss.str("");

    assert(report("tests/test3_in.txt", oss) == true);

    return 0;
}
#endif /* __PROGTEST__ */
