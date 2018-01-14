#include "CommandLine.hpp"

#include <stdexcept>
#include <utility>
#include <cassert>

CommandLine::CommandLine() : long_names(256)
{}
CommandLine::~CommandLine()
{}



void CommandLine::addFlag(char short_key, std::string&& long_key, bool default_value)
{
    assert(this->long_names[short_key].empty() == true);
    if(short_key != '\0')
        this->long_names[short_key] = long_key;
    this->flags.insert(std::make_pair(std::move(long_key), default_value));
}
void CommandLine::addFlag(char short_key, const std::string& long_key, bool default_value)
{
    assert(this->long_names[short_key].empty() == true);
    if(short_key != '\0')
        this->long_names[short_key] = long_key;
    this->flags.insert(std::make_pair(long_key, default_value));
}
void CommandLine::addOption(char short_key, std::string&& long_key, std::string&& default_value)
{
    assert(this->long_names[short_key].empty() == true);
    if(short_key != '\0')
        this->long_names[short_key] = long_key;
    this->options.insert(std::make_pair(std::move(long_key), std::move(default_value)));
}
void CommandLine::addOption(char short_key, const std::string& long_key, std::string&& default_value)
{
    assert(this->long_names[short_key].empty() == true);
    if(short_key != '\0')
        this->long_names[short_key] = long_key;
    this->options.insert(std::make_pair(long_key, std::move(default_value)));
}



bool CommandLine::findFlag(char key) const
{
    return this->findFlag(this->long_names[key]);
}
bool CommandLine::findFlag(const std::string& key) const
{
    auto it = this->flags.find(key);
    if(it == this->flags.end())
        throw std::out_of_range(std::string("CommandLine::findFlag: could not find ") + key);

    return (*it).second;
}
std::string CommandLine::findOption(char key) const
{
    return this->findOption(this->long_names[key]);
}
std::string CommandLine::findOption(const std::string& key) const
{
    auto it = this->options.find(key);
    if(it == this->options.end())
        throw std::out_of_range(std::string("CommandLine::findOption: could not find ") + key);

    return (*it).second;
}



const std::vector<std::string>& CommandLine::getArguments() const
{
    return this->arguments;
}



void CommandLine::parse(int argc, char** argv)
{
    if(argc == 1)
        return;

    for(int i = 1; i < argc; ++i)
    {
        char* it = argv[i];

        if((*it) != '-' || (*it) == '\0')
        {
            this->arguments.push_back(argv[i]);
        }
        else
        {
            ++it;
            std::string key;

            if((*it) != '-')
            {
                key = this->long_names[*it];
                ++it;
            }
            else
            {
                ++it;
                while((*it) != '\0' && (*it) != '=')
                {
                    key.push_back(*it);
                    ++it;
                }
            }

            if(this->flags.find(key) != this->flags.end())
            {
                if((*it) != '\0')
                    throw std::runtime_error(std::string("flags can't have values ") + argv[i]);
                this->flags[key] = true;
            }
            else if(this->options.find(key) != this->options.end())
            {
                if((*it) == '=')
                    ++it;
                else if((*it) == '\0')
                {
                    ++i;
                    if(i == argc)
                        throw std::runtime_error(std::string("expected option value ") + argv[i - 1]);
                    it = argv[i];
                }

                this->options[key] = it;
            }
            else
                throw std::runtime_error(std::string("unknown argument ") + argv[i]);
        }
    }
}
