#ifndef INCLUDED_COMMANDTREE_HPP
#define INCLUDED_COMMANDTREE_HPP

#include <Pipe.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>



class CommandTree
{
private:
    struct Node
    {
        enum Operations
        {
            Nop,

            Execute,

            Sequence,
            And,
            Or,
            Pipe,

            OperationsCount
        };

        int operation;
        std::vector<std::string> cmd;
        std::string stdin_filename;
        std::string stdout_filename;
        std::string stderr_filename;
        Node* left;
        Node* right;



        Node();

        bool isOperator() const;
        bool isOperand() const;
    };

public:
    CommandTree();
    CommandTree(const char* cmd);
    CommandTree(const std::string& cmd);
    ~CommandTree();

    CommandTree& parse(const char* cmd);
    CommandTree& parse(const std::string& cmd);

    template <typename F, typename... Args>
    int execute(F&& func, Args&&... args) const;

    void clear();
    std::size_t getSize() const;

private:
    typedef int callback_t(const std::vector<std::string>&, int, int, int, bool);

    static const char* getNextToken(const char* cmd, std::string& token);
    static const char* getNextNode(const char* cmd, Node*& node, bool& operand_expected);

    static std::size_t deleteSubtree(Node* root);

    int executeSubtree        (Node* root, int stdinfd, int stdoutfd, int stderrfd, std::function<callback_t>& callback) const;
    int executeSubtreeExecute (Node* root, int stdinfd, int stdoutfd, int stderrfd, bool async, std::function<callback_t>& callback) const;
    int executeSubtreeSequence(Node* root, int stdinfd, int stdoutfd, int stderrfd, std::function<callback_t>& callback) const;
    int executeSubtreeAnd     (Node* root, int stdinfd, int stdoutfd, int stderrfd, std::function<callback_t>& callback) const;
    int executeSubtreeOr      (Node* root, int stdinfd, int stdoutfd, int stderrfd, std::function<callback_t>& callback) const;
    int executeSubtreePipe    (Node* root, int stdinfd, int stdoutfd, int stderrfd, std::function<callback_t>& callback) const;



private:
    Node* root;
    std::size_t size;
};





template <typename F, typename... Args>
int CommandTree::execute(F&& func, Args&&... args) const
{
    using namespace std::placeholders;

    std::function<callback_t> callback = std::bind(std::forward<F&&>(func), std::forward<Args&&>(args)..., _1, _2, _3, _4, _5);
    return this->executeSubtree(this->root, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO, callback);
}

#endif
