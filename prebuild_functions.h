#pragma once
#include "VarHolder.h"

VarHolder input(VarHolder v)
{
    if (v.get_var()->get_length() != 0)
        std::cout << v.get_var()->get_collection()[0].get_var()->get_content();
    std::string s;
    std::cin >> s;
    return VarHolder(STRING_ID, s);
}

VarHolder print(VarHolder v)
{
    for (int i = 0; i < v.get_var()->get_length(); i++)
        std::cout << v.get_var()->get_collection()[i].get_var()->get_content();
    std::cout << '\n';
    return v;
}

VarHolder integer(VarHolder v)
{
    return VarHolder(INTEGER_ID, v.get_var()->get_collection()[0].get_var()->get_content());
}

std::unordered_map<std::string, std::function<VarHolder(VarHolder)>> prebuild_functions = {
    {"input", input},
    {"print", print},
    {"integer", integer}};