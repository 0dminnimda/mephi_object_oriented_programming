#include <charconv>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "EH.hpp"
#include "cocktail/cocktail.hpp"

template <typename T, typename... Ts>
using is_one_of = std::disjunction<std::is_same<T, Ts>...>;

template <typename T, typename... Ts>
inline constexpr bool is_one_of_v = is_one_of<T, Ts...>::value;

enum TokenKind {
    Identifier,
    Operator,
    Bracket,
    Decimal,
    Floating,
    String,
    End,
};

#define ENUM_TO_STR_CASE(NAME) \
    case NAME: return #NAME

const char *token_kind_name(TokenKind kind) {
    switch (kind) {
        ENUM_TO_STR_CASE(Identifier);
        ENUM_TO_STR_CASE(Operator);
        ENUM_TO_STR_CASE(Bracket);
        ENUM_TO_STR_CASE(Decimal);
        ENUM_TO_STR_CASE(Floating);
        ENUM_TO_STR_CASE(String);
        ENUM_TO_STR_CASE(End);
        default: return "Unknown";
    }
}

namespace std {
    std::string to_string(const TokenKind &kind) {
        return std::string() + "TokenKind::" + token_kind_name(kind);
    }
}  // namespace std

class Token {
public:
    TokenKind kind;
    std::string_view lexeme;

    Token() : Token(TokenKind::End) {}
    Token(TokenKind kind) : kind(kind), lexeme() {}
    Token(TokenKind kind, const std::string_view &lexeme) : kind(kind), lexeme(lexeme) {}
    Token(TokenKind kind, const char *start, const char *end)
        : Token(kind, std::string_view(start, end - start + 1)) {}
    Token(const Token &) = default;
    Token &operator=(const Token &) = default;
    bool operator==(const Token &other) const {
        return std::tie(kind, lexeme) == std::tie(other.kind, other.lexeme);
    };
    bool operator!=(const Token &other) const { return !(*this == other); };

    std::string to_string() const {
        return std::string() + "Token" + "(" + std::to_string(kind) + ", " + "\"" +
               std::string(lexeme) + "\"" + ")";
    }

    friend std::ostream &operator<<(std::ostream &stream, const Token &value) {
        stream << value.to_string();
        return stream;
    };
};

namespace std {
    std::string to_string(const Token &token) { return token.to_string(); }
}  // namespace std

bool isbracket(char c) {
    switch (c) {
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}': return true;
        default: return false;
    }
}

bool isquote(char c) {
    switch (c) {
        case '\'':
        case '"': return true;
        default: return false;
    }
}

bool isoperator(char c) { return std::ispunct(c) && !isbracket(c) && !isquote(c); }

class CharStream {
private:
    std::string &code;
    std::size_t cursor;

public:
    CharStream(std::string &code) : code(code), cursor(0) {}

    const char *peek_at(std::size_t position) {
        if (position < code.size()) return &code[position];
        return nullptr;
    }
    const char *peek() { return peek_at(cursor); }
    const char *peek_prev() {
        if (cursor == 0) return nullptr;
        return peek_at(cursor - 1);
    }
    void consume() { ++cursor; }
};

class Lexer {
private:
    std::vector<Token> tokens;
    std::size_t cursor;

public:
    Lexer() : tokens(), cursor(0) {}

private:
    Token get_number(CharStream &stream) {
        const char *start = stream.peek();
        while (stream.peek() && std::isdigit(*stream.peek())) stream.consume();
        if (stream.peek() && *stream.peek() == '.') {
            stream.consume();
            while (stream.peek() && std::isdigit(*stream.peek())) stream.consume();
            return Token(TokenKind::Floating, start, stream.peek_prev());
        } else {
            return Token(TokenKind::Decimal, start, stream.peek_prev());
        }
    }

    Token get_identifier(CharStream &stream) {
        const char *start = stream.peek();
        while (stream.peek() && std::isalnum(*stream.peek())) stream.consume();
        return Token(TokenKind::Identifier, start, stream.peek_prev());
    }

    Token get_operator(CharStream &stream) {
        const char *start = stream.peek();
        while (stream.peek() && isoperator(*stream.peek())) stream.consume();
        return Token(TokenKind::Operator, start, stream.peek_prev());
    }

    Token get_bracket(CharStream &stream) {
        const char *start = stream.peek();
        if (stream.peek() && isbracket(*stream.peek())) stream.consume();
        return Token(TokenKind::Bracket, start, stream.peek_prev());
    }

