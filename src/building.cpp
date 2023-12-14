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

const int NUM_OF_RESOURSES = 3;
const int WATER_INDEX = 0;
const int GAS_INDEX = 1;
const int ELEC_INDEX = 2;

const char EXE_RESOURCE[] = "resource.out";

Logger lg("Building");

class Building
{
    public:
        Building(const char* argv[]);
        int run();

    private:
        int program_pipe[2];
        std::string name;
        std::string buildings_path;
        std::vector<int> wanted_month;
        std::vector<std::string> wanted_resourses;
        int rd_resourse_pipes[NUM_OF_RESOURSES][2];
        int wr_resourse_pipes[NUM_OF_RESOURSES][2];
        std::vector<Info*> wanted_water;
        std::vector<Info*> wanted_gas;
        std::vector<Info*> wanted_elec;
        std::string office_fifo;

        int recv_from_prog();
        void decode_prog_msg(char buffer[BUFF_SIZE]);
        int create_resource_procs();
        int create_resource_pipes();
        std::string code_resource_msg();
        int exec_resource(int index);
        void wait_for_rescs();
        int parent_process(std::string msg, int index);
        void decode_resource_msg(char buffer[BUFF_SIZE], int index);
        void create_office_fifo();
        void send_msg_to_office();
};

Building::Building(const char* argv[])
{
    name = argv[1];
    program_pipe[0] = std::stoi(argv[2]);
    program_pipe[1] = std::stoi(argv[3]);
    buildings_path = argv[4];
}

void Building::decode_prog_msg(char buffer[BUFF_SIZE])
{
    std::vector<std::string> msg = split_line(buffer, ' ');
    
    int i = 0;
    for(i = 0; i < msg.size(); i++)
    {
        if(msg[i] == "$")
            break;
        wanted_month.push_back(stoi(msg[i]));
    }
    for(int j = i + 1; j < msg.size(); j++)
    {
        wanted_resourses.push_back(msg[j]);
    }
}

int Building::recv_from_prog()
{
    lg.info("Receiving wanted resources from Program");

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

std::string Building::code_resource_msg()
{
    std::string msg = "";
    for(int i = 0; i < wanted_month.size(); i++)
    {
        msg = msg + to_string(wanted_month[i]);
        if(i != wanted_month.size() - 1)
            msg = msg + " ";
    }
    msg = msg + '\0';
    return(msg);
}

std::string choose_name(int index)
{
    std::string name;
    if(index == WATER_INDEX)
        name = WATER_NAME;
    else if(index == GAS_INDEX)
        name = GAS_NAME;
    else if(index == ELEC_INDEX)
        name = ELEC_NAME;
    return(name);
}

int Building::exec_resource(int index)
{
    std::string resource_name = choose_name(index);

    std::string prev_path = std::string(buildings_path + "/" + name);
    char argv[6][256];
    sprintf(argv[0], "%s", resource_name.c_str());
    sprintf(argv[1], "%s", prev_path.c_str());
    sprintf(argv[2], "%d", rd_resourse_pipes[index][0]);
    sprintf(argv[3], "%d", rd_resourse_pipes[index][1]);
    sprintf(argv[4], "%d", wr_resourse_pipes[index][0]);
    sprintf(argv[5], "%d", wr_resourse_pipes[index][1]);

    if (execl(EXE_RESOURCE, EXE_RESOURCE, argv[0], argv[1], argv[2],
        argv[3], argv[4], argv[5], NULL) == -1) 
    {
        lg.error("Problem with execute "s + argv[0]);
        return (1);
    }


    return(0);
}

void Building::wait_for_rescs()
{
    for (int i = 0; i < NUM_OF_RESOURSES; i++) 
    {
        int status;
        wait(&status);
    }
}

void Building::decode_resource_msg(char buffer[BUFF_SIZE], int index)
{
    std::vector<std::string> parameters = split_line(buffer, ' ');
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
        if(index == WATER_INDEX)
            wanted_water.push_back(info);
        else if(index == GAS_INDEX)
            wanted_gas.push_back(info);
        else if(index == ELEC_INDEX)
            wanted_elec.push_back(info);
    }
}

int Building::parent_process(std::string msg, int index)
{
    close(wr_resourse_pipes[index][0]);
    close(rd_resourse_pipes[index][1]);
    write(wr_resourse_pipes[index][1], msg.c_str(), msg.size());
    close(wr_resourse_pipes[index][1]);
    char buffer[BUFF_SIZE];
    int readed_bytes = read(rd_resourse_pipes[index][0], buffer, BUFF_SIZE);
    close(rd_resourse_pipes[index][0]);
    if(readed_bytes == -1)
    {
        lg.error("Problem with reading from pipe");
        return(1);
    }   
    decode_resource_msg(buffer, index);
    return(0);
}

int Building::create_resource_procs()
{
    lg.info("Creating process for resources");

    std::string msg = code_resource_msg();
    for (int i = 0; i < NUM_OF_RESOURSES; i++) 
    {
        int pid = fork();
        if (pid < 0)
        {
            lg.error("Problem with creating child process for resource");
            return(1);
        }
        else if (pid == 0)
        { 
            if(exec_resource(i))    
                return(1);
        }
        else if (pid > 0)
        {
            if(parent_process(msg, i))
                return(1);
        }
    }
    return(0);
}

int Building::create_resource_pipes()
{
    lg.info("Creating pipes for resources");

    for (int i = 0; i < NUM_OF_RESOURSES; i++) 
    {
        if (pipe(rd_resourse_pipes[i]) == -1 || pipe(wr_resourse_pipes[i]) == -1) 
        {
            lg.error("Problem with creating resource pipe");
            return (1);
        }
    }
    return(0);
}

void Building::create_office_fifo()
{
    lg.info("Creating fifo for office");

    office_fifo = FIFO_PATH + name;
    mkfifo(office_fifo.c_str(), 0666);
}

void Building::send_msg_to_office()
{
    lg.info("Sending information of usage to Office");

    std::string msg = "";
    for(int i = 0; i < wanted_resourses.size(); i++)
    {
        msg = "";
        if(wanted_resourses[i] == WATER_NAME)
            msg = code_info(ALL, wanted_water);
        else if(wanted_resourses[i] == GAS_NAME)
            msg = code_info(ALL, wanted_gas);
        else if(wanted_resourses[i] == ELEC_NAME)
            msg = code_info(ALL, wanted_elec);

        int fd = open(office_fifo.c_str(), O_WRONLY);
        write(fd, msg.c_str(), msg.size());
        close(fd);
    }
}

int Building::run()
{
    if(recv_from_prog())
        return(1);
    if(create_resource_pipes())
        return(1);
    if(create_resource_procs())
        return(1);
    create_office_fifo();
    send_msg_to_office();

    wait_for_rescs();

    return(0);
}

int main(int argc, const char* argv[]) 
{
    if (argc != 5) 
    {
        lg.error("Invalid number of arguments");
        return (1);
    }
    Building building(argv);
    if(building.run())
        return(1);
    return(0);
}