#include "Util.hpp"
#include <cstdlib>
#include <iostream>

void execute_command(const char * command)
{
    if(std::system(command) != 0) {
        std::cout << "Command execution failed" << std::endl;
    }
}
