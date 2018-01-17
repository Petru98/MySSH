#ifndef INCLUDED_COMMANDLINE_HPP
#define INCLUDED_COMMANDLINE_HPP

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <stdexcept>



class CommandLine
{
public:
    class Error      : public std::runtime_error {public: using std::runtime_error::runtime_error;};
    class ParseError : public Error              {public: using Error::Error;};



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

    const std::vector<std::string>& getArguments() const;

    void parse(int argc, char** argv);



private:
    std::vector<std::string> long_names;
    std::vector<std::string> arguments;
    std::unordered_map<std::string, bool> flags;
    std::unordered_map<std::string, std::string> options;
};

#endif





////////////////////////////////////////////////////////////////////////////////
/// \class CommandLine
/// \ingroup shared
////////////////////////////////////////////////////////////////////////////////
