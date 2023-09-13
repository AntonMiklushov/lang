#pragma once
#include <iostream>
#include <string>
#include <regex>
#include <unordered_map>

enum {INTEGER_ID, FRACTIONAL_ID, STRING_ID, FUNCTION_ID, LIST_ID};

class VarHolder
{
private:

    class variable
    {
    protected:
        std::string content;
    public:
        virtual std::string get_content() const {return content;};
        virtual VarHolder process(VarHolder) const = 0;
        virtual VarHolder* get_collection() const = 0;
        virtual unsigned int get_length() const = 0;
    };

    class integer : public variable
    {
    public:
        integer(std::string content)
        {
            this->content = content;
        }
        VarHolder process(VarHolder v) const override {return VarHolder();}
        VarHolder* get_collection() const override {return new VarHolder();}
        unsigned int get_length() const override {return 0;}
    };

    class fractional : public variable
    {
    public:
        fractional(std::string content) {}
        VarHolder process(VarHolder v) const override {return VarHolder();}
        VarHolder* get_collection() const override {return new VarHolder();}
        unsigned int get_length() const override {return 0;}
    };

    class string : public variable
    {
    public:
        string(std::string content)
        {
            this->content = content;
        }
        VarHolder process(VarHolder v) const override {return VarHolder();}
        VarHolder* get_collection() const override {return new VarHolder();}
        unsigned int get_length() const override {return 0;}
    };

    class function : public variable
    {
    private:
        unsigned int body_index;
        std::unordered_map<std::string, VarHolder> input_variables;
        unsigned int input_variable_count;
        VarHolder (*func)(VarHolder);
    public:
        function(variable* v)
        {
            content = v->get_content();
            std::smatch match;
            std::regex_match(content, match, std::regex(R"(\((\D+)\)\<(.+)\>)"));
            this->content = std::string("<function(") + match[1].str()
            + std::string(")<body:") + match[2].str() + std::string(">>");
        }
        function(std::string content) // "(inputs)<bodyname>"
        {
            std::smatch match;
            std::regex_match(content, match, std::regex(R"(\((.*)\)\s*\<body\:(.+)\>)"));
            this->content = std::string("<function(") + match[1].str()
            + std::string(")<body:") + match[2].str() + std::string(">>");
        }
        VarHolder process(VarHolder vars) const override
        {
            return VarHolder();
        }
        std::string get_content() const override
        {
            return content;
        }
        VarHolder* get_collection() const override {return new VarHolder();}
        unsigned int get_length() const override {return 0;}
    };

    class list : public variable
    {
    private:
        unsigned int length;
        VarHolder* collection;
    public:
        list(VarHolder* collection, unsigned int length)
        {
            this->collection = collection;
            this->length = length;
            content = "<list:[";
            if (length > 0)
            {
                for (int i = 0; i < length - 1; i++) content += (collection[i].get_var()->get_content() + ", ");
                content += collection[length - 1].get_var()->get_content();
            }
            content += "]>";
        }
        std::string get_content() const override
        {
            return content;
        }

        VarHolder process(VarHolder v) const override {return VarHolder();}

        unsigned int get_length() const override
        {
            return length;
        }

        VarHolder* get_collection() const override
        {
            return collection;
        }
    };

    variable* var;
    unsigned int type;
public:
    static std::unordered_map<std::string, unsigned int> types;

    VarHolder(variable* var, unsigned int type)
    {
        this->var = var;
        this->type = type;
    }

    VarHolder(std::string type, std::string content)
    {
        variable* ptr;
        this->type = types.at(type);
        switch (types.at(type))
        {
            case INTEGER_ID:
                ptr = new integer(content);
                break;
            case FRACTIONAL_ID:
                ptr = new fractional(content);
                break;
            case STRING_ID:
                ptr = new string(content);
                break;
            case FUNCTION_ID:
                ptr = new function(content);
                break;
        }
        var = ptr;
    }

    VarHolder(unsigned int type, std::string content)
    {
        variable* ptr;
        this->type = type;
        switch (type)
        {
            case INTEGER_ID:
                ptr = new integer(content);
                break;
            case FRACTIONAL_ID:
                ptr = new fractional(content);
                break;
            case STRING_ID:
                ptr = new string(content);
                break;
            case FUNCTION_ID:
                ptr = new function(content);
                break;
        }
        var = ptr;
    }

    // spcial constructor for lists
    VarHolder(VarHolder* list, unsigned int length)
    {
        var = new VarHolder::list(list, length);
        this->type = LIST_ID;
    }

    VarHolder() {}

    variable* get_var()
    {
        return var;
    }

    unsigned int get_type()
    {
        return this->type;
    }

    VarHolder operator +(const VarHolder& v)
    {
        switch (this->type)
        {
        case STRING_ID:
            switch (v.type)
            {
            default:
                string_conversion:
                return VarHolder(
                    new string(this->var->get_content() + v.var->get_content()),
                    STRING_ID
                    );
            }
            break;
        case INTEGER_ID:
            switch (v.type)
            {
            case INTEGER_ID:
                return VarHolder(
                    new integer(std::to_string(std::stoi(this->var->get_content()) + std::stoi(v.var->get_content()))),
                    INTEGER_ID
                );
            case STRING_ID:
                goto string_conversion;
            }
            break;
        }
        return VarHolder();
    }

    VarHolder operator ==(const VarHolder& v)
    {
        if (this->var->get_content() == v.var->get_content()) return VarHolder(INTEGER_ID, "1");
        return VarHolder(INTEGER_ID, "0");
    }
};

std::unordered_map<std::string, unsigned int> VarHolder::types({
    {"integer", INTEGER_ID},
    {"fractional", FRACTIONAL_ID},
    {"string", STRING_ID},
    {"function", FUNCTION_ID},
    {"list", LIST_ID}});