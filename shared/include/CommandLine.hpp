#ifndef INCLUDED_COMMANDLINE_HPP
#define INCLUDED_COMMANDLINE_HPP

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

class CommandLine
{
public:
    CommandLine();
    ~CommandLine();

    void addFlag(char short_key, std::string&& long_key, bool default_value = false);
    void addFlag(char short_key, const std::string& long_key, bool default_value = false);
    void addOption(char short_key, std::string&& long_key, std::string&& default_value = std::string());
    void addOption(char short_key, const std::string& long_key, std::string&& default_value = std::string());

    bool findFlag(char key) const;
    bool findFlag(const std::string& key) const;
    std::string findOption(char key) const;
    std::string findOption(const std::string& key) const;

    void parse(int argc, char** argv);



private:
    std::vector<std::string> long_names;
    std::vector<std::string> arguments;
    std::unordered_map<std::string, bool> flags;
    std::unordered_map<std::string, std::string> options;
};

#endif
