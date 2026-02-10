#pragma once
#include <string>
#include <unordered_map>

static inline std::string mapTypeForFunctionCall(const std::string& targetLanguage,
                                                 const std::string& functionName) {
    if (targetLanguage == "cpp") {
        static const std::unordered_map<std::string, std::string> cppMap = {
            {"numpy.array", "std::vector<double>"},
            {"numpy.zeros", "std::vector<double>"},
            {"numpy.ones", "std::vector<double>"},
            {"pandas.DataFrame", "std::vector<std::vector<double>>"},
            {"torch.tensor", "std::vector<float>"}
        };
        auto it = cppMap.find(functionName);
        if (it != cppMap.end()) return it->second;
    }
    if (targetLanguage == "rust") {
        static const std::unordered_map<std::string, std::string> rustMap = {
            {"numpy.array", "Vec<f64>"},
            {"numpy.zeros", "Vec<f64>"},
            {"numpy.ones", "Vec<f64>"}
        };
        auto it = rustMap.find(functionName);
        if (it != rustMap.end()) return it->second;
    }
    return "";
}
