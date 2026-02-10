#pragma once
// Step 58: Mode-specific editor behavior
//
// EditorMode provides language-aware editing features:
// - Auto-indent rules per language
// - Comment toggling per language
// - Bracket/quote auto-close per language
// - Language-specific snippet templates
//
// The editor queries the active EditorMode to determine behavior
// when the user types or performs editing operations.

#include <string>
#include <vector>
#include <map>

struct IndentRule {
    std::string increasePattern;  // regex-like pattern that increases indent
    std::string decreasePattern;  // regex-like pattern that decreases indent
    int tabSize = 4;
    bool useTabs = false;
};

struct CommentStyle {
    std::string lineComment;    // e.g. "//", "#", ";;"
    std::string blockStart;     // e.g. "/*", "\"\"\""
    std::string blockEnd;       // e.g. "*/", "\"\"\""
};

struct BracketPair {
    char open;
    char close;
};

struct SnippetTemplate {
    std::string trigger;      // e.g. "def", "for", "if"
    std::string expansion;    // template with $1, $2 placeholders
    std::string description;
};

enum class EditorModeType {
    Python,
    Cpp,
    Elisp,
    JavaScript,
    TypeScript,
    Java,
    Rust,
    Go,
    Org,
    PlainText
};

class EditorMode {
public:
    explicit EditorMode(const std::string& language = "python") {
        setLanguage(language);
    }

    void setLanguage(const std::string& language) {
        language_ = language;
        if (language == "python")     loadPython();
        else if (language == "cpp")   loadCpp();
        else if (language == "elisp") loadElisp();
        else if (language == "javascript") loadJavaScript();
        else if (language == "typescript") loadTypeScript();
        else if (language == "java") loadJava();
        else if (language == "rust") loadRust();
        else if (language == "go") loadGo();
        else if (language == "org") loadOrg();
        else                          loadPlainText();
    }

    std::string getLanguage() const { return language_; }
    EditorModeType getType() const { return type_; }

    // --- Indentation ---

    const IndentRule& getIndentRule() const { return indent_; }

    // Should we increase indent after this line?
    bool shouldIndentAfter(const std::string& line) const {
        if (indent_.increasePattern.empty()) return false;
        // Simple pattern matching (not full regex)
        return lineEndsWith(line, indent_.increasePattern);
    }

    // Should we decrease indent for this line?
    bool shouldDedent(const std::string& line) const {
        if (indent_.decreasePattern.empty()) return false;
        return trimmedLineStartsWith(line, indent_.decreasePattern);
    }

    // Get indent string (tabs or spaces)
    std::string getIndentString() const {
        if (indent_.useTabs) return "\t";
        return std::string(indent_.tabSize, ' ');
    }

    int getTabSize() const { return indent_.tabSize; }
    void setTabSize(int size) { indent_.tabSize = size; }

    // --- Comments ---

    const CommentStyle& getCommentStyle() const { return comment_; }

    // Toggle line comment: add or remove line comment prefix
    std::string toggleLineComment(const std::string& line) const {
        if (comment_.lineComment.empty()) return line;

        std::string trimmed = trimLeft(line);
        std::string prefix = comment_.lineComment + " ";

        if (trimmed.substr(0, prefix.size()) == prefix) {
            // Remove comment
            size_t commentPos = line.find(prefix);
            return line.substr(0, commentPos) + line.substr(commentPos + prefix.size());
        } else if (trimmed.substr(0, comment_.lineComment.size()) == comment_.lineComment) {
            // Remove comment without space
            size_t commentPos = line.find(comment_.lineComment);
            return line.substr(0, commentPos) + line.substr(commentPos + comment_.lineComment.size());
        } else {
            // Add comment at the start of content (preserving indentation)
            size_t firstNonSpace = line.find_first_not_of(" \t");
            if (firstNonSpace == std::string::npos)
                return line;  // empty/whitespace line
            return line.substr(0, firstNonSpace) + prefix + line.substr(firstNonSpace);
        }
    }

    // --- Auto-close brackets ---

    const std::vector<BracketPair>& getBracketPairs() const { return brackets_; }

    // Given an opening character, return the closing character (0 if not a bracket)
    char getClosingBracket(char open) const {
        for (const auto& bp : brackets_) {
            if (bp.open == open) return bp.close;
        }
        return 0;
    }

