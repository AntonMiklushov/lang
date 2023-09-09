#pragma once
#include <regex>
#include <map>
#include <functional>
#include "VarHolder.h"
#include "prebuild_functions.h"
#include <algorithm>

typedef unsigned int uint;

namespace lang
{
    const auto body_regex = std::regex(R"(\{[^\{\}]*\})");
    const auto func_regex = std::regex(R"(func\s+(\(.*\)\s*\<body\:\d+\>))");
    const auto integer_regex = std::regex(R"((\d+)[^>])");
    const auto fractional_regex = std::regex();
    const auto string_regex = std::regex(R"(\"([^\"]*)\")");
    const auto naming_regex = std::regex(R"(\s*name\s+(\w+[^\s]*)\s+(.*);)");
    const auto lable_regex = std::regex(R"(lable\s+(\w+\d*))");
    const auto var_regex = std::regex(R"(\<variable\:(\d+)\>)");
    const auto list_regex = std::regex();
    const auto return_regex = std::regex(R"(return\s+<variable\:(\d+)>)");
    const auto ternary_regex = std::regex(R"(\((.*)\)\s*\?\s*<body\:(\d)>\s*\:\s*<body\:(\d)>)");
    const uint ADDITION_ID = 0;
    const uint SUBSTRACTION_ID = 1;
    const uint MULTIPLICATION_ID = 2;
    const uint DIVISION_ID = 3;
    const uint ISEQUAL_ID = 4;

    std::map<std::string, std::regex> type_regex_map = {
        {"string", string_regex},
        {"integer", integer_regex},
        {"fractional", fractional_regex},
        {"function", func_regex},
        {"list", list_regex}
    };
    
    // Order DOES matter
    std::map<uint, std::regex> operations = {
        {ISEQUAL_ID, std::regex(R"((<variable:\d+>)\s*\==\s*(<variable:\d+>))")},
        {MULTIPLICATION_ID, std::regex(R"((<variable:\d+>)\s*\*\s*(<variable:\d+>))")},
        {DIVISION_ID, std::regex(R"((<variable:\d+>)\s*\/\s*(<variable:\d+>))")},
        {ADDITION_ID, std::regex(R"((<variable:\d+>)\s*\+\s*(<variable:\d+>))")},
        {SUBSTRACTION_ID, std::regex(R"((<variable:\d+>)\s*\-\s*(<variable:\d+>))")}
    };


class code_block
{
private:
    std::string code;
    std::vector<std::string> expressions;
    std::map<uint, code_block> subblocks;
    uint subblock_count = 0;
    uint floating_variables_count = 0;
    std::map<uint, VarHolder> variables;
    std::map<std::string, uint> names;
    std::map<std::string, uint> lables;
    bool returned = false;
    std::string return_value;

    bool match_blocks(std::string input, uint* dest)
    {
        std::string code_buffer = input;
        std::smatch match;
        std::string match_buffer;
        uint length_buffer;
        refind:
        if (std::regex_search(code_buffer, match, lang::body_regex))
        {
            match_buffer = match.str();
            uint bob = 0, bcb = 0, fob = 0, fcb = 0; // behind openning bracket, behind closing bracket, forward...
            for (int i = 0; i < match.position(); i++)
                if (code_buffer[i] == char('{')) bob =+ 1;
                else if (code_buffer[i] == char('}')) bcb += 1;
            for (int i = code_buffer.length(); i > match.position() + match.length(); i--)
                if (code_buffer[i] == char('{')) fob =+ 1;
                else if (code_buffer[i] == char('}')) fcb += 1;
            if ((bob != bcb) || (fob != fcb))
            {
                code_buffer.replace(match.position(), match.length(), std::string(match.length(), '_'));
                goto refind;
            }
            dest[0] = match.position();
            dest[1] = match.length();
            return true;
        }
        return false;
    }

    bool match_expression(std::string input, std::string* match)
    {
        for (int i = 0; i < input.length(); i++)
            if (input[i] == char(';')) 
            {
                *match = input.substr(0, i + 1);
                return true;
            }
        return false;
    }

    uint get_index(std::string s)
    {
        std::smatch match;
        std::regex_search(s, match, std::regex(R"(\d+)"));
        return std::stoi(match.str());
    }