    Token get_string(CharStream &stream) {
        const char *quote = stream.peek();
        stream.consume();
        const char *start = stream.peek();

        while (stream.peek() && *stream.peek() != *quote) stream.consume();
        const char *end = stream.peek_prev();

        if (!stream.peek() || *stream.peek() != *quote)
            throw std::runtime_error("String did not close correctly");

        stream.consume();
        return Token(TokenKind::String, start, end);
    }

    bool get_next_token(CharStream &stream, Token &token) {
        while (stream.peek()) {
            if (std::isdigit(*stream.peek())) {
                token = get_number(stream);
                return true;
            } else if (std::isalpha(*stream.peek())) {
                token = get_identifier(stream);
                return true;
            } else if (isoperator(*stream.peek())) {
                token = get_operator(stream);
                return true;
            } else if (isquote(*stream.peek())) {
                token = get_string(stream);
                return true;
            } else if (isbracket(*stream.peek())) {
                token = get_bracket(stream);
                return true;
            } else if (std::isspace(*stream.peek())) {
                stream.consume();
            } else {
                throw std::runtime_error(
                    "Unexpected character '" + std::string(stream.peek(), 1) + "'"
                );
            }
        }
        return false;
    }

public:
    void lex(std::string &code) {
        tokens.clear();
        cursor = 0;

        CharStream stream(code);
        Token token;
        while (get_next_token(stream, token)) {
            tokens.push_back(token);
        }
    }

    Token peek_at(std::size_t position) {
        if (position < tokens.size()) return tokens[position];
        return Token(TokenKind::End);
    }
    Token peek() { return peek_at(cursor); }
    Token peek_prev() {
        if (cursor == 0) return Token(TokenKind::End);
        return peek_at(cursor - 1);
    }
    void consume() { ++cursor; }
};

#define BINARY_OPERATION_IMPL(T1, lhs, T2, rhs, code)                             \
    std::string_view lexeme = lexer.peek().lexeme;                                \
    lexer.consume();                                                              \
    return std::visit(                                                            \
        [&](auto lhs, auto rhs) -> value_type {                                   \
            using T1 = std::decay_t<decltype(lhs)>;                               \
            using T2 = std::decay_t<decltype(rhs)>;                               \
            code;                                                                 \
            throw std::runtime_error(                                             \
                "Unsupported operation '" + std::string(lexeme) + "' between '" + \
                typeid(T1).name() + "' and '" + typeid(T2).name() + "'"           \
            );                                                                    \
        },                                                                        \
        lhs, eval()                                                               \
    );

