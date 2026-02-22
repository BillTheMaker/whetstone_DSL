#pragma once
// Step 1454: repro packet archive + replay.

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "FailurePacket.h"

struct ReproPacket {
    FailurePacket failure;
    std::string envHash;
    nlohmann::json fileHashes = nlohmann::json::object();
    nlohmann::json patchHistory = nlohmann::json::array();
    std::string reproCommand;
};

class ReproPacketStore {
public:
    static bool save(const std::string& path, const ReproPacket& p) {
        std::ofstream out(path);
        if (!out.good()) return false;
        out << toJson(p).dump(2);
        return true;
    }

    static bool load(const std::string& path, ReproPacket* out) {
        if (!out) return false;
        std::ifstream in(path);
        if (!in.good()) return false;
        nlohmann::json j;
        in >> j;
        *out = fromJson(j);
        return true;
    }

    static bool appendHistory(const std::string& path, const nlohmann::json& historyItem) {
        ReproPacket p;
        if (!load(path, &p)) return false;
        p.patchHistory.push_back(historyItem);
        return save(path, p);
    }

    static bool replayClassMatches(const ReproPacket& archived,
                                   const FailurePacket& current) {
        return archived.failure.failureClass == current.failureClass &&
               archived.failure.primaryFile == current.primaryFile;
    }

    static bool exportJsonl(const std::string& path,
                            const std::vector<ReproPacket>& packets) {
        std::ofstream out(path);
        if (!out.good()) return false;
        for (const auto& p : packets) out << toJson(p).dump() << "\n";
        return true;
    }

    static nlohmann::json toJson(const ReproPacket& p) {
        return {
            {"failure", FailurePacketModel::toJson(p.failure)},
            {"env_hash", p.envHash},
            {"file_hashes", p.fileHashes},
            {"patch_history", p.patchHistory},
            {"repro_command", p.reproCommand}
        };
    }

    static ReproPacket fromJson(const nlohmann::json& j) {
        ReproPacket p;
        p.failure = FailurePacketModel::fromJson(j.value("failure", nlohmann::json::object()));
        p.envHash = j.value("env_hash", "");
        p.fileHashes = j.value("file_hashes", nlohmann::json::object());
        p.patchHistory = j.value("patch_history", nlohmann::json::array());
        p.reproCommand = j.value("repro_command", "");
        return p;
    }
};
