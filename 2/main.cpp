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
#include "cocktail/cocktail_map.hpp"

#define TRY_CATCH_ALL(code)                                    \
    TRY_CATCH_WRAPPER({                                        \
        try {                                                  \
            code;                                              \
        } catch (const std::exception &e) {                    \
            std::cout << "Error: " << e.what() << std::endl;   \
        } catch (...) {                                        \
            std::cout << "Unknwon error occured" << std::endl; \
        }                                                      \
    });

template <typename T, typename... Ts>
using is_one_of = std::disjunction<std::is_same<T, Ts>...>;

template <typename T, typename... Ts>
inline constexpr bool is_one_of_v = is_one_of<T, Ts...>::value;

enum TokenKind {
    Identifier,
    Operator,
    OpeningBracket,
    ClosingBracket,
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
        ENUM_TO_STR_CASE(OpeningBracket);
        ENUM_TO_STR_CASE(ClosingBracket);
        ENUM_TO_STR_CASE(Decimal);
        ENUM_TO_STR_CASE(Floating);
        ENUM_TO_STR_CASE(String);
        ENUM_TO_STR_CASE(End);
        default: return "Unknown";
    }
}

namespace std {
    [[nodiscard]] std::string to_string(const TokenKind &kind) {
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

bool is_opening_bracket(char c) {
    switch (c) {
        case '(':
        case '[':
        case '{': return true;
        default: return false;
    }
}

bool is_closing_bracket(char c) {
    switch (c) {
        case ')':
        case ']':
        case '}': return true;
        default: return false;
    }
}

bool isbracket(char c) { return is_opening_bracket(c) || is_closing_bracket(c); }

bool isquote(char c) {
    switch (c) {
        case '\'':
        case '"': return true;
        default: return false;
    }
}

bool isoperator(char c) { return std::ispunct(c) && !isbracket(c) && !isquote(c); }

bool isident(char c) { return std::isalnum(c) || c == '_'; }

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
    explicit Lexer() : tokens(), cursor(0) {}

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
        while (stream.peek() && isident(*stream.peek())) stream.consume();
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
        if (is_opening_bracket(*start)) {
            return Token(TokenKind::OpeningBracket, start, stream.peek_prev());
        } else {
            return Token(TokenKind::ClosingBracket, start, stream.peek_prev());
        }
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

    Token end() const { return Token(TokenKind::End, "EOF"); }

    Token peek_at(std::size_t position) {
        if (position < tokens.size()) return tokens[position];
        return end();
    }
    Token peek() { return peek_at(cursor); }
    Token peek_prev() {
        if (cursor == 0) return end();
        return peek_at(cursor - 1);
    }
    void consume() { ++cursor; }
};

template <typename T>
struct name_of_type {
    static constexpr const char *value = "unknown";
};

template <>
struct name_of_type<std::string> {
    static constexpr const char *value = "string";
};

template <>
struct name_of_type<long long> {
    static constexpr const char *value = "decimal";
};

template <>
struct name_of_type<float> {
    static constexpr const char *value = "floating";
};

template <>
struct name_of_type<Cocktail> {
    static constexpr const char *value = "cocktail";
};

template <>
struct name_of_type<CocktailMap> {
    static constexpr const char *value = "cocktail_map";
};

#define UNARY_OPERATION_IMPL_(T1, lhs, in_lhs, code)                          \
    std::string_view lexeme = lexer.peek().lexeme;                            \
    lexer.consume();                                                          \
    return std::visit(                                                        \
        [&](auto lhs) -> value_type {                                         \
            using T1 = std::decay_t<decltype(lhs)>;                           \
            code;                                                             \
            throw std::runtime_error(                                         \
                "Unsupported operation '" + std::string(lexeme) + "' for '" + \
                name_of_type<T1>::value + "'"                                 \
            );                                                                \
        },                                                                    \
        in_lhs                                                                \
    );

#define UNARY_OPERATION_IMPL(T1, lhs, code) UNARY_OPERATION_IMPL_(T1, lhs, lhs, code)

#define BINARY_OPERATION_IMPL_(T1, lhs, T2, rhs, in_lhs, in_rhs, code)              \
    std::string_view lexeme = lexer.peek().lexeme;                                  \
    lexer.consume();                                                                \
    return std::visit(                                                              \
        [&](auto lhs, auto rhs) -> value_type {                                     \
            using T1 = std::decay_t<decltype(lhs)>;                                 \
            using T2 = std::decay_t<decltype(rhs)>;                                 \
            code;                                                                   \
            throw std::runtime_error(                                               \
                "Unsupported operation '" + std::string(lexeme) + "' between '" +   \
                name_of_type<T1>::value + "' and '" + name_of_type<T2>::value + "'" \
            );                                                                      \
        },                                                                          \
        in_lhs, in_rhs                                                              \
    );

#define BINARY_OPERATION_IMPL(T1, lhs, T2, rhs, code) \
    BINARY_OPERATION_IMPL_(T1, lhs, T2, rhs, lhs, eval(), code)

class Evaluator {
public:
    using decimal = long long;
    using floating = float;
    using string = std::string;
    using value_type = std::variant<bool, decimal, floating, string, Cocktail, CocktailMap>;

