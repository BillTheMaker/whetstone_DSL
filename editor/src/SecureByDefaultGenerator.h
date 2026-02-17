#pragma once

// Step 489: Secure-by-Default Code Generation
// Emits secure templates and defaults for common risky operations.

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

enum class SecureSnippetKind {
    SqlQuery,
    InputHandling,
    FileOperation,
    NetworkCall,
    CryptoOperation
};

struct SecureGenerationRequest {
    std::string language;
    SecureSnippetKind kind = SecureSnippetKind::InputHandling;
    std::string operationName;
    std::string securityRequirement;
};

struct SecureGenerationOutput {
    std::string code;
    std::vector<std::string> annotations;
    std::vector<std::string> notes;
    bool requiresHumanReview = false;
};

class SecureByDefaultGenerator {
public:
    static SecureGenerationOutput generate(const SecureGenerationRequest& req) {
        SecureGenerationOutput out;
        const std::string lang = lower(req.language);

        out.annotations.push_back("@Risk(security)");
        out.annotations.push_back("@Review(required, human)");
        if (!req.securityRequirement.empty()) {
            out.annotations.push_back("@SecurityRequirement(" + req.securityRequirement + ")");
        }

        switch (req.kind) {
            case SecureSnippetKind::SqlQuery:
                out.code = generateSql(lang, req.operationName);
                out.notes.push_back("Uses parameterized query placeholders");
                break;
            case SecureSnippetKind::InputHandling:
                out.code = generateInputHandling(lang, req.operationName);
                out.annotations.push_back("@InputValidation(raw)");
                out.notes.push_back("Input marked raw until explicit sanitization");
                break;
            case SecureSnippetKind::FileOperation:
                out.code = generateFileOperation(lang, req.operationName);
                out.notes.push_back("Path traversal checks included");
                break;
            case SecureSnippetKind::NetworkCall:
                out.code = generateNetworkCall(lang, req.operationName);
                out.notes.push_back("Timeout and TLS verification included");
                break;
            case SecureSnippetKind::CryptoOperation:
                out.code = generateCrypto(lang, req.operationName, out.requiresHumanReview);
                out.annotations.push_back("@Automatability(human)");
                out.notes.push_back("Crypto generated with modern algorithm defaults");
                break;
        }

        if (contains(lower(out.code), "md5") || contains(lower(out.code), "sha1")) {
            out.requiresHumanReview = true;
            out.notes.push_back("Weak crypto keyword detected; escalation required");
        }
        return out;
    }

private:
    static std::string lower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static bool contains(const std::string& text, const std::string& needle) {
        return text.find(needle) != std::string::npos;
    }

    static std::string opName(const std::string& n, const std::string& fallback) {
        return n.empty() ? fallback : n;
    }

    static std::string generateSql(const std::string& lang, const std::string& op) {
        const std::string f = opName(op, "fetchRecord");
        if (lang == "python") {
            return "def " + f + "(conn, user_id):\n"
                   "    query = \"SELECT * FROM users WHERE id = %s\"\n"
                   "    return conn.execute(query, (user_id,)).fetchone()\n";
        }
        if (lang == "typescript" || lang == "javascript") {
            return "export async function " + f + "(db, userId) {\n"
                   "  return db.query(\"SELECT * FROM users WHERE id = ?\", [userId]);\n"
                   "}\n";
        }
        if (lang == "rust") {
            return "pub async fn " + f + "(pool: &DbPool, user_id: i64) -> Result<Row, Error> {\n"
                   "    sqlx::query(\"SELECT * FROM users WHERE id = $1\").bind(user_id).fetch_one(pool).await\n"
                   "}\n";
        }
        return "STUB: use parameterized query for " + f + "\n";
    }