class Evaluator {
public:
    using decimal = long long;
    using floating = float;
    using string = std::string;
    using value_type = std::variant<decimal, floating, string, Cocktail>;

private:
    Lexer lexer;

public:
    Evaluator() : lexer() {}

private:
    decimal eval_decimal() {
        decimal result = 0;
        const std::string_view &lexeme = lexer.peek().lexeme;
        auto conversion = std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), result);
        if (conversion.ec == std::errc{}) {
            lexer.consume();
            return result;
        }
        throw std::runtime_error("Invalid decimal");
    }

    floating eval_floating() {
        const std::string_view &lexeme = lexer.peek().lexeme;
        char *end;
        errno = 0;
        floating result = std::strtof(lexeme.data(), &end);
        if (end != lexeme.data() && end == lexeme.data() + lexeme.size() && errno != ERANGE) {
            lexer.consume();
            return result;
        }
        throw std::runtime_error("Invalid floating");
    }

    string eval_string() {
        string result = string(lexer.peek().lexeme);
        lexer.consume();
        return result;
    }

    Cocktail eval_cocktail() {
        string arg1;
        floating arg2, arg3;

        if (lexer.peek() != Token(TokenKind::Bracket, "("))
            goto bad;
        else
            lexer.consume();

        if (lexer.peek() == Token(TokenKind::Bracket, ")")) {
            lexer.consume();
            return Cocktail();
        }

        arg1 = eval_as<string>("name");
        if (lexer.peek() == Token(TokenKind::Bracket, ")")) {
            lexer.consume();
            return Cocktail(arg1, 0);
        }

        if (lexer.peek() != Token(TokenKind::Operator, ","))
            goto bad;
        else
            lexer.consume();

        arg2 = eval_as<floating>("number");
        if (lexer.peek() == Token(TokenKind::Bracket, ")")) {
            lexer.consume();
            return Cocktail(arg1, arg2);
        }

        if (lexer.peek() != Token(TokenKind::Operator, ","))
            goto bad;
        else
            lexer.consume();

        arg3 = eval_as<floating>("number");
        if (lexer.peek() == Token(TokenKind::Bracket, ")")) {
            lexer.consume();
            return Cocktail(arg1, arg2, arg3);
        }

    bad:
        throw std::runtime_error(
            "Invalid cocktail, unexpected token '" + std::to_string(lexer.peek()) + "'"
        );
    }

    Cocktail eval_identifier() {
        if (lexer.peek().lexeme == "Cocktail" || lexer.peek().lexeme == "Cock") {
            lexer.consume();
            return eval_cocktail();
        }
        throw std::runtime_error("Unknown identifier");
    }

    value_type eval_add(value_type &lhs) {
        BINARY_OPERATION_IMPL(
            T1, lhs, T2, rhs,
            if constexpr (is_one_of_v<T1, decimal, floating> && is_one_of_v<T2, decimal, floating>) {
                return lhs + rhs;
            } else if constexpr (std::is_same_v<T1, Cocktail> && std::is_same_v<T2, Cocktail>) {
                return lhs + rhs;
            } else if constexpr (std::is_same_v<T1, string> && std::is_same_v<T2, string>) {
                return lhs + rhs;
            }
        );
    }

    value_type eval_sub(value_type &lhs) {
        BINARY_OPERATION_IMPL(
            T1, lhs, T2, rhs,
            if constexpr (is_one_of_v<T1, decimal, floating> && is_one_of_v<T2, decimal, floating>) {
                return lhs - rhs;
            }
        );
    }

    value_type eval_mul(value_type &lhs) {
        BINARY_OPERATION_IMPL(
            T1, lhs, T2, rhs,
            if constexpr (is_one_of_v<T1, decimal, floating> && is_one_of_v<T2, decimal, floating>) {
                return lhs * rhs;
            } else if constexpr (std::is_same_v<T1, Cocktail> && is_one_of_v<T2, decimal, floating>) {
                return lhs * rhs;
            }
        );
    }

    value_type eval_shift_right(value_type &lhs, bool return_first = false) {
        BINARY_OPERATION_IMPL(
            T1, lhs, T2, rhs,
            if constexpr (std::is_same_v<T1, Cocktail> && std::is_same_v<T2, Cocktail>) {
                lhs >> rhs;
                if (return_first) {
                    return lhs;
                } else {
                    return rhs;
                }
            } else if constexpr (std::is_same_v<T1, decimal> && std::is_same_v<T2, decimal>) {
                return lhs >> rhs;
            }
        );
    }

    value_type eval_shift_left(value_type &lhs, bool return_first = false) {
        BINARY_OPERATION_IMPL(
            T1, lhs, T2, rhs,
            if constexpr (std::is_same_v<T1, Cocktail> && std::is_same_v<T2, Cocktail>) {
                rhs >> lhs;
                if (return_first) {
                    return lhs;
                } else {
                    return rhs;
                }
            } else if constexpr (std::is_same_v<T1, decimal> && std::is_same_v<T2, decimal>) {
                return lhs << rhs;
            }
        );
    }

    value_type eval_operator(value_type &lhs) {
        std::string_view lexeme = lexer.peek().lexeme;
        if (lexeme == "+") {
            return eval_add(lhs);
        } else if (lexeme == "-") {
            return eval_sub(lhs);
        } else if (lexeme == "*") {
            return eval_mul(lhs);
        } else if (lexeme == ">>" or lexeme == ">>!") {
            return eval_shift_right(lhs, lexeme != ">>");
        } else if (lexeme == "<<" or lexeme == "<<!") {
            return eval_shift_left(lhs, lexeme != "<<");
        } else {
            throw std::runtime_error("Unknown operator '" + std::string(lexeme) + "'");
        }
    }

    value_type eval() {
        value_type prev = decimal{0};

        while ((lexer.peek().kind != TokenKind::End) && (lexer.peek().kind != TokenKind::Bracket) &&
               (lexer.peek() != Token(TokenKind::Operator, ",")))
        {
            if (lexer.peek().kind == TokenKind::Identifier) {
                prev = eval_identifier();
            } else if (lexer.peek().kind == TokenKind::Decimal) {
                prev = eval_decimal();
            } else if (lexer.peek().kind == TokenKind::Floating) {
                prev = eval_floating();
            } else if (lexer.peek().kind == TokenKind::String) {
                prev = eval_string();
            } else if (lexer.peek().kind == TokenKind::Operator) {
                prev = eval_operator(prev);
            } else {
                throw std::runtime_error(
                    "Unknown token '" + std::string(lexer.peek().lexeme) + "'"
                );
            }
        }

        return prev;
    }

    template <typename OutT>
    OutT eval_as(const char *name) {
        return std::visit(
            [&](auto arg) -> OutT {
                using ArgT = std::decay_t<decltype(arg)>;
                if constexpr (std::is_convertible_v<ArgT, OutT>) {
                    return arg;
                } else {
                    throw std::runtime_error("Invalid " + std::string(name));
                }
            },
            eval()
        );
    }

