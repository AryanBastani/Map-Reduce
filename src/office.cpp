#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <regex>
#include <vector>
#include <bits/stdc++.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <exception>
#include <iostream>
#include <stdexcept>
#include "logger.hpp"
#include "functions.cpp"

const std::string BILL_NAME = "bills.csv";

Logger lg("Office");

typedef struct coefficients
{
    std::string year;
    int month;
    int water_coef;
    int gas_coef;
    int elec_coef;
}Coeffs;

typedef struct wanted_paramerts
{
    int month;
    int total_usage;
    int mean;
    int most_usage_hour;
    int cost;
    int time_diff;
}Bill_parametrs;

typedef struct bill
{
    std::string name;
    std::string resource;
    std::vector<Bill_parametrs*> parameters;
}Bill;

typedef struct day_usage
{
    int hours_usage[NUM_OF_HOURS];
}Month_usage;

class Office
{
    public:
        Office(const char* argv[]);
        int run();

    private:
        int program_pipe[2];
        std::string buildings_path;
        std::vector<std::string> builds_name;
        std::vector<Coeffs*> coeffs_list;
        std::vector<std::string> wanted_resources;
        std::vector<int> wanted_months;
        std::vector<Bill*> bills;

        int recv_names();
        void decode_prog_msg(char buffer[BUFF_SIZE]);
        void load_csv();
        void add_coeffs(std::vector<std::string> parameters);
        void decode_recos_msg(char buffer[BUFF_SIZE]);
        void recv_from_builds();
        void decode_info(char buffer[BUFF_SIZE],
            int resource_index, int name_index);

        void add_to_bills(std::vector<Info*> curr_infos,
            int resource_index, int name_index);
        Bill_parametrs* create_parameter(std::vector<Info*> curr_infos, int index);
        void print_result();
};

Office::Office(const char* argv[])
{
    program_pipe[0] = std::stoi(argv[1]);
    program_pipe[1] = std::stoi(argv[2]);
    buildings_path = argv[3];
}

void Office::decode_prog_msg(char buffer[BUFF_SIZE])
{
    std::vector<std::string> msg = split_line(buffer, ' ');
    
    int i = 0;
    for(i = 0; i < msg.size(); i++)
    {
        if(msg[i] == "$")
            break;
        builds_name.push_back(msg[i]);
    }
    for(int j = i + 1; j < msg.size(); j++)
    {
        wanted_resources.push_back(msg[j]);
    }

}

int Office::recv_names()
{
    lg.info("Receiving building names and wanted resourses from Program");

    close(program_pipe[1]);
    char buffer[BUFF_SIZE];
    int readed_bytes = read(program_pipe[0], buffer, BUFF_SIZE);
    close(program_pipe[0]);
    if(readed_bytes == -1)
    {
        lg.error("Problem with reading from pipe");
        return(1);
    }
    decode_prog_msg(buffer);
    return(0);
}

void Office::add_coeffs(std::vector<std::string> parameters)
{
    Coeffs* coeffs = new Coeffs;
    coeffs->year = parameters[0];
    coeffs->month = atoi(parameters[1].c_str());
    coeffs->water_coef = atoi(parameters[2].c_str());
    coeffs->gas_coef = atoi(parameters[3].c_str());
    coeffs->elec_coef = atoi(parameters[4].c_str());
    coeffs_list.push_back(coeffs);
}

std::vector<std::string> make_coeffs_params(std::string line)
{
    std::stringstream str(line);
    std::vector<std::string> params; 
    while (str.good()) 
    {
        std::string substr;
        std::getline(str, substr, ',');
        params.push_back(substr);
    }
    return(params);
}

void Office::load_csv()
{
    lg.info("Loading csv");

    std::string path = std::string(buildings_path + '/' + BILL_NAME);
    std::ifstream file(path);
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) 
    {
        std::vector<std::string> params = make_coeffs_params(line);
        add_coeffs(params);
    }
}

