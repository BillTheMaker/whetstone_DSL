#pragma once
// Step 570: Watch Expression Evaluator Hardening

#include <cctype>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct WatchEvalLimits {
    std::size_t maxExpressionLength = 256;
    std::size_t maxTokens = 64;
    std::size_t maxOperations = 32;
    std::uint64_t timeoutBudgetMs = 32;
};

struct WatchEvalResult {
    bool ok = false;
    bool timedOut = false;
    bool rejectedUnsafe = false;
    std::int64_t value = 0;
    std::size_t operations = 0;
    std::string error;
};

class WatchExpressionEvaluatorHardening {
public:
    static WatchEvalResult evaluate(const std::string& expression,
                                    const std::map<std::string, std::int64_t>& scope,
                                    const WatchEvalLimits& limits = {}) {
        WatchEvalResult result;
        if (expression.empty()) {
            fail(result, "expression_empty");
            return result;
        }
        if (expression.size() > limits.maxExpressionLength) {
            fail(result, "expression_too_long");
            return result;
        }
        if (containsUnsafePattern(expression)) {
            result.rejectedUnsafe = true;
            fail(result, "expression_unsafe");
            return result;
        }

        std::vector<std::string> tokens;
        if (!tokenize(expression, limits.maxTokens, &tokens, &result.error)) return result;
        if (tokens.empty()) {
            fail(result, "expression_empty");
            return result;
        }

        std::vector<std::int64_t> values;
        std::vector<char> operators;
        for (const auto& token : tokens) {
            if (isOperatorToken(token)) {
                operators.push_back(token[0]);
            } else {
                std::int64_t value = 0;
                if (!parseValue(token, scope, &value, &result.error)) return result;
                values.push_back(value);
            }
        }
        if (values.empty()) {
            fail(result, "value_missing");
            return result;
        }
        if (values.size() != operators.size() + 1) {
            fail(result, "operator_operand_mismatch");
            return result;
        }

        if (!applyOperations(values, operators, &result, limits)) return result;
        result.ok = true;
        return result;
    }

private:
    static bool fail(WatchEvalResult& result, const char* code) {
        result.error = code;
        result.ok = false;
        return false;
    }

    static bool containsUnsafePattern(const std::string& expression) {
        static const char* badSubstrings[] = {
            ";", "{", "}", "[", "]", "__", "system", "exec", "include", "while", "for("
        };
        for (const auto* pattern : badSubstrings) {
            if (expression.find(pattern) != std::string::npos) return true;
        }
        return false;
    }

    static bool tokenize(const std::string& expression,
                         std::size_t maxTokens,
                         std::vector<std::string>* outTokens,
                         std::string* error) {
        std::string current;
        auto flushCurrent = [&]() {
            if (!current.empty()) {
                outTokens->push_back(current);
                current.clear();
            }
        };

        for (char c : expression) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                flushCurrent();
                continue;
            }
            if (c == '+' || c == '-' || c == '*' || c == '/') {
                flushCurrent();
                outTokens->push_back(std::string(1, c));
            } else if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                current.push_back(c);
            } else {
                *error = "token_invalid_character";
                return false;
            }
            if (outTokens->size() > maxTokens) {
                *error = "token_limit_exceeded";
                return false;
            }
        }
        flushCurrent();
        if (outTokens->size() > maxTokens) {
            *error = "token_limit_exceeded";
            return false;
        }
        return true;
    }

    static bool isOperatorToken(const std::string& token) {
        return token.size() == 1 &&
               (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/');
    }

    static bool parseValue(const std::string& token,
                           const std::map<std::string, std::int64_t>& scope,
                           std::int64_t* outValue,
                           std::string* error) {
        if (token.empty()) {
            *error = "value_missing";
            return false;
        }
        bool numeric = true;
        for (char c : token) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                numeric = false;
                break;
            }
        }
        if (numeric) {
            *outValue = std::stoll(token);
            return true;
        }
        auto it = scope.find(token);
        if (it == scope.end()) {
            *error = "identifier_unknown";
            return false;
        }
        *outValue = it->second;
        return true;
    }

    static bool applyOperations(std::vector<std::int64_t> values,
                                std::vector<char> operators,
                                WatchEvalResult* result,
                                const WatchEvalLimits& limits) {
        std::vector<std::int64_t> reducedValues;
        std::vector<char> reducedOperators;
        reducedValues.push_back(values[0]);

        for (std::size_t i = 0; i < operators.size(); ++i) {
            const char op = operators[i];
            const std::int64_t rhs = values[i + 1];
            if (op == '*' || op == '/') {
                const std::int64_t lhs = reducedValues.back();
                if (op == '/' && rhs == 0) return fail(*result, "division_by_zero");
                reducedValues.back() = op == '*' ? lhs * rhs : lhs / rhs;
                if (!countOperation(result, limits)) return false;
            } else {
                reducedValues.push_back(rhs);
                reducedOperators.push_back(op);
            }
        }

        std::int64_t acc = reducedValues[0];
        for (std::size_t i = 0; i < reducedOperators.size(); ++i) {
            const char op = reducedOperators[i];
            const std::int64_t rhs = reducedValues[i + 1];
            acc = op == '+' ? acc + rhs : acc - rhs;
            if (!countOperation(result, limits)) return false;
        }

        result->value = acc;
        return true;
    }

    static bool countOperation(WatchEvalResult* result, const WatchEvalLimits& limits) {
        ++result->operations;
        if (result->operations > limits.maxOperations) return fail(*result, "operation_limit_exceeded");
        if (result->operations > limits.timeoutBudgetMs) {
            result->timedOut = true;
            return fail(*result, "evaluation_timeout");
        }
        return true;
    }
};
