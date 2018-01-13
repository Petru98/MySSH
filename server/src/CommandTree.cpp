#include <CommandTree.hpp>
#include <Pipe.hpp>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>



// CommandTree::Node
CommandTree::Node::Node() : operation(Nop), cmd(), stdin_filename(), stdout_filename(), stderr_filename(), left(nullptr), right(nullptr)
{}

bool CommandTree::Node::isOperator() const
{
    return this->operation == Sequence || this->operation == And || this->operation == Or || this->operation == Pipe;
}
bool CommandTree::Node::isOperand() const
{
    return this->operation == Execute;
}



// CommandTree
CommandTree::CommandTree() : root(nullptr), callback(nullptr), size(0)
{}
CommandTree::CommandTree(Callback callback) : root(nullptr), callback(callback), size(0)
{}
CommandTree::CommandTree(const char* cmd) : root(nullptr), callback(nullptr), size(0)
{
    this->parse(cmd);
}
CommandTree::CommandTree(const std::string& cmd) : root(nullptr), callback(nullptr), size(0)
{
    this->parse(cmd);
}
CommandTree::CommandTree(Callback callback, const char* cmd) : root(nullptr), callback(callback), size(0)
{
    this->parse(cmd);
}
CommandTree::CommandTree(Callback callback, const std::string& cmd) : root(nullptr), callback(callback), size(0)
{
    this->parse(cmd);
}
CommandTree::~CommandTree()
{
    this->clear();
}



CommandTree& CommandTree::parse(const char* cmd)
{
    this->clear();

    std::vector<Node*> operators_stack;
    std::vector<Node*> operands_stack;
    bool operand_expected = true;

    try
    {
        while(true)
        {
            Node* node = nullptr;
            cmd = this->getNextNode(cmd, node, operand_expected);

            if(node == nullptr)
                break;

            if(node->isOperand())
                operands_stack.push_back(node);
            else
                operators_stack.push_back(node);
        }

        if(operands_stack.size() > 0)
        {
            this->root = operands_stack.back();
            operands_stack.pop_back();

            while(operands_stack.size() > 0)
            {
                Node* operand_node = operands_stack.back();
                Node* operator_node = operators_stack.back();

                operator_node->left = operand_node;
                operator_node->right = this->root;
                this->root = operator_node;

                operands_stack.pop_back();
                operators_stack.pop_back();
            }
        }
    }
    catch(...)
    {
        for(Node* node : operators_stack)
            delete node;
        for(Node* node : operands_stack)
            delete node;
        throw;
    }

    return (*this);
}
CommandTree& CommandTree::parse(const std::string& cmd)
{
    return this->parse(cmd.c_str());
}

