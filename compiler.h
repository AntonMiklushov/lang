#pragma once
#include <iostream>
#include <unordered_map>
#include "lang.h"
#include <chrono>

#define umap unordered_map

class compiler
{
private:
    lang::code_block* main_block;

public:
    lang::code_block* get_main_block()
    {
        return main_block;
    }

    compiler(std::string code, bool debug)
    {
        auto start = std::chrono::high_resolution_clock::now();
        main_block = new lang::code_block(code);
        main_block->find_subblocks();
        if (debug) std::cout << "Subblocks were found" << '\n';
        main_block->split_expressions();
        if (debug) std::cout << "Expressions were splited" << '\n';
        main_block->find_floating_variables();
        if (debug) std::cout << "Floating variables found" << '\n';
        main_block->find_lables();
        if (debug) std::cout << "Lables were found" << '\n';
        std::string output = main_block->execute();
        if (debug) std::cout << '\n' << "Execution completed" << '\n';
        auto stop = std::chrono::high_resolution_clock::now();
        if (debug)
        {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
            std::cout << "Execution duration: " << duration.count() << " milliseconds";
        }
        std::cout << '\n' << "Output: " << output << '\n';
    }
};
