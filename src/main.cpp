#include <iostream>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <optional>
#include <chrono>
#include <algorithm>
#include <variant>

enum class Token
{
    SPACE,
    TAB,
    NEWLINE,
    END_OF_FILE
};

[[nodiscard]] std::string_view token_to_string(Token const tok)
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

[[nodiscard]] std::string read_file(std::filesystem::path const &path)
{
    std::ifstream file(path, std::fstream::ate);
    const auto size = file.tellg();
    file.seekg(0);
    std::string output(static_cast<std::size_t>(size), ' ');
    if (!file.read(output.data(), size))
    {
        return {};
    }

    return output;
}

[[nodiscard]] std::vector<Token> tokenise(std::string const &program)
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

[[nodiscard]] std::string instruction_to_string(InstructionType type)
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
    std::variant<int, std::string> data;
};

std::vector<Instruction> parse(std::vector<Token> const &tokens)
{
    std::vector<Instruction> instructions;

    if (tokens.size() == 1)
    {
        return instructions;
    }

    std::size_t current_index = 0;

    const auto consume = [&]
    {
        return tokens[++current_index];
    };

    const auto peek = [&](std::size_t ahead)
    {
        return tokens[current_index + ahead];
    };

    const auto parse_number = [&]
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

        const bool is_negative = digits.front() == 1;

        int num = 0;
        for (std::size_t i = 1; i < digits.size(); i++)
        {
            num += static_cast<int>(std::pow(2, digits.size() - 1 - i) * digits[i]);
        }

        return is_negative ? num * -1 : num;
    };

    const auto parse_string = [&]
    {
        std::string s;
        auto current_digit = consume();
        while (current_digit != Token::NEWLINE)
        {
            constexpr std::size_t BINARY_CHAR_SIZE = 8;
            std::vector<int> digits;
            for (std::size_t i = 0; i < BINARY_CHAR_SIZE; i++)
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

    const auto match = [&]<typename... Ts>(Ts... toks)
    {
        std::array<Token, sizeof...(toks)> list{toks...};
        for (std::size_t i = 0; i < list.size(); i++)
        {
            if (peek(i) != list[i])
            {
                return false;
            }
        }

        for (std::size_t i = 0; i < list.size() - 1; i++)
        {
            consume();
        }

        return true;
    };

    auto current = tokens[0];

    while (current != Token::END_OF_FILE)
    {
        // IMP : [SPACE] : Stack manipulation
        if (match(Token::SPACE, Token::SPACE))
        {
            instructions.emplace_back(InstructionType::Push, parse_number());
        }
        else if (match(Token::SPACE, Token::NEWLINE, Token::SPACE))
        {
            instructions.emplace_back(InstructionType::Dup);
        }
        else if (match(Token::SPACE, Token::TAB, Token::SPACE))
        {
            instructions.emplace_back(InstructionType::Ref, parse_number());
        }
        else if (match(Token::SPACE, Token::TAB, Token::NEWLINE))
        {
            instructions.emplace_back(InstructionType::Slide, parse_number());
        }
        else if (match(Token::SPACE, Token::NEWLINE, Token::TAB))
        {
            instructions.emplace_back(InstructionType::Swap);
        }
        else if (match(Token::SPACE, Token::NEWLINE, Token::NEWLINE))
        {
            instructions.emplace_back(InstructionType::Discard);
        }
        // IMP : [TAB][SPACE] : Arithmetic operations
        else if (match(Token::TAB, Token::SPACE, Token::SPACE, Token::SPACE))
        {
            instructions.emplace_back(InstructionType::InfixPlus);
        }
        else if (match(Token::TAB, Token::SPACE, Token::SPACE, Token::TAB))
        {
            instructions.emplace_back(InstructionType::InfixMinus);
        }
        else if (match(Token::TAB, Token::SPACE, Token::SPACE, Token::NEWLINE))
        {
            instructions.emplace_back(InstructionType::InfixTimes);
        }
        else if (match(Token::TAB, Token::SPACE, Token::TAB, Token::SPACE))
        {
            instructions.emplace_back(InstructionType::InfixDivide);
        }
        else if (match(Token::TAB, Token::SPACE, Token::TAB, Token::TAB))
        {
            instructions.emplace_back(InstructionType::InfixModulo);
        }
        // IMP : [TAB][TAB] : Heap Access
        else if (match(Token::TAB, Token::TAB, Token::SPACE))
        {
            instructions.emplace_back(InstructionType::Store);
        }
        else if (match(Token::TAB, Token::TAB, Token::TAB))
        {
            instructions.emplace_back(InstructionType::Retrieve);
        }
        // IMP : [LineFeed] : Control Flow
        else if (match(Token::NEWLINE, Token::SPACE, Token::SPACE))
        {
            instructions.emplace_back(InstructionType::Label, parse_string());
        }
        else if (match(Token::NEWLINE, Token::SPACE, Token::TAB))
        {
            instructions.emplace_back(InstructionType::Call, parse_string());
        }
        else if (match(Token::NEWLINE, Token::SPACE, Token::NEWLINE))
        {
            instructions.emplace_back(InstructionType::Jump, parse_string());
        }
        else if (match(Token::NEWLINE, Token::TAB, Token::SPACE))
        {
            instructions.emplace_back(InstructionType::IfZero, parse_string());
        }
        else if (match(Token::NEWLINE, Token::TAB, Token::TAB))
        {
            instructions.emplace_back(InstructionType::IfNegative, parse_string());
        }
        else if (match(Token::NEWLINE, Token::TAB, Token::NEWLINE))
        {
            instructions.emplace_back(InstructionType::Return);
        }
        else if (match(Token::NEWLINE, Token::NEWLINE, Token::NEWLINE))
        {
            instructions.emplace_back(InstructionType::End);
        }
        // IMP : [TAB][NEWLINE] : IO instructions
        else if (match(Token::TAB, Token::NEWLINE, Token::SPACE, Token::SPACE))
        {
            instructions.emplace_back(InstructionType::OutputChar);
        }
        else if (match(Token::TAB, Token::NEWLINE, Token::SPACE, Token::TAB))
        {
            instructions.emplace_back(InstructionType::OutputNum);
        }
        else if (match(Token::TAB, Token::NEWLINE, Token::TAB, Token::SPACE))
        {
            instructions.emplace_back(InstructionType::ReadChar);
        }
        else if (match(Token::TAB, Token::NEWLINE, Token::TAB, Token::TAB))
        {
            instructions.emplace_back(InstructionType::ReadNum);
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

    [[nodiscard]] std::size_t size() const
    {
        return values.size();
    }

    [[nodiscard]] int top() const
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

class Heap
{
    mutable std::vector<int> values;

public:
    [[nodiscard]] int &operator[](std::size_t idx)
    {
        if (values.size() <= idx)
        {
            values.resize(idx + 1, 0);
        }
        return values[idx];
    }

    [[nodiscard]] int const &operator[](std::size_t idx) const
    {
        if (values.size() <= idx)
        {
            values.resize(idx + 1, 0);
        }
        return values[idx];
    }
};

struct VM
{
    Stack value_stack;
    Stack call_stack;
    Heap memory;
    std::size_t program_counter{0};
};

std::optional<std::size_t> find_label(std::vector<Instruction> const &instructions, std::string_view label)
{
    for (std::size_t i = 0; i < instructions.size(); i++)
    {
        switch (instructions[i].type)
        {
        case InstructionType::Label:
            if (std::get<std::string>(instructions[i].data) == label)
            {
                return {i};
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

    const auto jump_to = [&](auto const &label)
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
    };

    while (true)
    {
        const auto current_instruction = instructions[vm.program_counter++];

        switch (current_instruction.type)
        {
        case InstructionType::Push:
            vm.value_stack.push(std::get<int>(current_instruction.data));
            break;
        case InstructionType::Dup:
            vm.value_stack.push(vm.value_stack.top());
            break;
        case InstructionType::Ref:
            vm.value_stack.push(vm.value_stack[static_cast<std::size_t>(std::get<int>(current_instruction.data))]);
            break;
        case InstructionType::Slide:
            vm.value_stack.slide(static_cast<std::size_t>(std::get<int>(current_instruction.data)));
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
            const auto character = char_input.empty() ? '\n' : static_cast<unsigned char>(char_input[0]);
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
            const auto label = std::get<std::string>(current_instruction.data);
            jump_to(label);
        }
        break;
        case InstructionType::Jump:
        {
            const auto label = std::get<std::string>(current_instruction.data);
            jump_to(label);
        }
        break;
        case InstructionType::IfNegative:
        {
            const auto n = vm.value_stack.pop();
            if (n < 0)
            {
                const auto label = std::get<std::string>(current_instruction.data);
                jump_to(label);
            }
        }
        break;
        case InstructionType::IfZero:
        {
            const auto n = vm.value_stack.pop();
            if (n == 0)
            {
                const auto label = std::get<std::string>(current_instruction.data);
                jump_to(label);
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

    std::filesystem::path file(argv[1]);
    execute(file);

    return EXIT_SUCCESS;
}