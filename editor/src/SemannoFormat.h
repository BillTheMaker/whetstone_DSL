#pragma once
// SemannoFormat.h — Semanno inline comment format standard
//
// Format:  @semanno:type(key=value,key2=value2)
//
// SemannoEmitter  — converts annotation ASTNodes to Semanno comment strings
// SemannoParser   — parses Semanno comment strings back to structured data
//
// Covers all 67+ annotation types defined in Annotation.h and EnvironmentSpec.h.

#include "ast/ASTNode.h"
#include "ast/Annotation.h"
#include "EnvironmentSpec.h"

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace semanno_detail {

inline std::string escapeValue(const std::string& v) {
    std::string out;
    out.reserve(v.size() + 2);
    for (char c : v) {
        if (c == '"')       out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else                out += c;
    }
    return out;
}

inline std::string unescapeValue(const std::string& v) {
    std::string out;
    out.reserve(v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i] == '\\' && i + 1 < v.size()) {
            if (v[i + 1] == '"')       { out += '"';  ++i; }
            else if (v[i + 1] == '\\') { out += '\\'; ++i; }
            else                        out += v[i];
        } else {
            out += v[i];
        }
    }
    return out;
}

inline std::string joinVec(const std::vector<std::string>& vec) {
    std::string out;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) out += ";";
        out += escapeValue(vec[i]);
    }
    return out;
}

inline std::vector<std::string> splitVec(const std::string& s) {
    std::vector<std::string> out;
    if (s.empty()) return out;
    std::string cur;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            cur += s[i];
            cur += s[i + 1];
            ++i;
        } else if (s[i] == ';') {
            out.push_back(unescapeValue(cur));
            cur.clear();
        } else {
            cur += s[i];
        }
    }
    out.push_back(unescapeValue(cur));
    return out;
}

// Build "key=\"value\"" pair; omits the pair entirely when value is empty.
inline void appendProp(std::string& buf, const std::string& key,
                       const std::string& val, bool& first) {
    if (val.empty()) return;
    if (!first) buf += ",";
    first = false;
    buf += key + "=\"" + escapeValue(val) + "\"";
}

inline void appendInt(std::string& buf, const std::string& key,
                      int val, bool& first) {
    if (!first) buf += ",";
    first = false;
    buf += key + "=" + std::to_string(val);
}

inline void appendBool(std::string& buf, const std::string& key,
                       bool val, bool& first) {
    if (!first) buf += ",";
    first = false;
    buf += key + "=" + (val ? "true" : "false");
}

inline void appendVec(std::string& buf, const std::string& key,
                      const std::vector<std::string>& vec, bool& first) {
    if (vec.empty()) return;
    if (!first) buf += ",";
    first = false;
    buf += key + "=\"" + joinVec(vec) + "\"";
}

} // namespace semanno_detail

// ---------------------------------------------------------------------------
// SemannoEmitter
// ---------------------------------------------------------------------------

class SemannoEmitter {
public:
    /// Returns "@semanno:type(key=value,...)" or empty string if unrecognised.
#include "semanno/SemannoEmitterBody.h"
#include "semanno/SemannoParserSection.h"
