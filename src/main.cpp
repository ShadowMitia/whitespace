#include <iostream>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <optional>
#include <chrono>
#include <thread>
#include <algorithm>

enum class Token
{
    SPACE,
    TAB,
    NEWLINE,
    END_OF_FILE
};

std::string_view token_to_string(Token const tok)
{
    switch (tok)
    {
    case Token::SPACE:
        return "[SPACE]";
        break;
    case Token::TAB:
        return "[TAB]";
        break;
    case Token::NEWLINE:
        return "[NEWLINE]";
        break;
    case Token::END_OF_FILE:
        return "[EOF]";
        break;
    }
    return "";
}

void usage(std::string_view app_name)
{
    std::cout << "wspace 0.2 (c) 2003 Edwin Brady\n";
    std::cout << "-------------------------------\n";
    std::cout << "Usage: " << app_name << " [file]\n";
}

std::string read_file(std::filesystem::path const &path)
{
    std::ifstream file(path, std::fstream::ate);
    const auto size = file.tellg();
    file.seekg(0);
    std::string output;
    output.resize(static_cast<std::size_t>(size));
    if (!file.read(output.data(), size))
    {
        return {};
    }

    return output;
}

std::vector<Token> tokenise(std::string const &program)
{
    std::vector<Token> tokens;

    for (auto const &c : program)
    {
        switch (c)
        {
        case ' ':
            tokens.push_back(Token::SPACE);
            break;
        case '\n':
            // case '\r':
            tokens.push_back(Token::NEWLINE);
            break;
        case '\t':
            tokens.push_back(Token::TAB);
            break;

        default:
            continue;
            break;
        }
    }

    tokens.push_back(Token::END_OF_FILE);

    return tokens;
}

enum class InstructionType
{
    Push,
    Dup,
    Ref,
    Slide,
    Swap,
    Discard,
    InfixPlus,
    InfixMinus,
    InfixTimes,
    InfixDivide,
    InfixModulo,
    Store,
    Retrieve,
    Label,
    Call,
    Jump,
    IfZero,
    IfNegative,
    Return,
    End,
    OutputChar,
    OutputNum,
    ReadChar,
    ReadNum
};

std::string instruction_to_string(InstructionType type)
{
    switch (type)
    {
    case InstructionType::Push:
        return "Push";
        break;
    case InstructionType::Dup:
        return "Dup";
        break;
    case InstructionType::Ref:
        return "Ref";
        break;
    case InstructionType::Slide:
        return "Slide";
        break;
    case InstructionType::Swap:
        return "Swap";
        break;
    case InstructionType::Discard:
        return "Discard";
        break;
    case InstructionType::InfixPlus:
        return "InfixPlus";
        break;
    case InstructionType::InfixMinus:
        return "InfixMinus";
        break;
    case InstructionType::InfixTimes:
        return "InfixTimes";
        break;
    case InstructionType::InfixDivide:
        return "InfixDivide";
        break;
    case InstructionType::InfixModulo:
        return "InfixModulo";
        break;
    case InstructionType::Store:
        return "Store";
        break;
    case InstructionType::Retrieve:
        return "Retrieve";
        break;
    case InstructionType::Label:
        return "Label";
        break;
    case InstructionType::Call:
        return "Call";
        break;
    case InstructionType::Jump:
        return "Jump";
        break;
    case InstructionType::IfZero:
        return "IfZero";
        break;
    case InstructionType::IfNegative:
        return "IfNegative";
        break;
    case InstructionType::Return:
        return "Return";
        break;
    case InstructionType::End:
        return "End";
        break;
    case InstructionType::OutputChar:
        return "OutputChar";
        break;
    case InstructionType::OutputNum:
        return "OutputNum";
        break;
    case InstructionType::ReadChar:
        return "ReadChar";
        break;
    case InstructionType::ReadNum:
        return "ReadNum";
        break;
    };
    return "";
}

struct Instruction
{
    InstructionType type;
    int number{0};
    std::string string{};
    bool is_number{false};
    bool is_string{false};

    Instruction() = delete;

    Instruction(InstructionType t) : type(t)
    {
    }

    Instruction(InstructionType t, int num) : type(t), number(num), is_number(true)
    {
    }

    Instruction(InstructionType t, std::string s) : type(t), string(std::move(s)), is_string(true)
    {
    }
};