public:
    value_type evaluate(std::string &code) {
        lexer.lex(code);
        return eval();
    }
};

void print_tokens(std::string code) {
    // std::string code = "Cocktail (10, 0.1, var3)";
    // std::string code = "3 - 3 -";
    Lexer lexer;
    lexer.lex(code);

    while (lexer.peek().kind != TokenKind::End) {
        std::cout << lexer.peek() << std::endl;
        lexer.consume();
    }
    std::cout << lexer.peek() << std::endl;
}

void evaluate() {
    std::cout << "\n~> " << std::flush;

    std::string line;
    std::getline(std::cin, line);

    std::string prefix = "lex:";
    if (line.rfind(prefix, 0) == 0) {
        print_tokens(line.substr(prefix.size()));
        return;
    }

    std::visit([](auto value) { std::cout << value << std::endl; }, Evaluator().evaluate(line));
}

int main() {
    while (1) {
        TRY_CATCH_WRAPPER({
            try {
                evaluate();
            } catch (const std::exception &e) {
                std::cout << "Error: " << e.what() << std::endl;
            } catch (...) {
                std::cout << "Unknwon error occured" << std::endl;
            }
        })
    }

    return 0;
}

/*

3 - 3 -
3 + 4.5 - 3.7
3 + 435435 + 343  - 3
fgdfgdf
2 + 4 ~ 5
948038453490580934850934
32423424234234234234233432342.564563463453498537094530475347507348537459834057345038450
0.34536456456456456498203470982370423705482347052734095870234750320485023
341123123123123123123123123132123132123.34536456456456456498203470982370423705482347052734095870234750320485023
Cocktail()
Cocktail("Vodka", 10)
Cocktail("Vodka", 10, 0.1)
Cocktail("Vodka", 10, 0.1) + Cocktail("Vine", 16, 0.1)
Cocktail("Vodka", 10, 0.1) >> Cocktail("Vine", 16, 0.1)
Cocktail("Vodka", 10, 0.1) >>! Cocktail("Vine", 16, 0.1)
Cocktail("Vodka", 10, 0.1) << Cocktail("Vine", 16, 0.1)
Cocktail("Vodka", 10, 0.1) <<! Cocktail("Vine", 16, 0.1)
Cocktail() + 3
Cocktail() * 3
Cocktail("Beer", 5, 0.3) * 3
"it's a string!"
"he" + "llo"

// TODO: update

$ ./main.out
~> 3 - 3 -
0
~> 3 + 4.5 - 3.7
3.8
~> 3 + 435435 + 343  - 3
435778
~> fgdfgdf
Error: Unknown identifier
~> 2 + 4 ~ 5
Error: Unknown operator '~'
~> 948038453490580934850934
Error: Invalid decimal
~> 32423424234234234234233432342.564563463453498537094530475347507348537459834057345038450
3.24234e+28
~> 0.34536456456456456498203470982370423705482347052734095870234750320485023
0.345365
~> 341123123123123123123123123132123132123.
Error: Invalid floating
~> Cocktail()
Cocktail(volume=0.000000, alcohol_fraction=0.000000)
~> Cocktail(10)
Cocktail(volume=10.000000, alcohol_fraction=0.000000)
~> Cocktail(10, 0.1)
Cocktail(volume=10.000000, alcohol_fraction=0.100000)
~> Cocktail(10, 0.1) + Cocktail(16, 0.1)
Cocktail(volume=26.000000, alcohol_fraction=0.100000)
~> Cocktail(10, 0.1) >> Cocktail(16, 0.1)
Cocktail(volume=17.000000, alcohol_fraction=0.100000)
~> Cocktail(10, 0.1) >>! Cocktail(16, 0.1)
Cocktail(volume=9.000000, alcohol_fraction=0.100000)
~> Cocktail(10, 0.1) << Cocktail(16, 0.1)
Cocktail(volume=15.000000, alcohol_fraction=0.100000)
~> Cocktail(10, 0.1) <<! Cocktail(16, 0.1)
Cocktail(volume=11.000000, alcohol_fraction=0.100000)
~> Cocktail() + 3
Error: Unsupported operation '+' between 'class Cocktail' and '__int64'
~> Cocktail() * 3
Cocktail(volume=0.000000, alcohol_fraction=0.000000)
~> Cocktail(5, 0.3) * 3
Cocktail(volume=15.000000, alcohol_fraction=0.300000)
~> "it's a string!"
it's a string!
~> "he" + "llo"
hello
*/
