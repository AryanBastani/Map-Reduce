#include <iostream>
#include <string>
#include <vector>

using namespace std;

const int BUFF_SIZE = 1024;
const int NUM_OF_HOURS = 6;
const std::string FIFO_PATH = "/fifos/";
const int ALL = -1;

const std::string WATER_NAME = "Water";
const std::string GAS_NAME = "Gas";
const std::string ELEC_NAME = "Electricity";


typedef struct info
{
    int day;
    int month;
    string year;
    int hours_usage[NUM_OF_HOURS];
}Info;

vector<string> split_line(string line,char separator)
{
    vector<string> spiliteds;
    string spred_word;
    for(int char_id=0;char_id<(int)line.size();char_id++)
    {
        if(line[char_id]==separator)
        {
            spiliteds.push_back(spred_word);
            spred_word="";
        }
        else
            spred_word+=line[char_id];
    }
    spiliteds.push_back(spred_word);
    return(spiliteds);
}

std::string code_info(int the_month, std::vector<Info*> info)
{
    std::string msg = "";
    for(int i = 0; i < info.size(); i++)
    {
        if(info[i]->month == the_month || the_month == ALL)
        {
            msg = msg + info[i]->year + ' ';
            msg = msg + to_string(info[i]->month) + ' ';
            msg = msg + to_string(info[i]->day) + ' ';
            for(int j = 0; j < NUM_OF_HOURS; j++)
                msg = msg + to_string(info[i]->hours_usage[j]) + ' ';
        }
    }
    msg[msg.size() - 1] = '\0';
    return(msg);
}

vector<string> get_input()
{
    vector<string> input;
    string line;
    getline(cin, line);
    input = split_line(line, ' ');
    return(input);
}