std::vector<Instruction> parse(std::vector<Token> const &tokens)
{
    std::vector<Instruction> instructions;

    if (tokens.size() == 1)
    {
        return instructions;
    }

    std::size_t current_index = 0;

    auto consume = [&]
    {
        return tokens[++current_index];
    };

    auto peek = [&](std::size_t ahead)
    {
        return tokens[current_index + ahead];
    };

    auto parse_number = [&]
    {
        std::vector<int> digits;
        auto current_digit = consume();

        while (current_digit != Token::NEWLINE)
        {
            if (current_digit == Token::SPACE)
            {
                digits.push_back(0);
            }
            else if (current_digit == Token::TAB)
            {
                digits.push_back(1);
            }
            current_digit = consume();
        }

        bool is_negative = digits.front() == 1 ? true : false;
        digits.erase(std::begin(digits));

        int num = 0;
        for (std::size_t i = 0; i < digits.size(); i++)
        {
            num += static_cast<int>(std::pow(2, digits.size() - 1 - i) * digits[i]);
        }

        return is_negative ? num * -1 : num;
    };

    auto parse_string = [&]
    {
        std::string s;
        auto current_digit = consume();
        while (current_digit != Token::NEWLINE)
        {
            std::vector<int> digits;
            for (std::size_t i = 0; i < 8; i++)
            {
                if (current_digit == Token::SPACE)
                {
                    digits.push_back(0);
                }
                else if (current_digit == Token::TAB)
                {
                    digits.push_back(1);
                }

                current_digit = consume();
                if (current_digit == Token::NEWLINE)
                {
                    break;
                }
            }

            int num = 0;
            for (std::size_t i = 0; i < digits.size(); i++)
            {
                num += static_cast<int>(std::pow(2, i) * digits[digits.size() - 1 - i]);
            }

            s.push_back(static_cast<char>(num));
        }
        return s;
    };

    auto current = tokens[0];

    while (current != Token::END_OF_FILE)
    {
        // IMP : [SPACE] : Stack manipulation
        if (current == Token::SPACE && peek(1) == Token::SPACE)
        {
            consume();
            instructions.push_back(Instruction(InstructionType::Push, parse_number()));
        }
        else if (current == Token::SPACE && peek(1) == Token::NEWLINE && peek(2) == Token::SPACE)
        {
            consume();
            consume();
            instructions.push_back(Instruction{InstructionType::Dup});
        }
        else if (current == Token::SPACE && peek(1) == Token::TAB && peek(2) == Token::SPACE)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::Ref, parse_number()));
        }
        else if (current == Token::SPACE && peek(1) == Token::TAB && peek(2) == Token::NEWLINE)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::Slide, parse_number()));
        }
        else if (current == Token::SPACE && peek(1) == Token::NEWLINE && peek(2) == Token::TAB)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::Swap));
        }
        else if (current == Token::SPACE && peek(1) == Token::NEWLINE && peek(2) == Token::NEWLINE)
        {
            consume();
            consume();

            instructions.push_back(Instruction(InstructionType::Discard));
        }
        // IMP : [TAB][SPACE] : Arithmetic operations
        else if (current == Token::TAB && peek(1) == Token::SPACE && peek(2) == Token::SPACE && peek(3) == Token::SPACE)
        {
            consume();
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::InfixPlus));
        }
        else if (current == Token::TAB && peek(1) == Token::SPACE && peek(2) == Token::SPACE && peek(3) == Token::TAB)
        {
            consume();
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::InfixMinus));
        }
        else if (current == Token::TAB && peek(1) == Token::SPACE && peek(2) == Token::SPACE && peek(3) == Token::NEWLINE)
        {
            consume();
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::InfixTimes));
        }
        else if (current == Token::TAB && peek(1) == Token::SPACE && peek(2) == Token::TAB && peek(3) == Token::SPACE)
        {
            consume();
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::InfixDivide));
        }
        else if (current == Token::TAB && peek(1) == Token::SPACE && peek(2) == Token::TAB && peek(3) == Token::TAB)
        {
            consume();
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::InfixModulo));
        }
        // IMP : [TAB][TAB] : Heap Access
        else if (current == Token::TAB && peek(1) == Token::TAB && peek(2) == Token::SPACE)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::Store));
        }
        else if (current == Token::TAB && peek(1) == Token::TAB && peek(2) == Token::TAB)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::Retrieve));
        }
        // IMP : [LineFeed] : Control Flow
        else if (current == Token::NEWLINE && peek(1) == Token::SPACE && peek(2) == Token::SPACE)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::Label, parse_string()));
        }
        else if (current == Token::NEWLINE && peek(1) == Token::SPACE && peek(2) == Token::TAB)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::Call, parse_string()));
        }
        else if (current == Token::NEWLINE && peek(1) == Token::SPACE && peek(2) == Token::NEWLINE)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::Jump, parse_string()));
        }
        else if (current == Token::NEWLINE && peek(1) == Token::TAB && peek(2) == Token::SPACE)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::IfZero, parse_string()));
        }
        else if (current == Token::NEWLINE && peek(1) == Token::TAB && peek(2) == Token::TAB)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::IfNegative, parse_string()));
        }
        else if (current == Token::NEWLINE && peek(1) == Token::TAB && peek(2) == Token::NEWLINE)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::Return));
        }
        else if (current == Token::NEWLINE && peek(1) == Token::NEWLINE && peek(2) == Token::NEWLINE)
        {
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::End));
        }
        // IMP : [TAB][NEWLINE] : IO instructions
        else if (current == Token::TAB && peek(1) == Token::NEWLINE && peek(2) == Token::SPACE && peek(3) == Token::SPACE)
        {
            consume();
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::OutputChar));
        }
        else if (current == Token::TAB && peek(1) == Token::NEWLINE && peek(2) == Token::SPACE && peek(3) == Token::TAB)
        {
            consume();
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::OutputNum));
        }
        else if (current == Token::TAB && peek(1) == Token::NEWLINE && peek(2) == Token::TAB && peek(3) == Token::SPACE)
        {
            consume();
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::ReadChar));
        }
        else if (current == Token::TAB && peek(1) == Token::NEWLINE && peek(2) == Token::TAB && peek(3) == Token::TAB)
        {
            consume();
            consume();
            consume();
            instructions.push_back(Instruction(InstructionType::ReadNum));
        }
        else
        {
            std::cerr << "Unrecognised input";
            std::terminate();
        }
        current = consume();
    }

    return instructions;
}