    // --- Snippets ---

    const std::vector<SnippetTemplate>& getSnippets() const { return snippets_; }

    // Find snippet by trigger
    const SnippetTemplate* findSnippet(const std::string& trigger) const {
        for (const auto& s : snippets_) {
            if (s.trigger == trigger) return &s;
        }
        return nullptr;
    }

    // --- Static helpers ---

    static const char* modeName(EditorModeType t) {
        switch (t) {
            case EditorModeType::Python:    return "Python";
            case EditorModeType::Cpp:       return "C++";
            case EditorModeType::Elisp:     return "Elisp";
            case EditorModeType::JavaScript:return "JavaScript";
            case EditorModeType::TypeScript:return "TypeScript";
            case EditorModeType::Java:      return "Java";
            case EditorModeType::Rust:      return "Rust";
            case EditorModeType::Go:        return "Go";
            case EditorModeType::Org:       return "Org";
            case EditorModeType::PlainText: return "Plain Text";
        }
        return "Unknown";
    }

    static EditorModeType modeFromLanguage(const std::string& lang) {
        if (lang == "python") return EditorModeType::Python;
        if (lang == "cpp")    return EditorModeType::Cpp;
        if (lang == "elisp")  return EditorModeType::Elisp;
        if (lang == "javascript") return EditorModeType::JavaScript;
        if (lang == "typescript") return EditorModeType::TypeScript;
        if (lang == "java") return EditorModeType::Java;
        if (lang == "rust") return EditorModeType::Rust;
        if (lang == "go") return EditorModeType::Go;
        if (lang == "org") return EditorModeType::Org;
        return EditorModeType::PlainText;
    }

private:
    static std::string trimLeft(const std::string& s) {
        size_t start = s.find_first_not_of(" \t");
        return (start == std::string::npos) ? "" : s.substr(start);
    }

    static bool lineEndsWith(const std::string& line, const std::string& suffix) {
        std::string trimmed = line;
        // Remove trailing whitespace
        while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t'))
            trimmed.pop_back();
        if (trimmed.size() < suffix.size()) return false;
        return trimmed.substr(trimmed.size() - suffix.size()) == suffix;
    }

    static bool trimmedLineStartsWith(const std::string& line, const std::string& prefix) {
        std::string trimmed = trimLeft(line);
        return trimmed.substr(0, prefix.size()) == prefix;
    }

    void loadPython() {
        type_ = EditorModeType::Python;
        indent_ = {":", "return", 4, false};
        comment_ = {"#", "\"\"\"", "\"\"\""};
        brackets_ = {{'(', ')'}, {'[', ']'}, {'{', '}'}, {'\'', '\''}, {'"', '"'}};
        snippets_ = {
            {"def",   "def $1($2):\n    $0",           "Function definition"},
            {"class", "class $1:\n    def __init__(self):\n        $0", "Class definition"},
            {"if",    "if $1:\n    $0",                 "If statement"},
            {"for",   "for $1 in $2:\n    $0",          "For loop"},
            {"while", "while $1:\n    $0",              "While loop"},
            {"try",   "try:\n    $0\nexcept $1:\n    pass", "Try/except block"},
        };
    }

    void loadCpp() {
        type_ = EditorModeType::Cpp;
        indent_ = {"{", "}", 4, false};
        comment_ = {"//", "/*", "*/"};
        brackets_ = {{'(', ')'}, {'[', ']'}, {'{', '}'}, {'\'', '\''}, {'"', '"'}, {'<', '>'}};
        snippets_ = {
            {"main",   "int main(int argc, char* argv[]) {\n    $0\n    return 0;\n}", "Main function"},
            {"class",  "class $1 {\npublic:\n    $1();\n    ~$1();\nprivate:\n    $0\n};", "Class definition"},
            {"if",     "if ($1) {\n    $0\n}",           "If statement"},
            {"for",    "for (int $1 = 0; $1 < $2; ++$1) {\n    $0\n}", "For loop"},
            {"while",  "while ($1) {\n    $0\n}",        "While loop"},
            {"switch", "switch ($1) {\n    case $2:\n        $0\n        break;\n    default:\n        break;\n}", "Switch statement"},
            {"inc",    "#include <$1>",                   "Include directive"},
        };
    }