int CommandTree::execute() const
{
    return this->executeSubtree(this->root, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
}



void CommandTree::clear()
{
    if(this->root != nullptr)
    {
        this->deleteSubtree(this->root);
        this->root = nullptr;
        this->size = 0;
    }
}
void CommandTree::setCallback(Callback callback)
{
    this->callback = callback;
}

std::size_t CommandTree::getSize() const
{
    return this->size;
}
CommandTree::Callback CommandTree::getCallback() const
{
    return this->callback;
}





const char* CommandTree::getNextToken(const char* cmd, std::string& token)
{
    constexpr char delims[] = " \t";

    while((*cmd) != '\0' && strchr(delims, *cmd) != nullptr)
            ++cmd;

    if((*cmd) != '\0')
    {
        bool quotes_open = false;

        while((*cmd) != '\0' && (quotes_open == true || strchr(delims, *cmd) == nullptr))
        {
            if((*cmd) == '\"')
                quotes_open = !quotes_open;

            token.push_back(*cmd);
            ++cmd;
        }
    }

    return cmd;
}

const char* CommandTree::getNextNode(const char* cmd, Node*& node, bool& operand_expected)
{
    std::string token;
    node = nullptr;
    cmd = getNextToken(cmd, token);

    if(token.size() > 0)
    {
        auto get_token_id = [](const std::string& token)
        {
            if(token == ";")
                return Node::Sequence;
            if(token == "&&")
                return Node::And;
            if(token == "||")
                return Node::Or;
            if(token == "|")
                return Node::Pipe;
            return Node::Execute;
        };

        int operation = get_token_id(token);

        if(operation != Node::Execute)
        {
            if(operand_expected == true)
                throw std::runtime_error("syntax error");
            operand_expected = true;

            node = new Node();
            node->operation = operation;
        }
        else
        {
            if(operand_expected == false)
                throw std::runtime_error("syntax error");
            operand_expected = false;

            node = new Node();
            node->operation = operation;
            node->cmd.push_back(std::move(token));
            const char* new_cmd;

            while(true)
            {
                new_cmd = getNextToken(cmd, token);
                if(token.size() == 0 || get_token_id(token) != Node::Execute)
                    break;
                cmd = new_cmd;

                if(token == "<")
                {
                    cmd = getNextToken(cmd, token);
                    if(token.size() == 0)
                    {
                        delete node;
                        node = nullptr;
                        throw std::runtime_error("syntax error");
                    }

                    node->stdin_filename = std::move(token);
                }
                else if(token == ">")
                {
                    cmd = getNextToken(cmd, token);
                    if(token.size() == 0)
                    {
                        delete node;
                        node = nullptr;
                        throw std::runtime_error("syntax error");
                    }

                    node->stdout_filename = std::move(token);
                }
                else if(token == "2>")
                {
                    cmd = getNextToken(cmd, token);
                    if(token.size() == 0)
                    {
                        delete node;
                        node = nullptr;
                        throw std::runtime_error("syntax error");
                    }

                    node->stderr_filename = std::move(token);
                }
                else
                {
                    node->cmd.push_back(std::move(token));
                }
            }
        }
    }

    return cmd;
}



std::size_t CommandTree::deleteSubtree(Node* root)
{
    if(root == nullptr)
        return 0;

    std::size_t deleted = 1;

    if(root->operation != Node::Execute)
    {
        deleted += deleteSubtree(root->left);
        deleted += deleteSubtree(root->right);
    }

    delete root;
    return deleted;
}



int CommandTree::executeSubtree(Node* root, int stdinfd, int stdoutfd, int stderrfd) const
{
    switch(root->operation)
    {
    case Node::Execute:  return this->executeSubtreeExecute (root, stdinfd, stdoutfd, stderrfd, false);
    case Node::Sequence: return this->executeSubtreeSequence(root, stdinfd, stdoutfd, stderrfd);
    case Node::And:      return this->executeSubtreeAnd     (root, stdinfd, stdoutfd, stderrfd);
    case Node::Or:       return this->executeSubtreeOr      (root, stdinfd, stdoutfd, stderrfd);
    case Node::Pipe:     return this->executeSubtreePipe    (root, stdinfd, stdoutfd, stderrfd);
    }

    return 0;
}

int CommandTree::executeSubtreeExecute(Node* root, int stdinfd, int stdoutfd, int stderrfd, bool async) const
{
    assert(root->operation == Node::Execute);
    const int stdinfd_def  = stdinfd;
    const int stdoutfd_def = stdoutfd;
    const int stderrfd_def = stderrfd;

    auto open_file = [](int fd, const std::string& filename, int mode)
    {
        if(filename.length() > 0)
        {
            fd = ::open(filename.c_str(), mode);
            if(fd == -1)
                throw std::runtime_error("could not open file " + filename);
        }
        return fd;
    };
    auto close_all_files = [stdinfd, stdoutfd, stderrfd, stdinfd_def, stdoutfd_def, stderrfd_def]()
    {
        auto close_file = [](int fd, int defaultfd)
        {
            if(fd != defaultfd && fd != -1)
                ::close(fd);
        };

        close_file(stdinfd , stdinfd_def);
        close_file(stdoutfd, stdoutfd_def);
        close_file(stderrfd, stderrfd_def);
    };



    int exit_code = 0;

    try
    {
        stdinfd  = open_file(stdinfd , root->stdin_filename , O_RDONLY);
        stdoutfd = open_file(stdoutfd, root->stdout_filename, O_WRONLY | O_CREAT | O_TRUNC);
        stderrfd = open_file(stderrfd, root->stderr_filename, O_WRONLY | O_CREAT | O_TRUNC);
        exit_code = this->callback(root->cmd, stdinfd, stdoutfd, stderrfd, async);
    }
    catch(...)
    {
        close_all_files();
        throw;
    }



    close_all_files();
    return exit_code;
}

int CommandTree::executeSubtreeSequence(Node* root, int stdinfd, int stdoutfd, int stderrfd) const
{
    this->executeSubtreeExecute(root->left, stdinfd, stdoutfd, stderrfd, false);
    return this->executeSubtree(root->right, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
}

int CommandTree::executeSubtreeAnd(Node* root, int stdinfd, int stdoutfd, int stderrfd) const
{
    const int exit_code = this->executeSubtreeExecute(root->left, stdinfd, stdoutfd, stderrfd, false);
    if(exit_code != 0)
        return exit_code;

    return this->executeSubtree(root->right, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
}

int CommandTree::executeSubtreeOr(Node* root, int stdinfd, int stdoutfd, int stderrfd) const
{
    const int exit_code = this->executeSubtreeExecute(root->left, stdinfd, stdoutfd, stderrfd, false);
    if(exit_code == 0)
        return exit_code;

    return this->executeSubtree(root->right, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
}

int CommandTree::executeSubtreePipe(Node* root, int stdinfd, int stdoutfd, int stderrfd) const
{
    ((void)stdoutfd);

    Pipe pipe;
    this->executeSubtreeExecute(root->left, stdinfd, pipe.getWriteFD(), stderrfd, true);
    return this->executeSubtree(root->right, pipe.getReadFD(), STDOUT_FILENO, STDERR_FILENO);
}
