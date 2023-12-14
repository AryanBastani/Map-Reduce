#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <regex>
#include <vector>
#include "logger.hpp"
#include "functions.cpp"
#include "color_print.hpp"

using namespace std::string_literals;
namespace fs = std::filesystem;

const char PIPES_PATH[] = "namedpipes";
const char EXE_BUILDING[] = "building.out";
const char EXE_OFFICE[] = "office.out";

Logger lg("Program");

class Program
{
    public:
        Program(std::string path);
        int run();
    private:
        std::string buildings_path;
        std::vector<fs::path> buildings;
        std::vector<std::string> wanted_buildings;
        std::vector<int> wanted_months;
        std::vector<std::string> wanted_resourses;
        int office_pipe[2];

        int make_pipes_path();
        int find_buildings();
        int make_pipes(int (*building_pipes)[2]);
        void get_resourses();
        void get_wanted_builds();
        void get_months();
        int create_builds_procs(int (*building_pipes)[2]);
        bool is_wanted(std::string name);
        std::string code_build_message();
        int exec_building(int index, int (*building_pipes)[2]);
        int create_office_proc();
        std::string clean_build_name(std::string path);
        void wait_for_childs();
        std::string code_office_msg();
        int exec_office();
        std::string code_wanted_resc();
};

Program::Program(std::string path)
{
    buildings_path = path;
}

int Program::make_pipes_path()
{
    lg.info("Making path for pipes");

    if (fs::exists(PIPES_PATH)) 
        fs::remove_all(PIPES_PATH);

    if (mkdir(PIPES_PATH, 0777) == -1) 
    {
        lg.error("Problem with creating pipes directory");
        return (1);
    }
    return(0);
}

std::string Program::clean_build_name(std::string path)
{
    std::string name = path.substr(buildings_path.size() + 1);
    return(name);
}

int Program::find_buildings() 
{
    lg.info("Searching for buildings");

    if (fs::exists(buildings_path) && fs::is_directory(buildings_path)) 
    {
        fs::path directory_path = buildings_path;
        for (const auto& entry : fs::directory_iterator(directory_path)) 
            if (entry.is_directory()) 
                buildings.push_back(clean_build_name(entry.path()));
    }
    else 
    {
        lg.error("Problem with opening directory: "s + buildings_path);
        return (1);
    }
    return (0);
}

int Program::make_pipes(int (*building_pipes)[2])
{
    lg.info("Making pipes");

    for (int i = 0; i < buildings.size(); i++) 
    {
        if (pipe(building_pipes[i]) == -1) 
        {
            lg.error("Problem with creating building pipe");
            return (1);
        }
    }
    if(pipe(office_pipe) == -1)
    {
        lg.error("Problem with creating office pipe");
        return(1);
    }
    return(0);
}

void Program::get_resourses()
{
    std::cout << "Choose the resourses you want to see " << 
        "factors between :\n" << Color::YEL <<
         "-Water\n-Gas\n-Electricity\n" << Color::RST; 
    
    wanted_resourses = get_input();
}

void Program::get_wanted_builds()
{
    std::cout << "Choose the buildings you want to see factors between :\n";
    for(int i = 0; i < buildings.size(); i++)
        std::cout << Color::GRN << '-' << buildings[i].c_str() << '\n' << Color::RST; 

    wanted_buildings = get_input();
}

void Program::get_months()
{
    std::cout << "Choose the month numbers that you want to see factors" <<
       Color::BLU << " start with 1 and end with 12:\n" << Color::RST;
    
    vector<string> months_str = get_input();
    for(int i = 0; i < months_str.size(); i++)
    {
        wanted_months.push_back(stoi(months_str[i]));
    } 
}

bool Program::is_wanted(std::string name)
{
    for(int i = 0; i < wanted_buildings.size(); i++)
    {
        if(name == wanted_buildings[i])
            return(true);
    }
    return(false);
}

std::string Program::code_build_message()
{
    std::string message = "";

    for(int i = 0; i < wanted_months.size(); i++)
    {
        message = message + to_string(wanted_months[i]);
        message = message + ' ';
    }

    message = message + "$ ";

    for(int i = 0; i < wanted_resourses.size(); i++)
    {
        message = message + wanted_resourses[i];
        if(i != wanted_resourses.size() - 1)
            message = message + ' ';
    }
    message = message + '\0';

    return(message);
} 

