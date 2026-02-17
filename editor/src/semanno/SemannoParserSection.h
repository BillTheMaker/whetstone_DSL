
// ---------------------------------------------------------------------------
// SemannoParser
// ---------------------------------------------------------------------------

struct SemannoEntry {
    std::string type;                             // e.g. "intent"
    std::map<std::string, std::string> properties; // key→raw value string
};

class SemannoParser {
public:
    /// Check whether a line contains a Semanno annotation.
    static bool isSemannoComment(const std::string& line) {
        return line.find("@semanno:") != std::string::npos;
    }

    /// Parse a single Semanno comment line.
    /// Accepts lines with any comment prefix: "//", "#", "/*", "--", etc.
    /// Returns a SemannoEntry with type and key-value properties.
    static SemannoEntry parse(const std::string& line) {
        SemannoEntry entry;

        // Locate the "@semanno:" marker
        auto pos = line.find("@semanno:");
        if (pos == std::string::npos) return entry;

        pos += 9;  // skip past "@semanno:"

        // Extract the type name (until '(' or end of relevant content)
        size_t typeEnd = pos;
        while (typeEnd < line.size() && line[typeEnd] != '(' &&
               line[typeEnd] != ')' && line[typeEnd] != ' ' &&
               line[typeEnd] != '\t' && line[typeEnd] != '\n' &&
               line[typeEnd] != '\r') {
            ++typeEnd;
        }
        entry.type = line.substr(pos, typeEnd - pos);

        // Strip any trailing comment close markers from type (e.g. "*/" )
        while (!entry.type.empty() &&
               (entry.type.back() == '*' || entry.type.back() == '/')) {
            entry.type.pop_back();
        }

        // If there is no property block, we are done.
        if (typeEnd >= line.size() || line[typeEnd] != '(') return entry;

        // Find the matching closing paren (respecting escaped quotes).
        size_t propStart = typeEnd + 1;
        size_t propEnd = findMatchingParen(line, propStart);
        if (propEnd == std::string::npos) propEnd = line.size();

        std::string propBlock = line.substr(propStart, propEnd - propStart);
        parseProperties(propBlock, entry.properties);

        return entry;
    }

private:
    /// Find the closing ')' that matches an opening '(' at `start`,
    /// respecting quoted strings.
    static size_t findMatchingParen(const std::string& s, size_t start) {
        bool inQuote = false;
        for (size_t i = start; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                ++i;  // skip escaped char
                continue;
            }
            if (s[i] == '"') {
                inQuote = !inQuote;
            } else if (s[i] == ')' && !inQuote) {
                return i;
            }
        }
        return std::string::npos;
    }

    /// Parse "key=\"value\",key2=value2,..." into a map.
    static void parseProperties(const std::string& block,
                                std::map<std::string, std::string>& props) {
        size_t i = 0;
        while (i < block.size()) {
            // Skip whitespace
            while (i < block.size() && (block[i] == ' ' || block[i] == '\t'))
                ++i;
            if (i >= block.size()) break;

            // Read key (up to '=')
            size_t keyStart = i;
            while (i < block.size() && block[i] != '=') ++i;
            if (i >= block.size()) break;
            std::string key = block.substr(keyStart, i - keyStart);
            // Trim trailing whitespace from key
            while (!key.empty() && (key.back() == ' ' || key.back() == '\t'))
                key.pop_back();
            ++i;  // skip '='

            // Skip whitespace after '='
            while (i < block.size() && (block[i] == ' ' || block[i] == '\t'))
                ++i;

            std::string value;
            if (i < block.size() && block[i] == '"') {
                // Quoted value — read until unescaped closing '"'
                ++i;  // skip opening '"'
                while (i < block.size()) {
                    if (block[i] == '\\' && i + 1 < block.size()) {
                        value += block[i];
                        value += block[i + 1];
                        i += 2;
                    } else if (block[i] == '"') {
                        ++i;  // skip closing '"'
                        break;
                    } else {
                        value += block[i];
                        ++i;
                    }
                }
                // Unescape the value
                value = semanno_detail::unescapeValue(value);
            } else {
                // Unquoted value — read until ',' or end
                size_t valStart = i;
                while (i < block.size() && block[i] != ',') ++i;
                value = block.substr(valStart, i - valStart);
                // Trim trailing whitespace
                while (!value.empty() &&
                       (value.back() == ' ' || value.back() == '\t'))
                    value.pop_back();
            }

            props[key] = value;

            // Skip comma separator
            if (i < block.size() && block[i] == ',') ++i;
        }
    }
};

// end of SemannoFormat.h