    void loadElisp() {
        type_ = EditorModeType::Elisp;
        indent_ = {"(", ")", 2, false};
        comment_ = {";;", "", ""};
        brackets_ = {{'(', ')'}, {'[', ']'}, {'"', '"'}};
        snippets_ = {
            {"defun",  "(defun $1 ($2)\n  \"$3\"\n  $0)", "Function definition"},
            {"let",    "(let (($1 $2))\n  $0)",            "Let binding"},
            {"if",     "(if $1\n    $2\n  $0)",            "If expression"},
            {"cond",   "(cond\n  (($1) $2)\n  (t $0))",    "Cond expression"},
            {"lambda", "(lambda ($1) $0)",                  "Lambda expression"},
        };
    }

    void loadPlainText() {
        type_ = EditorModeType::PlainText;
        indent_ = {"", "", 4, false};
        comment_ = {"", "", ""};
        brackets_ = {{'(', ')'}, {'[', ']'}, {'{', '}'}, {'"', '"'}};
        snippets_ = {};
    }

    void loadJavaScript() {
        type_ = EditorModeType::JavaScript;
        indent_ = {"{", "}", 2, false};
        comment_ = {"//", "/*", "*/"};
        brackets_ = {{'(', ')'}, {'[', ']'}, {'{', '}'}, {'\'', '\''}, {'"', '"'}};
        snippets_ = {
            {"fn", "function $1($2) {\\n  $0\\n}", "Function"},
            {"if", "if ($1) {\\n  $0\\n}", "If statement"},
            {"for", "for (let $1 = 0; $1 < $2; $1++) {\\n  $0\\n}", "For loop"},
        };
    }

    void loadTypeScript() {
        type_ = EditorModeType::TypeScript;
        indent_ = {"{", "}", 2, false};
        comment_ = {"//", "/*", "*/"};
        brackets_ = {{'(', ')'}, {'[', ']'}, {'{', '}'}, {'\'', '\''}, {'"', '"'}, {'<', '>'}};
        snippets_ = {
            {"fn", "function $1($2): $3 {\\n  $0\\n}", "Function"},
            {"if", "if ($1) {\\n  $0\\n}", "If statement"},
        };
    }

    void loadJava() {
        type_ = EditorModeType::Java;
        indent_ = {"{", "}", 4, false};
        comment_ = {"//", "/*", "*/"};
        brackets_ = {{'(', ')'}, {'[', ']'}, {'{', '}'}, {'\'', '\''}, {'"', '"'}};
        snippets_ = {
            {"class", "class $1 {\\n    $0\\n}", "Class"},
            {"if", "if ($1) {\\n    $0\\n}", "If statement"},
        };
    }

    void loadRust() {
        type_ = EditorModeType::Rust;
        indent_ = {"{", "}", 4, false};
        comment_ = {"//", "/*", "*/"};
        brackets_ = {{'(', ')'}, {'[', ']'}, {'{', '}'}, {'\'', '\''}, {'\"', '\"'}};
        snippets_ = {
            {"fn", "fn $1($2) -> $3 {\\n    $0\\n}", "Function"},
            {"if", "if $1 {\\n    $0\\n}", "If statement"},
        };
    }

    void loadGo() {
        type_ = EditorModeType::Go;
        indent_ = {"{", "}", 4, false};
        comment_ = {"//", "/*", "*/"};
        brackets_ = {{'(', ')'}, {'[', ']'}, {'{', '}'}, {'\'', '\''}, {'\"', '\"'}};
        snippets_ = {
            {"fn", "func $1($2) $3 {\\n    $0\\n}", "Function"},
            {"if", "if $1 {\\n    $0\\n}", "If statement"},
        };
    }

    void loadOrg() {
        type_ = EditorModeType::Org;
        indent_ = {"", "", 2, false};
        comment_ = {"#", "", ""};
        brackets_ = {{'(', ')'}, {'[', ']'}, {'{', '}'}, {'\"', '\"'}};
        snippets_ = {
            {"src", "#+begin_src $1\\n$0\\n#+end_src", "Source block"},
            {"hdr", "* $1\\n$0", "Heading"},
        };
    }

    std::string language_;
    EditorModeType type_;
    IndentRule indent_;
    CommentStyle comment_;
    std::vector<BracketPair> brackets_;
    std::vector<SnippetTemplate> snippets_;
};