    using VarTable = HashTable<string, value_type>;

private:
    Lexer lexer;
    VarTable &variables;
    bool evaluates_operand;

public:
    explicit Evaluator(VarTable &variables)
        : lexer(), variables(variables), evaluates_operand(false) {}

private:
    decimal eval_decimal() {
        const std::string_view &lexeme = lexer.peek().lexeme;
        char *end;
        errno = 0;
        decimal result = std::strtoll(lexeme.data(), &end, 10);
        if (end != lexeme.data() && end == lexeme.data() + lexeme.size() && errno != ERANGE) {
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

    [[nodiscard]] string eval_string() {
        string result = string(lexer.peek().lexeme);
        lexer.consume();
        return result;
    }

    template <typename T>
    bool consume_if_got(T lexeme) {
        if (lexer.peek().lexeme == lexeme) {
            lexer.consume();
            return true;
        }
        return false;
    }

    template <typename T>
    void expect_and_consume(T lexeme) {
        if (lexer.peek().lexeme != lexeme) {
            throw std::runtime_error(
                "Unexpected token '" + std::string(lexer.peek().lexeme) +
                "'. Perhaps you forgot a '" + std::string(lexeme) + "'?"
            );
        }
        lexer.consume();
    }

    Cocktail eval_cocktail() {
        string arg1;
        floating arg2, arg3;

        expect_and_consume("(");

        if (consume_if_got(")")) {
            return Cocktail();
        }

        arg1 = eval_as<string>(true);
        if (consume_if_got(")")) {
            return Cocktail(arg1, 0);
        }

        expect_and_consume(",");

        arg2 = eval_as<floating>(true);
        if (consume_if_got(")")) {
            return Cocktail(arg1, arg2);
        }

        expect_and_consume(",");

        arg3 = eval_as<floating>(true);
        expect_and_consume(")");

        return Cocktail(arg1, arg2, arg3);
    }

    [[nodiscard]] value_type eval_identifier() {
        if (lexer.peek().lexeme == "Cocktail" || lexer.peek().lexeme == "Cock") {
            lexer.consume();
            return eval_cocktail();
        }
        std::string lexeme(lexer.peek().lexeme);
        if (variables.contains(lexeme)) {
            lexer.consume();
            return variables.at(lexeme);
        }
        throw std::runtime_error("Unknown identifier '" + lexeme + "'");
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

    value_type eval_in_place_add(value_type &lhs) {
        BINARY_OPERATION_IMPL(
            T1, lhs, T2, rhs,
            if constexpr (std::is_same_v<T1, CocktailMap> && std::is_same_v<T2, Cocktail>) {
                lhs += rhs;
                return lhs;
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
                lhs << rhs;
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

    Quartile eval_quartile() {
        if (lexer.peek().lexeme == "FIRST") {
            lexer.consume();
            return Quartile::FIRST;
        } else if (lexer.peek().lexeme == "SECOND") {
            lexer.consume();
            return Quartile::SECOND;
        } else if (lexer.peek().lexeme == "THIRD") {
            lexer.consume();
            return Quartile::THIRD;
        } else if (lexer.peek().lexeme == "FOURTH") {
            lexer.consume();
            return Quartile::FOURTH;
        }
        throw std::runtime_error("Expected quartile");
    }

    [[nodiscard]] value_type eval_getattr(value_type &lhs) {
        UNARY_OPERATION_IMPL(
            T1, lhs,
            if constexpr (std::is_same_v<T1, CocktailMap>) {
                if (lexer.peek().kind != TokenKind::Identifier) {
                    throw std::runtime_error(
                        "Expected identifier for operation '" + std::string(lexeme) + "'"
                    );
                }
                if (lexer.peek().lexeme == "is_empty") {
                    lexer.consume();
                    expect_and_consume("(");
                    expect_and_consume(")");
                    return lhs.is_empty();
                } else if (lexer.peek().lexeme == "is_full") {
                    lexer.consume();
                    expect_and_consume("(");
                    expect_and_consume(")");
                    return lhs.is_full();
                } else if (lexer.peek().lexeme == "is_partially_full") {
                    lexer.consume();
                    expect_and_consume("(");
                    expect_and_consume(")");
                    return lhs.is_partially_full();
                } else if (lexer.peek().lexeme == "erase") {
                    lexer.consume();
                    expect_and_consume("(");
                    string arg1 = eval_as<string>(true);
                    expect_and_consume(")");
                    std::cout << lhs.erase(arg1) << std::endl;
                    return lhs;
                } else if (lexer.peek().lexeme == "rename") {
                    lexer.consume();
                    expect_and_consume("(");
                    string arg1 = eval_as<string>(true);
                    expect_and_consume(",");
                    string arg2 = eval_as<string>(true);
                    expect_and_consume(")");
                    lhs.rename(arg1, arg2);
                    return lhs;
                } else if (lexer.peek().lexeme == "volume_with_alcohol_fraction_in_quartile" || lexer.peek().lexeme == "volquar")
                {
                    lexer.consume();
                    expect_and_consume("(");
                    Quartile arg1 = eval_quartile();
                    expect_and_consume(")");
                    return lhs.volume_with_alcohol_fraction_in_quartile(arg1);
                } else if (lexer.peek().lexeme == "mix_for_alcohol_fraction") {
                    lexer.consume();
                    expect_and_consume("(");
                    floating arg1 = eval_as<floating>(true);
                    expect_and_consume(")");
                    std::cout << lhs.mix_for_alcohol_fraction(arg1) << std::endl;
                    return lhs;
                }
            }
        );
    }

    [[nodiscard]] value_type eval_assign(value_type &lhs) {
        std::string_view lexeme = lexer.peek().lexeme;
        lexer.consume();

        if (lexer.peek().kind != TokenKind::Identifier) {
            throw std::runtime_error(
                "Expected identifier for operation '" + std::string(lexeme) + "'"
            );
        }

        variables.insert(std::string(lexer.peek().lexeme), lhs);
        lexer.consume();

        return lhs;
    }

    [[nodiscard]] value_type eval_operator_(value_type &lhs) {
        std::string_view lexeme = lexer.peek().lexeme;
        if (lexeme == "+") {
            return eval_add(lhs);
        } else if (lexeme == "+=") {
            return eval_in_place_add(lhs);
        } else if (lexeme == "-") {
            return eval_sub(lhs);
        } else if (lexeme == "*") {
            return eval_mul(lhs);
        } else if (lexeme == ">>" or lexeme == ">>!") {
            return eval_shift_right(lhs, lexeme != ">>");
        } else if (lexeme == "<<" or lexeme == "<<!") {
            return eval_shift_left(lhs, lexeme != "<<");
        } else if (lexeme == ".") {
            return eval_getattr(lhs);
        } else if (lexeme == "=") {
            return eval_assign(lhs);
        } else {
            throw std::runtime_error("Unknown operator '" + std::string(lexeme) + "'");
        }
    }

    [[nodiscard]] value_type eval_operator(value_type &lhs) {
        evaluates_operand = true;
        value_type result = eval_operator_(lhs);
        evaluates_operand = false;
        return result;
    }

    [[nodiscard]] CocktailMap eval_map() {
        CocktailMap result;
        expect_and_consume("{");
        while (!consume_if_got("}")) {
            if (result.size()) {
                expect_and_consume(",");
            }
            result += eval_as<Cocktail>(true);
        }
        return result;
    }

    [[nodiscard]] value_type eval_lookup(value_type &lhs) {
        BINARY_OPERATION_IMPL(
            T1, lhs, T2, rhs,
            if constexpr (std::is_same_v<T1, CocktailMap> && std::is_same_v<T2, string>) {
                return lhs[rhs];
            }
        );
    }

    [[nodiscard]] value_type eval_bracket(value_type &lhs) {
        std::string_view lexeme = lexer.peek().lexeme;
        if (lexeme == "{") {
            return eval_map();
        } else if (lexeme == "[") {
            return eval_lookup(lhs);
        } else {
            throw std::runtime_error("Unexpected bracket '" + std::string(lexeme) + "'");
        }
    }

    [[nodiscard]] value_type eval() {
        value_type prev = decimal{0};

        while ((lexer.peek().kind != TokenKind::End) &&
               (lexer.peek().kind != TokenKind::ClosingBracket) &&
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
                if (evaluates_operand) break;
                prev = eval_operator(prev);
            } else if (lexer.peek().kind == TokenKind::OpeningBracket) {
                prev = eval_bracket(prev);
            } else {
                throw std::runtime_error(
                    "Unknown token '" + std::string(lexer.peek().lexeme) + "'"
                );
            }
        }

        return prev;
    }

    [[nodiscard]] value_type eval(bool can_eval_operators) {
        if (can_eval_operators) {
            bool saved_evaluates_operand = evaluates_operand;
            evaluates_operand = false;
            value_type result = eval();
            evaluates_operand = saved_evaluates_operand;
            return result;
        }
        return eval();
    }

    template <typename OutT>
    [[nodiscard]] OutT eval_as(bool can_eval_operators = false) {
        return std::visit(
            [&](auto arg) -> OutT {
                using ArgT = std::decay_t<decltype(arg)>;
                if constexpr (std::is_convertible_v<ArgT, OutT>) {
                    return arg;
                } else {
                    throw std::runtime_error(std::string("Invalid ") + name_of_type<OutT>::value);
                }
            },
            eval(can_eval_operators)
        );
    }

public:
    [[nodiscard]] value_type evaluate(std::string &code) {
        lexer.lex(code);
        return eval();
    }
};

void print_tokens(std::string code) {
    Lexer lexer;
    lexer.lex(code);

    while (lexer.peek().kind != TokenKind::End) {
        std::cout << lexer.peek() << std::endl;
        lexer.consume();
    }
    std::cout << lexer.peek() << std::endl;
}

void evaluate(Evaluator::VarTable &variables) {
    std::cout << "\n~> " << std::flush;

    std::string line;
    std::getline(std::cin, line);

    std::string prefix = "lex:";
    if (line.rfind(prefix, 0) == 0) {
        print_tokens(line.substr(prefix.size()));
        return;
    }

    std::visit(
        [](auto value) { std::cout << value << std::endl; }, Evaluator(variables).evaluate(line)
    );
}

void intrpret() {
    Evaluator::VarTable variables;

    while (1) {
        TRY_CATCH_ALL({ evaluate(variables); })
    }
}

int main() {
    TRY_CATCH_ALL({ intrpret(); })

    return 0;
}

// clang-format off

/*

3 - 3 -
3 + 4.5 - 3.7
3 + 435435 + 343  - 3
3 + 4 - 6
-6 + 3 + 4
fgdfgdf
2 + 4 ~ 5
9170401481593653160
948038453490580934850934
32423424234234234234233432342.564563463453498537094530475347507348537459834057345038450
0.34536456456456456498203470982370423705482347052734095870234750320485023
341123123123123123123123123132123132123.34536456456456456498203470982370423705482347052734095870234750320485023
Cocktail()
Cocktail("Vodka", 10)
Cocktail("Vodka", 10, 0.1)
Cocktail("Vodka", 10, 0.1) + Cocktail("Vine", 16, 0.1)
Cocktail("Vodka", 10, 0.1) + Cocktail("Vine", 10 + 6, 0.1)
Cocktail("Vodka", 10, 0.1) >> Cocktail("Vine", 16, 0.1)
Cocktail("Vodka", 10, 0.1) >>! Cocktail("Vine", 16, 0.1)
Cocktail("Vodka", 10, 0.1) << Cocktail("Vine", 16, 0.1)
Cocktail("Vodka", 10, 0.1) <<! Cocktail("Vine", 16, 0.1)
Cocktail() + 3
Cocktail() * 3
Cocktail("Beer", 5, 0.3) * 3
"it's a string!"
"he" + "llo"
{}
{Cock("Volna", 20, 0.3)}
{Cock("a"), Cock("b"), Cock("c")}
{} += Cock("a")
{} += Cock("a") += Cock("b")
{Cock(), Cock("Vodka", 10, 0.1) + Cock("Vine", 16, 0.1)}
1 = a
a
a + 6
{} += Cock("a", 10, 0.3) += Cock("b", 5, 0.8) = mp
mp.volume_with_alcohol_fraction_in_quartile(FIRST)
mp.volume_with_alcohol_fraction_in_quartile(SECOND)
mp.volume_with_alcohol_fraction_in_quartile(THIRD)
mp.volume_with_alcohol_fraction_in_quartile(FOURTH)
mp.erase("a")
mp.rename("a", "d")
mp.mix_for_alcohol_fraction(0.6)

$ ./main.out
~> 3 - 3 -
0
~> 3 + 4.5 - 3.7
3.8
~> 3 + 435435 + 343  - 3
435778
~> 3 + 4 - 6
1
~> -6 + 3 + 4
1
~> fgdfgdf
Error: Unknown identifier
~> 2 + 4 ~ 5
Error: Unknown operator '~'
~> 9170401481593653160
9170401481593653160
~> 948038453490580934850934
Error: Invalid decimal
~> 32423424234234234234233432342.564563463453498537094530475347507348537459834057345038450
3.24234e+28
~> 0.34536456456456456498203470982370423705482347052734095870234750320485023
0.345365
~> 341123123123123123123123123132123132123.34536456456456456498203470982370423705482347052734095870234750320485023
Error: Invalid floating
~> Cocktail()
Cocktail("", 0.000000, 0.000000)
~> Cocktail("Vodka", 10)
Cocktail("Vodka", 10.000000, 0.000000)
~> Cocktail("Vodka", 10, 0.1)
Cocktail("Vodka", 10.000000, 0.100000)
~> Cocktail("Vodka", 10, 0.1) + Cocktail("Vine", 16, 0.1)
Cocktail("VodkaVine", 26.000000, 0.100000)
~> Cocktail("Vodka", 10, 0.1) + Cocktail("Vine", 10 + 6, 0.1)
Cocktail("VodkaVine", 26.000000, 0.100000)
~> Cocktail("Vodka", 10, 0.1) >> Cocktail("Vine", 16, 0.1)
Cocktail("VineVodka", 17.000000, 0.100000)
~> Cocktail("Vodka", 10, 0.1) >>! Cocktail("Vine", 16, 0.1)
Cocktail("Vodka", 9.000000, 0.100000)
~> Cocktail("Vodka", 10, 0.1) << Cocktail("Vine", 16, 0.1)
Cocktail("Vine", 15.000000, 0.100000)
~> Cocktail("Vodka", 10, 0.1) <<! Cocktail("Vine", 16, 0.1)
Cocktail("VodkaVine", 11.000000, 0.100000)
~> Cocktail() + 3
Error: Unsupported operation '+' between 'cocktail' and 'decimal'
~> Cocktail() * 3
Cocktail("", 0.000000, 0.000000)
~> Cocktail("Beer", 5, 0.3) * 3
Cocktail("Beer", 15.000000, 0.300000)
~> "it's a string!"
it's a string!
~> "he" + "llo"
hello
~> {}
{}
~> {Cock("Volna", 20, 0.3)}
{Cocktail("Volna", 20.000000, 0.300000)}
~> {Cock("a"), Cock("b"), Cock("c")}
{Cocktail("c", 0.000000, 0.000000), Cocktail("b", 0.000000, 0.000000), Cocktail("a", 0.000000, 0.000000)}
~> {} += Cock("a")
{Cocktail("a", 0.000000, 0.000000)}
~> {} += Cock("a") += Cock("b")
{Cocktail("a", 0.000000, 0.000000), Cocktail("b", 0.000000, 0.000000)}
~> {Cock(), Cock("Vodka", 10, 0.1) + Cock("Vine", 16, 0.1)}
{Cocktail("", 0.000000, 0.000000), Cocktail("VodkaVine", 26.000000, 0.100000)}
~> 1 = a
1
~> a
1
~> a + 6
7
~> {} += Cock("a", 10, 0.3) += Cock("b", 5, 0.8) = mp
{Cocktail("b", 5.000000, 0.800000), Cocktail("a", 10.000000, 0.300000)}
~> mp.volume_with_alcohol_fraction_in_quartile(FIRST)
0
~> mp.volume_with_alcohol_fraction_in_quartile(SECOND)
10
~> mp.volume_with_alcohol_fraction_in_quartile(THIRD)
0
~> mp.volume_with_alcohol_fraction_in_quartile(FOURTH)
5
~> mp.erase("a")
1
{Cocktail("b", 5.000000, 0.800000)}
~> mp.rename("a", "d")
{Cocktail("b", 5.000000, 0.800000), Cocktail("d", 10.000000, 0.300000)}
~> mp.mix_for_alcohol_fraction(0.6)
Cocktail("ab", 5.000000, 0.600000)
{Cocktail("b", 2.000000, 0.800000), Cocktail("a", 8.000000, 0.300000)}
*/

// clang-format on