    std::string find_border_bracket(std::string str)
    {
        std::smatch match;
        uint o = 0, c = 0;
        std::string buffer = str;
        while (std::regex_search(buffer, match, std::regex(R"(\()")))
        {
            o += 1;
            buffer.replace(match.position(), 1, "");
        }
        while (std::regex_search(buffer, match, std::regex(R"(\))")))
        {
            c += 1;
            buffer.replace(match.position(), 1, "");
        }
        for (int i = 0; i < o - c; i++) 
        {
            std::regex_search(str, match, std::regex(R"(\()"));
            str.replace(match.position(), 1, "");
        }
        return str;
    }

public:

    std::string get_code()
    {
        return code;
    }

    code_block(std::string code)
    {
        this->code = code.substr(1, code.length() - 2);
    }

    code_block(std::string code, code_block father)
    {
        this->code = code.substr(1, code.length() - 2);
        for (auto pair: father.names)
        {
            this->names.insert({pair.first,this->floating_variables_count});
            this->register_new_variable(father.variables.at(pair.second));
        }
    }
    
    void find_subblocks()
    {
        std::string body_text("<body:x>");
        std::string code_buffer(this->code);
        std::string match;
        uint out[] = {0, 0};
        while(match_blocks(code_buffer, out))
        {
            match = code_buffer.substr(out[0], out[1]);
            subblocks.insert({subblock_count, code_block(match, *this)});
            body_text.replace(6, body_text.length() - 7, std::to_string(subblock_count));
            code_buffer.replace(out[0], out[1], body_text);
            subblock_count++;
        }
        code = code_buffer;
    }

    void split_expressions()
    {
        std::string match;
        std::string buffer = code;
        while(match_expression(buffer, &match))
        {
            expressions.push_back(match);
            buffer.replace(0, match.length(), "");
        }
    }

    void resplit_expressions()
    {
        std::string match;
        std::vector<std::string> collection;
        for (auto it = begin (expressions); it != end (expressions); ++it)
        {
            std::string buffer = it->data();
            while(match_expression(buffer, &match))
            {
                collection.push_back(match);
                buffer.replace(0, match.length(), "");
            }
        }
        expressions = collection;
    }

    void find_floating_variables()
    {
        std::smatch match;
        std::string variable_text;
        uint l;
        for (std::string &exp: expressions)
        {
            for (const auto &pair: VarHolder::types)
            {
                std::string type = pair.first;
                while (std::regex_search(exp, match, type_regex_map.at(type)))
                {
                    VarHolder v = VarHolder(type, match[1]);
                    variable_text = register_new_variable(v);
                    l = match.length() - (pair.second == INTEGER_ID ? 1 : 0);
                    exp.replace(match.position(), l, variable_text);
                }
            }
        }
    }

    void find_name(std::string exp)
    {
        std::smatch match;
        std::string b;
        if (std::regex_search(exp, match, naming_regex))
        {
            b = put_variables(match[2].str());
            evaluate(b);
            std::cout << "match: " << match.str() << '\n';
            names.insert({match[1].str(), get_index(b)});
        }
    }

    void reveal_expressions()
    {
        for (std::string exp: expressions) std::cout << exp << '\n';
    }

    void reveal_lables()
    {
        for (auto &pair: lables)
        {
            std::cout << pair.first << " at " << pair.second << '\n';
        }
    }

    void reveal_variables()
    {
        for(auto pair=variables.begin();pair!=variables.end();pair++) {
            uint index = pair->first;
            VarHolder v = pair->second;
            std::string content = v.get_var()->get_content();
            std::cout << pair->first << ":" << content << '\n';
        }
    }

    void reveal_names()
    {
        for (auto &pair: names) std::cout << pair.first << ":" << variables[pair.second].get_var()->get_content() << '\n';
    }

    std::pair<VarHolder*, uint> get_vars(std::string vars)
    {
        VarHolder* collection;
        uint count = 0;
        std::smatch match;
        std::string buffer = put_variables(vars); // getting rid of names, replacing them with coresponding variables
        while (std::regex_search(buffer, match, var_regex))
        {
            collection = put_in_array(collection, count, variables.at(std::stoi(match[1].str())));
            buffer.replace(match.position(), match.length(), "");
            count++;
        }
        return std::pair<VarHolder*, uint>({collection, count});
    }

    VarHolder* put_in_array(VarHolder* a, uint len, VarHolder b)
    {
        VarHolder* buffer = new VarHolder[len + 1];
        for (int i = 0; i < len; i++) buffer[i] = a[i];
        buffer[len] = b;
        return buffer;
    }

