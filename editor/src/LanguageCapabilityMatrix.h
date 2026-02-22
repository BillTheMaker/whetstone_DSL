#pragma once
// Step 691: Language capability matrix model.

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct LanguageCapabilityRow {
    std::string language;
    std::string paradigm; // systems, dynamic, ast-native, logic, actor, etc.
    std::string typeModel;
    std::string memoryModel;
    std::string errorModel;
    std::string concurrencyModel;
    std::string macroModel;
    std::string ffiProfile;
    std::string buildEcosystem;
    std::string runtimeAssumptions;
    bool experimental = true;
};

struct CapabilityDiff {
    std::string language;
    std::vector<std::string> changedFields;
};

class LanguageCapabilityMatrix {
public:
    static bool validateRow(const LanguageCapabilityRow& row,
                            bool strict,
                            std::string* error) {
        if (error) *error = "";
        if (row.language.empty()) {
            if (error) *error = "language_missing";
            return false;
        }
        if (!strict) return true;
        if (row.paradigm.empty()) { if (error) *error = "paradigm_missing"; return false; }
        if (row.typeModel.empty()) { if (error) *error = "type_model_missing"; return false; }
        if (row.memoryModel.empty()) { if (error) *error = "memory_model_missing"; return false; }
        if (row.errorModel.empty()) { if (error) *error = "error_model_missing"; return false; }
        if (row.concurrencyModel.empty()) { if (error) *error = "concurrency_model_missing"; return false; }
        return true;
    }

    static void addOrUpdate(std::vector<LanguageCapabilityRow>* rows,
                            const LanguageCapabilityRow& row) {
        if (!rows) return;
        for (auto& r : *rows) {
            if (r.language == row.language) { r = row; return; }
        }
        rows->push_back(row);
    }

    static const LanguageCapabilityRow* get(const std::vector<LanguageCapabilityRow>& rows,
                                            const std::string& language) {
        for (const auto& r : rows) if (r.language == language) return &r;
        return nullptr;
    }

    static std::map<std::string, std::vector<LanguageCapabilityRow>>
    groupByParadigm(const std::vector<LanguageCapabilityRow>& rows) {
        std::map<std::string, std::vector<LanguageCapabilityRow>> out;
        for (const auto& r : rows) out[r.paradigm].push_back(r);
        return out;
    }

    static std::vector<LanguageCapabilityRow> sorted(const std::vector<LanguageCapabilityRow>& rows) {
        std::vector<LanguageCapabilityRow> out = rows;
        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
            return a.language < b.language;
        });
        return out;
    }

    static nlohmann::json rowToJson(const LanguageCapabilityRow& r) {
        return {
            {"language", r.language},
            {"paradigm", r.paradigm},
            {"typeModel", r.typeModel},
            {"memoryModel", r.memoryModel},
            {"errorModel", r.errorModel},
            {"concurrencyModel", r.concurrencyModel},
            {"macroModel", r.macroModel},
            {"ffiProfile", r.ffiProfile},
            {"buildEcosystem", r.buildEcosystem},
            {"runtimeAssumptions", r.runtimeAssumptions},
            {"experimental", r.experimental}
        };
    }

    static LanguageCapabilityRow rowFromJson(const nlohmann::json& j) {
        LanguageCapabilityRow r;
        r.language = j.value("language", "");
        r.paradigm = j.value("paradigm", "");
        r.typeModel = j.value("typeModel", "");
        r.memoryModel = j.value("memoryModel", "");
        r.errorModel = j.value("errorModel", "");
        r.concurrencyModel = j.value("concurrencyModel", "");
        r.macroModel = j.value("macroModel", "");
        r.ffiProfile = j.value("ffiProfile", "");
        r.buildEcosystem = j.value("buildEcosystem", "");
        r.runtimeAssumptions = j.value("runtimeAssumptions", "");
        r.experimental = j.value("experimental", true);
        return r;
    }

    static nlohmann::json toJson(const std::vector<LanguageCapabilityRow>& rows) {
        nlohmann::json out = nlohmann::json::array();
        for (const auto& r : sorted(rows)) out.push_back(rowToJson(r));
        return out;
    }

    static std::vector<LanguageCapabilityRow> fromJson(const nlohmann::json& j) {
        std::vector<LanguageCapabilityRow> out;
        if (!j.is_array()) return out;
        for (const auto& it : j) out.push_back(rowFromJson(it));
        return out;
    }

    static std::vector<CapabilityDiff> diff(const std::vector<LanguageCapabilityRow>& oldRows,
                                            const std::vector<LanguageCapabilityRow>& newRows) {
        std::vector<CapabilityDiff> out;
        for (const auto& n : newRows) {
            const auto* o = get(oldRows, n.language);
            if (!o) {
                out.push_back({n.language, {"row_added"}});
                continue;
            }
            CapabilityDiff d;
            d.language = n.language;
            if (o->paradigm != n.paradigm) d.changedFields.push_back("paradigm");
            if (o->typeModel != n.typeModel) d.changedFields.push_back("typeModel");
            if (o->memoryModel != n.memoryModel) d.changedFields.push_back("memoryModel");
            if (o->errorModel != n.errorModel) d.changedFields.push_back("errorModel");
            if (o->concurrencyModel != n.concurrencyModel) d.changedFields.push_back("concurrencyModel");
            if (o->macroModel != n.macroModel) d.changedFields.push_back("macroModel");
            if (o->ffiProfile != n.ffiProfile) d.changedFields.push_back("ffiProfile");
            if (o->buildEcosystem != n.buildEcosystem) d.changedFields.push_back("buildEcosystem");
            if (o->runtimeAssumptions != n.runtimeAssumptions) d.changedFields.push_back("runtimeAssumptions");
            if (o->experimental != n.experimental) d.changedFields.push_back("experimental");
            if (!d.changedFields.empty()) out.push_back(std::move(d));
        }
        return out;
    }

    static std::vector<LanguageCapabilityRow> defaultRows() {
        return {
            {"cpp", "systems", "static", "raii", "exceptions", "threads", "templates", "abi-stable", "cmake", "native runtime", false},
            {"rust", "systems", "static", "borrow-checked", "result+panic", "async+threads", "macros", "ffi-unsafe-boundary", "cargo", "native runtime", false},
            {"python", "dynamic", "dynamic", "gc", "exceptions", "async+threads", "metaprogramming", "ctypes/cffi", "pip", "interpreter runtime", true},
            {"go", "systems", "static", "gc", "error values", "goroutines", "limited", "cgo", "go mod", "native runtime", false},
            {"java", "managed", "static", "gc", "exceptions", "threads", "annotations", "jni", "maven/gradle", "jvm runtime", false}
        };
    }
};
