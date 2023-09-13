#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "compiler.h"

int main(int argc, char* argv[])
{
    std::ifstream file;
    std::string line;
    std::vector<std::string> lines;
    if (argc == 1)
        file.open("code.txt", std::ios::in);
    else
        file.open(argv[1], std::ios::in);
    if (file.is_open())
    {
        while(getline(file, line)) lines.push_back(line);
    }
    file.close();
    std::ostringstream comp_string_stream;
    comp_string_stream << "{";
    for(int i = 0; i < lines.size(); i++) comp_string_stream << lines[i];
    comp_string_stream << "}";
    const std::string code = comp_string_stream.str();
    compiler* com = new compiler(code, false);
    return 0;
}