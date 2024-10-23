#include "logger.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>

#include "color_print.hpp"

Logger::Logger(std::string program) : program_name(std::move(program)) {}

void Logger::error(const std::string& msg) 
{
    std::cerr << Color::RED << "[ERR:" << program_name << "] "
              << Color::RST << msg << '\n';
}

void Logger::warning(const std::string& msg) 
{
    std::cout << Color::YEL << "[WRN:" << program_name << "] "
              << Color::RST << msg << '\n';
}

void Logger::info(const std::string& msg) 
{
    std::cout << Color::BLU << "[INF:" << program_name << "] "
              << Color::RST << msg << '\n';
}

void Logger::perrno() 
{
    error(strerror(errno));
}