class Stack
{
    std::vector<int> values;

public:
    int &operator[](std::size_t idx)
    {
        return values[idx];
    }

    int const &operator[](std::size_t idx) const
    {
        return values[idx];
    }

    std::size_t size() const
    {
        return values.size();
    }

    int top() const
    {
        return values.back();
    }

    void push(int val)
    {
        values.push_back(val);
    }

    int pop()
    {
        int val = top();
        values.pop_back();
        return val;
    }

    void slide(std::size_t i)
    {
        auto n = top();
        values.resize(values.size() - i - 1);
        push(n);
    }

    void swap()
    {
        std::swap(values[values.size() - 1], values[values.size() - 2]);
    }
};

struct VM
{
    Stack value_stack;
    Stack call_stack;
    std::vector<int> memory;
    std::size_t program_counter{0};
};

std::optional<std::size_t> find_label(std::vector<Instruction> const &instructions, std::string_view label)
{
    for (std::size_t i = 0; i < instructions.size(); i++)
    {
        switch (instructions[i].type)
        {
        case InstructionType::Label:
            if (instructions[i].string == label)
            {
                return {i};
                break;
            }
        default:
            continue;
            break;
        }
    }
    return std::nullopt;
}

void run_vm(std::vector<Instruction> const &instructions)
{
    VM vm;

    while (true)
    {
        const auto current_instruction = instructions[vm.program_counter++];

        switch (current_instruction.type)
        {
        case InstructionType::Push:
            vm.value_stack.push(current_instruction.number);
            break;
        case InstructionType::Dup:
            vm.value_stack.push(vm.value_stack.top());
            break;
        case InstructionType::Ref:
            vm.value_stack.push(vm.value_stack[static_cast<std::size_t>(current_instruction.number)]);
            break;
        case InstructionType::Slide:
            vm.value_stack.slide(static_cast<std::size_t>(current_instruction.number));
            break;
        case InstructionType::Swap:
            vm.value_stack.swap();
            break;
        case InstructionType::Discard:
            vm.value_stack.pop();
            break;
        case InstructionType::InfixPlus:
        {
            const auto y = vm.value_stack.pop();
            const auto x = vm.value_stack.pop();
            vm.value_stack.push(x + y);
        }
        break;
        case InstructionType::InfixMinus:
        {
            const auto y = vm.value_stack.pop();
            const auto x = vm.value_stack.pop();
            vm.value_stack.push(x - y);
        }
        break;
        case InstructionType::InfixTimes:
        {
            const auto y = vm.value_stack.pop();
            const auto x = vm.value_stack.pop();
            vm.value_stack.push(x * y);
        }
        break;
        case InstructionType::InfixDivide:
        {
            const auto y = vm.value_stack.pop();
            const auto x = vm.value_stack.pop();
            vm.value_stack.push(x / y);
        }
        break;
        case InstructionType::InfixModulo:
        {
            const auto y = vm.value_stack.pop();
            const auto x = vm.value_stack.pop();
            vm.value_stack.push(x % y);
        }
        break;
        case InstructionType::OutputChar:
            std::cout << static_cast<unsigned char>(vm.value_stack.pop());
            break;
        case InstructionType::ReadChar:
        {
            std::string char_input;
            std::getline(std::cin, char_input);
            const auto loc = static_cast<std::size_t>(std::abs(vm.value_stack.pop()));
            if (vm.memory.size() <= loc)
            {
                vm.memory.resize(loc + 1, 0);
            }
            const auto character = char_input.empty() ? '\n' : static_cast<int>(char_input[0]);
            vm.memory[loc] = character;
        }
        break;
        case InstructionType::OutputNum:
            std::cout << vm.value_stack.pop();
            break;
        case InstructionType::ReadNum:
        {
            std::string number_input;
            std::getline(std::cin, number_input);
            const auto loc = static_cast<std::size_t>(vm.value_stack.pop());
            if (vm.memory.size() <= loc)
            {
                vm.memory.resize(loc + 1, 0);
            }
            const int number = std::stoi(number_input);
            vm.memory[loc] = number;
        }
        break;
        case InstructionType::Label:
            // Do nothing
            break;
        case InstructionType::Call:
        {
            vm.call_stack.push(static_cast<int>(vm.program_counter));
            const auto label = current_instruction.string;
            const auto jump = find_label(instructions, label);
            if (!jump)
            {
                std::cerr << "Undefined label " << label << '\n';
                std::terminate();
            }
            else
            {
                vm.program_counter = *jump;
            }
        }
        break;
        case InstructionType::Jump:
        {
            const auto jump = find_label(instructions, current_instruction.string);
            if (!jump)
            {
                std::cerr << "Undefined label " << current_instruction.string << '\n';
                std::terminate();
            }
            else
            {
                vm.program_counter = *jump;
            }
        }
        break;
        case InstructionType::IfNegative:
        {
            const auto label = current_instruction.string;
            const auto n = vm.value_stack.pop();
            if (n < 0)
            {
                const auto jump = find_label(instructions, label);
                if (!jump)
                {
                    std::cerr << "Undefined label " << label << '\n';
                    std::terminate();
                }
                else
                {
                    vm.program_counter = *jump;
                }
            }
        }
        break;
        case InstructionType::IfZero:
        {
            const auto label = current_instruction.string;
            const auto n = vm.value_stack.pop();
            if (n == 0)
            {
                const auto jump = find_label(instructions, label);
                if (!jump)
                {
                    std::cerr << "Undefined label " << label << '\n';
                    std::terminate();
                }
                else
                {
                    vm.program_counter = *jump;
                }
            }
        }
        break;
        case InstructionType::Return:
            vm.program_counter = static_cast<std::size_t>(vm.call_stack.pop());
            break;
        case InstructionType::Store:
        {
            const auto n = vm.value_stack.pop();
            const auto loc = static_cast<std::size_t>(vm.value_stack.pop());
            if (vm.memory.size() <= loc)
            {
                vm.memory.resize(loc + 1, 0);
            }
            vm.memory[loc] = n;
        }
        break;
        case InstructionType::Retrieve:
        {
            const auto loc = static_cast<std::size_t>(vm.value_stack.pop());
            vm.value_stack.push(vm.memory[loc]);
        }
        break;
        case InstructionType::End:
            return;
            break;
        default:
            std::cerr << "Unknown instruction\n";
            std::terminate();
            break;
        }
    }
}

void execute(std::filesystem::path const &file)
{
    const auto source = read_file(file);

    const auto tokens = tokenise(source);

    const auto instructions = parse(tokens);

    run_vm(instructions);
}

int main(int argc, char *argv[])
{

    std::string_view app_name{argv[0]};

    if (argc != 2)
    {
        usage(app_name);
        return EXIT_FAILURE;
    }
    else
    {
        std::filesystem::path file(argv[1]);
        execute(file);
    }

    return EXIT_SUCCESS;
}