void Office::decode_recos_msg(char buffer[BUFF_SIZE])
{
    std::vector<std::string> msg = split_line(buffer, ' ');
    
    for(int i = 0; i < msg.size(); i++)
        wanted_resources.push_back(msg[i]);
}

int find_max(std::vector<int> vect)
{
    int max = 0, index;
    for(int i = 0; i < vect.size(); i++)
    {
        if(vect[i] > max)
        {
            max = vect[i];
            index = i;
        }
    }
    return(index);
}

Bill_parametrs* calc_parameter(std::vector<int> usage_hours)
{
    Bill_parametrs* parameter = new Bill_parametrs;
    parameter->most_usage_hour = find_max(usage_hours) + 1;
    int sum = 0, diff = 0;
    for(int z = 0; z < NUM_OF_HOURS; z++)
        sum += usage_hours[z];

    parameter->total_usage = sum;
    parameter->mean = (sum / 30);

    for(int z = 0; z < NUM_OF_HOURS; z++)
        diff += (usage_hours[parameter->most_usage_hour - 1] - parameter->mean); 

    return(parameter); 
}

Bill_parametrs* Office::create_parameter(std::vector<Info*> curr_infos, int index)
{
    std::vector<int> usage_hours(NUM_OF_HOURS, 0);
    for(int j = 0; j < curr_infos.size(); j++)
    {
        if(curr_infos[j]->month == wanted_months[index])
        {
            for(int z = 0; z < NUM_OF_HOURS; z++)
            {
                usage_hours[z] += curr_infos[j]->hours_usage[z];
            }
        }
    }

    Bill_parametrs* parameter = calc_parameter(usage_hours);
    parameter->month = wanted_months[index];

    return(parameter);
}

void Office::add_to_bills(std::vector<Info*> curr_infos,
    int resource_index, int name_index)
{
    Bill* bill = new Bill;
    bill->name = builds_name[name_index];
    bill->resource = wanted_resources[resource_index];

    for(int i = 0; i < wanted_months.size(); i++)
    {
        bill->parameters.push_back(create_parameter(curr_infos, i));
    } 
    bills.push_back(bill);
}

void Office::decode_info(char buffer[BUFF_SIZE], int resource_index, int name_index)
{
    std::vector<std::string> parameters = split_line(buffer, ' ');
    std::vector<Info*> curr_infos;
    for(int i = 0; i < parameters.size(); i += 9)
    {
        Info* info = new Info;
        info->year = parameters[i];
        info->month = std::stoi(parameters[i + 1]);
        info->day = std::stoi(parameters[i + 2]);
        for(int j = 0; j < NUM_OF_HOURS; j++)
        {
            info->hours_usage[j] = std::stoi(parameters[i + 3 + j]);
        }
        curr_infos.push_back(info);
    }
    add_to_bills(curr_infos, resource_index, name_index);
}

void Office:: recv_from_builds()
{
    lg.info("Receiving information of usage from buildings");

    for(int i = 0; i < builds_name.size(); i++)
    {
        std::string fifo = FIFO_PATH + builds_name[i];
        int fd = open(fifo.c_str(), O_RDONLY);
        for(int j = 0; j < wanted_resources.size(); j++)
        {
            char buf[BUFF_SIZE];
            read(fd, buf, BUFF_SIZE);
            decode_info(buf, j, i);
        }
    }
}

void Office::print_result()
{
    for(int i = 0; i < bills.size(); i++)
    {
        cout << bills[i]->name << " for " << bills[i]->resource << ":\n";
        for(int j = 0; j < bills[i]->parameters.size(); j++)
            {
                cout << '\t' << "for month: " << bills[i]->parameters[j]->month;
                cout << "\n\t\t" << "total: " << bills[i]->parameters[j]->total_usage;
            }
    }
}

int Office::run()
{
    load_csv();
    if(recv_names())
        return(1);
    recv_from_builds();
    print_result();
    return(0);
}

int main(int argc, const char* argv[])
{
    if (argc != 4) 
    {
        std::cerr << "Wrong arguments!\n";
        return (1);
    }

    Office office(argv);
    if(office.run())
        return(1);
    
    return(0);
}