    std::string put_variables(std::string vars)
    {
        auto buffer = " " + vars + " ";
        std::smatch match;
        std::string re_template = R"(\Wx\W)";
        std::string var_text = "<variable:x>";
        uint template_length = re_template.length() - 1;
        for (auto &pair: names)
        {
            re_template.replace(2, re_template.length() - template_length, pair.first);
            while (std::regex_search(buffer, match, std::regex(re_template)))
            {
                var_text.replace(10, var_text.length() - 11, std::to_string(names.at(pair.first)));
                buffer.replace(match.position(), match.length(), var_text);
            }
        }
        return buffer;
    }

    void execute_funcitons(std::string exp)
    {
        std::regex re;
        std::smatch match;
        std::string re_template = R"(x\(([^\(\)]*)\))";
        std::string arguments;
        std::pair<VarHolder*, uint> list;
        uint template_length = re_template.length() - 1;
        for (auto &pair: prebuild_functions)
        {
            re_template.replace(0, re_template.length() - template_length, pair.first);
            if (std::regex_search(exp, match, std::regex(re_template)))
            {
                auto func = pair.second;
                arguments = put_variables(match[1].str());
                evaluate(arguments);
                list = get_vars(arguments);
                VarHolder out = func(VarHolder(list.first, list.second));
                exp.replace(match.position(), match.length(), register_new_variable(out));
                execute_funcitons(exp);
            }
        }
    }

    std::string register_new_variable(VarHolder v)
    {
        std::string var_text = "<variable:x>";
        variables.insert({floating_variables_count, v});
        var_text.replace(10, var_text.length() - 11, std::to_string(floating_variables_count));
        floating_variables_count++;
        return var_text;
    }

    bool check_for_return(std::string exp)
    {
        std::smatch match;
        if (std::regex_search(exp, match, return_regex))
        {
            set_return(variables.at(std::stoi(match[1].str())));
            return true;
        }
        return false;
    }

    void find_lables()
    {
        std::smatch match;
        std::string exp;
        for (uint i = 0; i < expressions.size(); i++)
        {
            exp = expressions[i];
            if (std::regex_search(exp, match, lable_regex))
            {
                lables.insert({match[1], i});
            }
        }
    }

    void execute_ternaries(std::string &exp)
    {
        std::smatch match;
        std::string clause;
        std::pair<VarHolder*, uint> buffer;
        VarHolder v;
        uint p, cl;
        bool found = false;
        while (std::regex_search(exp, match, ternary_regex))
        {
            found = true;
            clause = find_border_bracket(match[1].str());
            cl = clause.length();
            clause = put_variables(clause);
            execute_funcitons(clause);
            evaluate(clause);
            v = get_vars(clause).first[0];
            p = match[1].length() - cl;
            if (v.get_var()->get_content() != std::string("0"))
                exp.replace(match.position() + p, match.length() - p, subblocks.at(std::stoi(match[2])).get_code());
            else
                exp.replace(match.position() + p, match.length() - p, subblocks.at(std::stoi(match[3])).get_code());
        }
        if (found)
        {
            recompile();
        }
    }

    void check_for_goto()
    {}

    void recompile()
    {
        find_subblocks();
        resplit_expressions();
        find_lables();
        find_floating_variables();
    }

    std::string execute()
    {
        for (int i = 0; i < expressions.size(); i++)
        {
            std::string &exp = expressions[i];
            if (check_for_return(exp)) return return_value;
            execute_ternaries(exp);
            execute_funcitons(exp);
            std::cout << "exp" << exp << '\n';
            find_name(exp);
        }
        return "";
    }

    void set_return(VarHolder &v)
    {
        this->returned = true;
        this->return_value = v.get_var()->get_content();
    }

    void evaluate(std::string &exp)
    {
        std::smatch match;
        VarHolder* vars;
        for (auto &pair: operations)
        {
            while (std::regex_search(exp, match, pair.second))
            {
                auto b = get_vars(match[1].str() + match[2].str());
                vars = b.first;
                switch (pair.first)
                {
                case ADDITION_ID:
                    exp.replace(match.position(), match.length(), register_new_variable(vars[0] + vars[1]));
                    break;
                case SUBSTRACTION_ID:
                    break;
                case MULTIPLICATION_ID:
                    break;
                case DIVISION_ID:
                    break;
                case ISEQUAL_ID:
                    exp.replace(match.position(), match.length(), register_new_variable(vars[0] == vars[1]));
                    break;
                }
            }
        }
    }
};

}