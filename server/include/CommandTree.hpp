#ifndef INCLUDED_COMMANDTREE_HPP
#define INCLUDED_COMMANDTREE_HPP

#include <string>
#include <vector>



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
    typedef int (*Callback) (const std::vector<std::string>&, int stdinfd, int stdoutfd, int stderrfd, bool async);

    CommandTree();
    CommandTree(Callback callback);
    CommandTree(const char* cmd);
    CommandTree(const std::string& cmd);
    CommandTree(Callback callback, const char* cmd);
    CommandTree(Callback callback, const std::string& cmd);
    ~CommandTree();

    CommandTree& parse(const char* cmd);
    CommandTree& parse(const std::string& cmd);
    int execute() const;

    void clear();
    void setCallback(Callback callback);

    std::size_t getSize() const;
    Callback getCallback() const;

private:
    static const char* getNextToken(const char* cmd, std::string& token);
    static const char* getNextNode(const char* cmd, Node*& node, bool& operand_expected);

    static std::size_t deleteSubtree(Node* root);

    int executeSubtree        (Node* root, int stdinfd, int stdoutfd, int stderrfd) const;
    int executeSubtreeExecute (Node* root, int stdinfd, int stdoutfd, int stderrfd, bool async) const;
    int executeSubtreeSequence(Node* root, int stdinfd, int stdoutfd, int stderrfd) const;
    int executeSubtreeAnd     (Node* root, int stdinfd, int stdoutfd, int stderrfd) const;
    int executeSubtreeOr      (Node* root, int stdinfd, int stdoutfd, int stderrfd) const;
    int executeSubtreePipe    (Node* root, int stdinfd, int stdoutfd, int stderrfd) const;



private:
    Node* root;
    Callback callback;
    std::size_t size;
};

#endif
