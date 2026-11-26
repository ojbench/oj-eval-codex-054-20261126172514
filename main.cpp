#include <array>
#include <cctype>
#include <iostream>
#include <map>
#include <string>
#include <utility>

using Powers = std::array<int, 3>;
using Integer = long long;

class Polynomial {
public:
    std::map<Powers, Integer, std::greater<Powers>> terms;

    explicit Polynomial(Integer constant = 0) {
        add({0, 0, 0}, constant);
    }

    void add(const Powers& powers, Integer coefficient) {
        if (coefficient == 0) return;
        auto [it, inserted] = terms.try_emplace(powers, coefficient);
        if (!inserted) it->second += coefficient;
        if (it->second == 0) terms.erase(it);
    }

    Polynomial derivative() const {
        Polynomial result;
        for (const auto& [powers, coefficient] : terms) {
            for (int i = 0; i < 3; ++i) {
                if (powers[i] == 0) continue;
                Powers next = powers;
                --next[i];
                if (i == 1) ++next[2];
                if (i == 2) ++next[1];
                result.add(next, coefficient * powers[i] * (i == 2 ? -1 : 1));
            }
        }
        return result;
    }

    bool isOne() const {
        return terms.size() == 1 && terms.begin()->first == Powers{0, 0, 0}
               && terms.begin()->second == 1;
    }

    std::string str() const {
        if (terms.empty()) return "0";
        std::string result;
        for (const auto& [powers, coefficient] : terms) {
            if (coefficient < 0) result += '-';
            else if (!result.empty()) result += '+';
            Integer magnitude = coefficient < 0 ? -coefficient : coefficient;
            if (magnitude != 1 || powers == Powers{0, 0, 0})
                result += std::to_string(magnitude);
            for (int i = 0; i < 3; ++i) {
                if (powers[i] == 0) continue;
                if (i == 1) result += "sin";
                if (i == 2) result += "cos";
                if (i == 0) result += 'x';
                if (powers[i] != 1) result += '^' + std::to_string(powers[i]);
                if (i != 0) result += 'x';
            }
        }
        return result;
    }
};

Polynomial combine(Polynomial left, const Polynomial& right, int sign = 1) {
    for (const auto& [powers, coefficient] : right.terms)
        left.add(powers, sign * coefficient);
    return left;
}

Polynomial operator*(const Polynomial& left, const Polynomial& right) {
    Polynomial result;
    for (const auto& [a, ca] : left.terms)
        for (const auto& [b, cb] : right.terms)
            result.add({a[0] + b[0], a[1] + b[1], a[2] + b[2]}, ca * cb);
    return result;
}

struct Fraction {
    Polynomial numerator;
    Polynomial denominator{1};

    Fraction derivative() const {
        return {combine(numerator.derivative() * denominator,
                        denominator.derivative() * numerator, -1),
                denominator * denominator};
    }

    std::string str() const {
        if (numerator.terms.empty()) return "0";
        std::string top = numerator.str();
        if (denominator.isOne()) return top;
        std::string bottom = denominator.str();
        if (numerator.terms.size() > 1) top = '(' + top + ')';
        if (denominator.terms.size() > 1) bottom = '(' + bottom + ')';
        return top + '/' + bottom;
    }
};

Fraction operate(const Fraction& left, const Fraction& right, char op) {
    if (op == '*') return {left.numerator * right.numerator,
                           left.denominator * right.denominator};
    if (op == '/') return {left.numerator * right.denominator,
                           left.denominator * right.numerator};
    return {combine(left.numerator * right.denominator,
                    right.numerator * left.denominator, op == '+' ? 1 : -1),
            left.denominator * right.denominator};
}

class Parser {
    const std::string& input;
    std::size_t position = 0;

    char peek() const { return position < input.size() ? input[position] : '\0'; }

    int number() {
        int value = 0;
        while (std::isdigit(static_cast<unsigned char>(peek())))
            value = value * 10 + input[position++] - '0';
        return value;
    }

    Fraction atom() {
        int sign = 1;
        while (peek() == '-' || peek() == '+') {
            if (peek() == '-') sign = -sign;
            ++position;
        }
        if (peek() == '(') {
            ++position;
            Fraction result = expression();
            ++position; // Consume the matching closing parenthesis.
            if (sign == -1)
                for (auto& term : result.numerator.terms) term.second = -term.second;
            return result;
        }
        Integer coefficient = sign;
        if (std::isdigit(static_cast<unsigned char>(peek()))) coefficient *= number();
        Powers powers{0, 0, 0};
        // Juxtaposed factors form one monomial, including in a divisor like -6x.
        while (peek() == 'x' || peek() == 's' || peek() == 'c') {
            int index = peek() == 'x' ? 0 : (peek() == 's' ? 1 : 2);
            position += index == 0 ? 1 : 3;
            int exponent = 1;
            if (peek() == '^') {
                ++position;
                exponent = number();
            }
            if (index != 0) ++position; // The x in sin^n x or cos^n x.
            powers[index] += exponent;
        }
        Polynomial result;
        result.add(powers, coefficient);
        return {std::move(result), Polynomial(1)};
    }

    Fraction product() {
        Fraction result = atom();
        while (peek() == '*' || peek() == '/') {
            char op = input[position++];
            result = operate(result, atom(), op);
        }
        return result;
    }

public:
    explicit Parser(const std::string& text) : input(text) {}

    Fraction expression() {
        Fraction result = product();
        while (peek() == '+' || peek() == '-') {
            char op = input[position++];
            result = operate(result, product(), op);
        }
        return result;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::string input, line;
    std::getline(std::cin, line);
    for (unsigned char ch : line)
        if (!std::isspace(ch)) input += static_cast<char>(ch);
    if (input.empty()) return 0;
    Fraction result = Parser(input).expression();
    std::cout << result.str() << '\n' << result.derivative().str() << '\n';
}