int Program::exec_building(int index, int (*building_pipes)[2])
{
    char argv[4][256];
    sprintf(argv[0], "%s", buildings[index].c_str());
    sprintf(argv[1], "%d", building_pipes[index][0]);
    sprintf(argv[2], "%d", building_pipes[index][1]);
    std::string s = std::string(buildings_path + '\0');
    sprintf(argv[3], "%s", s.c_str());

    if (execl(EXE_BUILDING, EXE_BUILDING, argv[0], argv[1], argv[2], argv[3], NULL) == -1) 
    {
        lg.error("Problem with execute "s + argv[0]);
        return (1);
    }

    return(0);
}

void Program::wait_for_childs()
{
    for (int i = 0; i <= buildings.size(); i++) 
    {
        int status;
        wait(&status);
    }
}

int Program::create_builds_procs(int (*building_pipes)[2])
{
    lg.info("creating process for buildings");

    std::string msg = code_build_message();
    for (int i = 0; i < buildings.size(); i++) 
    {
        int pid = fork();
        if (pid < 0)
        {
            lg.error("Problem with creating child process for building for position: "s + buildings[i].c_str());
            return(1);
        }
        else if (pid == 0) 
        {
            if(exec_building(i, building_pipes))    
                return(1);
        }
        else if (pid > 0) 
        {
            close(building_pipes[i][0]);
            write(building_pipes[i][1], msg.c_str(), msg.size());
            close(building_pipes[i][1]);
        }
    }
    return(0);
}

std::string Program::code_office_msg()
{
    std::string message = "";

    for(int i = 0; i < buildings.size(); i++)
    {
        message = message + buildings[i].c_str();
        if(i != buildings.size() - 1)
            message = message + ' ';
    }
    message = message + " $ ";
    for(int i = 0; i < wanted_resourses.size(); i++)
    {
        message = message + wanted_resourses[i].c_str();
        if(i != wanted_resourses.size() - 1)
            message = message + ' ';
    }
    message = message + '\0';
    return(message);
}

int Program::exec_office()
{
    char argv[3][256];
    sprintf(argv[0], "%d", office_pipe[0]);
    sprintf(argv[1], "%d", office_pipe[1]);
    std::string s = std::string(buildings_path + '\0');
    sprintf(argv[2], "%s", s.c_str());

    if (execl(EXE_OFFICE, EXE_OFFICE, argv[0], argv[1], argv[2], NULL) == -1) 
    {
        lg.error("Problam with execute office.out");
        return (1);
    }  

    return(0);
}

std::string Program::code_wanted_resc()
{
    std::string message = "";

    for(int i = 0; i < wanted_resourses.size(); i++)
    {
        message = message + wanted_resourses[i].c_str();
        if(i != wanted_resourses.size() - 1)
            message = message + ' ';
    }
    message = message + '\0';
    return(message);
}

int Program::create_office_proc()
{
    lg.info("Creating process for office");

    std::string msg = code_office_msg();
    int pid = fork();
    if (pid < 0)
    {
        lg.error("Problem with creating child process for office");
        return(1);
    }
    else if(pid == 0)
    {
        if (exec_office())
            return(1);
    }
    else if(pid > 0)
    {
        close(office_pipe[0]);
        write(office_pipe[1], msg.c_str(), msg.size());
        close(office_pipe[1]);
    }
    return(0);
}

int Program::run()
{
    if(make_pipes_path())
        return (1);
    if(find_buildings())
        return(1);

    int building_pipes[buildings.size()][2];
    if(make_pipes(building_pipes))
        return(1);

    get_resourses();
    get_wanted_builds();
    get_months();

    if(create_builds_procs(building_pipes))
        return(1);
    sleep(1);
    if(create_office_proc())
        return(1);
    wait_for_childs();
        
    return(0);
}


int main(int argc, const char* argv[]) 
{
   if (argc != 2) 
    {
        std::cerr << "Wrong arguments!\n";
        return (1);
    }
    Program program(argv[1]);
    if(program.run())
        return(1);

    return(0);
}