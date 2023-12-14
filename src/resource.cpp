#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <regex>
#include <vector>
#include <fstream>
#include "logger.hpp"
#include "functions.cpp"

Logger lg("Resource");

class Resource
{
    public:
        Resource(const char* argv[]);
        int run();

    private:
        std::string name;
        std::string perv_path;
        std::vector<Info*> info_list;
        std::vector<int> wanted_month;
        int rd_resourse_pipes[2];
        int wr_resourse_pipes[2];

        void load_csv();
        void add_info(std::vector<std::string> inf_params);
        int recv_from_build();
        void decode_build_msg(char buffer[BUFF_SIZE]);
        void send_to_build();
};

Resource::Resource(const char* argv[])
{
    name = argv[1];
    perv_path = argv[2];
    wr_resourse_pipes[0] = std::stoi(argv[3]);
    wr_resourse_pipes[1] = std::stoi(argv[4]);
    rd_resourse_pipes[0] = std::stoi(argv[5]);
    rd_resourse_pipes[1] = std::stoi(argv[6]);
}

void trim(std::string& str) 
{
    str.erase(str.begin(), std::find_if(str.begin(),
        str.end(), [](unsigned char ch)
        { return !std::isspace(ch); }));
    str.erase(std::find_if(str.rbegin(), str.rend(),
        [](unsigned char ch) { return !std::isspace(ch);
        }).base(), str.end());
}

void Resource::add_info(std::vector<std::string> inf_params)
{
    trim(inf_params[0]);
    trim(inf_params[1]);
    trim(inf_params[2]);

    Info* info = new Info;
    info->year = inf_params[0];
    info->month = atoi(inf_params[1].c_str());
    info->day = atoi(inf_params[2].c_str());

    for (long unsigned int i = 0; i < NUM_OF_HOURS; i++)
        info->hours_usage[i] = atoi(inf_params[i + 3].c_str());

    info_list.push_back(info);
}

void Resource::load_csv() 
{
    lg.info("Loading csv");

    std::string path = std::string(perv_path + '/' + name + ".csv");
    std::ifstream file(path);
    std::string line;

    std::getline(file, line);
    while (std::getline(file, line)) 
    {
        std::vector<std::string> line_vect = split_line(line, ',');

        add_info(line_vect);
    }
}

void Resource::decode_build_msg(char buffer[BUFF_SIZE])
{
    std::vector<std::string> msg = split_line(buffer, ' ');
    for(int i = 0; i < msg.size(); i++)
    {
        wanted_month.push_back(stoi(msg[i]));
    }
}

int Resource::recv_from_build()
{
    lg.info("Receiving wanted months from buildings");

    close(rd_resourse_pipes[1]);
    close(wr_resourse_pipes[0]);

    char buffer[BUFF_SIZE];
    int readed_bytes = read(rd_resourse_pipes[0], buffer, BUFF_SIZE);
    close(rd_resourse_pipes[0]);
    if(readed_bytes == -1)
    {
        lg.error("Problem with reading from pipe");
        return(1);
    }

    decode_build_msg(buffer);
    return(0);
}

void Resource::send_to_build()
{
    lg.info("Sending information of usage to buildings");

    for(int i = 0; i < wanted_month.size(); i++)
    {
        std::string msg = code_info(wanted_month[i], info_list);
        write(wr_resourse_pipes[1], msg.c_str(), msg.size());
        close(wr_resourse_pipes[1]);
    }
}

int Resource::run()
{
    load_csv();
    if(recv_from_build())
        return(1);
    send_to_build();
    return(0);
}

int main(int argc, const char* argv[])
{
    if (argc != 7) 
    {
        lg.error("Invalid number of arguments");
        return (1);
    }
    Resource resource(argv);
    if(resource.run())
        return(1);
    return(0);
}