    static std::string generateInputHandling(const std::string& lang, const std::string& op) {
        const std::string f = opName(op, "handleInput");
        if (lang == "python") {
            return "def " + f + "(raw_input: str):\n"
                   "    # raw until sanitized\n"
                   "    return raw_input\n";
        }
        if (lang == "typescript" || lang == "javascript") {
            return "export function " + f + "(rawInput: string) {\n"
                   "  // raw until sanitized\n"
                   "  return rawInput;\n"
                   "}\n";
        }
        if (lang == "rust") {
            return "pub fn " + f + "(raw_input: &str) -> &str {\n"
                   "    raw_input\n"
                   "}\n";
        }
        return "STUB: treat input as raw until sanitized in " + f + "\n";
    }

    static std::string generateFileOperation(const std::string& lang, const std::string& op) {
        const std::string f = opName(op, "readFileSafe");
        if (lang == "python") {
            return "def " + f + "(base_dir, candidate):\n"
                   "    import os\n"
                   "    resolved = os.path.realpath(os.path.join(base_dir, candidate))\n"
                   "    if not resolved.startswith(os.path.realpath(base_dir)):\n"
                   "        raise ValueError(\"path traversal blocked\")\n"
                   "    with open(resolved, \"r\", encoding=\"utf-8\") as fh:\n"
                   "        return fh.read()\n";
        }
        if (lang == "typescript" || lang == "javascript") {
            return "export function " + f + "(baseDir, candidate) {\n"
                   "  const path = require(\"path\");\n"
                   "  const resolved = path.resolve(baseDir, candidate);\n"
                   "  if (!resolved.startsWith(path.resolve(baseDir))) throw new Error(\"path traversal blocked\");\n"
                   "  return require(\"fs\").readFileSync(resolved, \"utf8\");\n"
                   "}\n";
        }
        return "STUB: enforce canonical path prefix check in " + f + "\n";
    }

    static std::string generateNetworkCall(const std::string& lang, const std::string& op) {
        const std::string f = opName(op, "fetchSecure");
        if (lang == "python") {
            return "def " + f + "(url):\n"
                   "    import requests\n"
                   "    return requests.get(url, timeout=5, verify=True)\n";
        }
        if (lang == "typescript" || lang == "javascript") {
            return "export async function " + f + "(url) {\n"
                   "  const controller = new AbortController();\n"
                   "  setTimeout(() => controller.abort(), 5000);\n"
                   "  return fetch(url, { signal: controller.signal });\n"
                   "}\n";
        }
        if (lang == "rust") {
            return "pub async fn " + f + "(url: &str) -> Result<Response, reqwest::Error> {\n"
                   "    reqwest::Client::builder().use_rustls_tls().timeout(std::time::Duration::from_secs(5)).build()?.get(url).send().await\n"
                   "}\n";
        }
        return "STUB: include timeout and TLS verification in " + f + "\n";
    }

    static std::string generateCrypto(const std::string& lang, const std::string& op,
                                      bool& requiresHumanReview) {
        requiresHumanReview = true;
        const std::string f = opName(op, "encryptData");
        if (lang == "python") {
            return "def " + f + "(plaintext: bytes, key: bytes):\n"
                   "    from cryptography.hazmat.primitives.ciphers.aead import AESGCM\n"
                   "    nonce = b\"000000000000\"\n"
                   "    return AESGCM(key).encrypt(nonce, plaintext, None)\n";
        }
        if (lang == "typescript" || lang == "javascript") {
            return "export function " + f + "(plaintext, key, iv) {\n"
                   "  const crypto = require(\"crypto\");\n"
                   "  const cipher = crypto.createCipheriv(\"aes-256-gcm\", key, iv);\n"
                   "  return Buffer.concat([cipher.update(plaintext), cipher.final()]);\n"
                   "}\n";
        }
        if (lang == "rust") {
            return "pub fn " + f + "(plaintext: &[u8]) {\n"
                   "    // STUB: use AES-GCM via audited crate; keep human review in loop\n"
                   "}\n";
        }
        return "STUB: use modern AEAD algorithm (AES-GCM/ChaCha20-Poly1305) in " + f + "\n";
    }
};
