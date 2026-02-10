#pragma once
// Language keyword helpers for SyntaxHighlighter.

    static bool isPythonKeyword(const std::string& text) {
        static const char* keywords[] = {
            "def", "class", "if", "elif", "else", "for", "while", "return",
            "import", "from", "as", "try", "except", "finally", "raise",
            "with", "yield", "lambda", "pass", "break", "continue",
            "and", "or", "not", "in", "is", "del", "global", "nonlocal",
            "assert", "async", "await", nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }
    static bool isPythonBuiltin(const std::string& text) {
        static const char* builtins[] = {
            "True", "False", "None", "print", "len", "range", "int",
            "str", "float", "list", "dict", "set", "tuple", "type",
            "isinstance", "super", "self", nullptr
        };
        for (const char** b = builtins; *b; ++b) {
            if (text == *b) return true;
        }
        return false;
    }
    static bool isCppKeyword(const std::string& text) {
        static const char* keywords[] = {
            "auto", "break", "case", "catch", "class", "const", "constexpr",
            "continue", "default", "delete", "do", "else", "enum", "explicit",
            "extern", "for", "friend", "goto", "if", "inline", "mutable",
            "namespace", "new", "noexcept", "operator", "private", "protected",
            "public", "register", "return", "sizeof", "static", "static_assert",
            "static_cast", "struct", "switch", "template", "this", "throw",
            "try", "typedef", "typeid", "typename", "union", "using",
            "virtual", "volatile", "while", "override", "final",
            "co_await", "co_return", "co_yield", "concept", "requires",
            "consteval", "constinit", nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }
    static bool isCppType(const std::string& text) {
        static const char* types[] = {
            "void", "bool", "char", "int", "float", "double", "long",
            "short", "unsigned", "signed", "size_t", "nullptr_t",
            "int8_t", "int16_t", "int32_t", "int64_t",
            "uint8_t", "uint16_t", "uint32_t", "uint64_t",
            "string", "vector", "map", "set", "array", "tuple",
            "unique_ptr", "shared_ptr", "weak_ptr", "optional",
            nullptr
        };
        for (const char** t = types; *t; ++t) {
            if (text == *t) return true;
        }
        return false;
    }
    static bool isJsKeyword(const std::string& text) {
        static const char* keywords[] = {
            "function", "return", "if", "else", "for", "while", "class",
            "const", "let", "var", "import", "export", "new", "try", "catch",
            "finally", "switch", "case", "break", "continue", "throw",
            "async", "await", "yield", nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }
    static bool isJavaKeyword(const std::string& text) {
        static const char* keywords[] = {
            "class", "interface", "enum", "public", "private", "protected",
            "static", "final", "void", "int", "float", "double", "boolean",
            "return", "if", "else", "for", "while", "switch", "case",
            "break", "continue", "new", "try", "catch", "finally", "throw",
            "extends", "implements", "import", "package", "this", "super",
            nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }
    static bool isRustKeyword(const std::string& text) {
        static const char* keywords[] = {
            "fn", "let", "mut", "pub", "impl", "trait", "struct", "enum",
            "use", "mod", "crate", "self", "super", "return", "if", "else",
            "for", "while", "loop", "match", "break", "continue", "const",
            "static", "async", "await", "move", nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }
    static bool isGoKeyword(const std::string& text) {
        static const char* keywords[] = {
            "func", "package", "import", "return", "if", "else", "for",
            "switch", "case", "break", "continue", "struct", "interface",
            "type", "var", "const", "go", "defer", "range", nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }
    static bool isElispKeyword(const std::string& text) {
        static const char* keywords[] = {
            "defun", "defvar", "defconst", "defmacro", "defcustom",
            "let", "let*", "if", "when", "unless", "cond", "while",
            "dolist", "dotimes", "progn", "prog1", "prog2",
            "lambda", "setq", "setf", "require", "provide",
            "interactive", "save-excursion", "save-restriction",
            "condition-case", "unwind-protect", "catch", "throw",
            nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }
