#pragma once
// Step 569: Disassembly + Register View Baseline

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct DisassemblyInstruction {
    std::uint64_t address = 0;
    std::string opcode;
    std::string operands;
    bool isCurrentInstruction = false;
};

struct RegisterValue {
    std::string name;
    std::uint64_t value = 0;
};

class DisassemblyRegisterViewModel {
public:
    bool setInstructions(const std::vector<DisassemblyInstruction>& instructions, std::string* error) {
        if (!error) return false;
        error->clear();
        if (instructions.empty()) return fail(error, "instruction_list_empty");

        for (const auto& instruction : instructions) {
            if (instruction.address == 0) return fail(error, "instruction_address_invalid");
            if (instruction.opcode.empty()) return fail(error, "instruction_opcode_missing");
        }

        instructions_ = instructions;
        std::stable_sort(instructions_.begin(),
                         instructions_.end(),
                         [](const DisassemblyInstruction& a, const DisassemblyInstruction& b) {
                             return a.address < b.address;
                         });
        clearCurrentIp();
        return true;
    }

    bool setCurrentInstructionPointer(std::uint64_t address, std::string* error) {
        if (!error) return false;
        error->clear();
        if (address == 0) return fail(error, "instruction_pointer_invalid");

        bool found = false;
        clearCurrentIp();
        for (auto& instruction : instructions_) {
            if (instruction.address == address) {
                instruction.isCurrentInstruction = true;
                found = true;
                break;
            }
        }
        if (!found) return fail(error, "instruction_pointer_not_found");
        return true;
    }

    std::vector<DisassemblyInstruction> windowAroundAddress(std::uint64_t centerAddress,
                                                            std::size_t radius) const {
        std::vector<DisassemblyInstruction> window;
        if (instructions_.empty()) return window;

        std::size_t centerIndex = instructions_.size();
        for (std::size_t i = 0; i < instructions_.size(); ++i) {
            if (instructions_[i].address == centerAddress) {
                centerIndex = i;
                break;
            }
        }
        if (centerIndex == instructions_.size()) return window;

        const std::size_t begin = centerIndex > radius ? centerIndex - radius : 0;
        const std::size_t end = std::min(instructions_.size(), centerIndex + radius + 1);
        for (std::size_t i = begin; i < end; ++i) window.push_back(instructions_[i]);
        return window;
    }

    bool setRegisterValue(const std::string& registerName,
                          std::uint64_t value,
                          std::string* error) {
        if (!error) return false;
        error->clear();
        if (registerName.empty()) return fail(error, "register_name_missing");
        registers_[registerName] = value;
        return true;
    }

    bool hasRegister(const std::string& registerName) const {
        return registers_.count(registerName) != 0;
    }

    std::uint64_t registerValueOrZero(const std::string& registerName) const {
        auto it = registers_.find(registerName);
        if (it == registers_.end()) return 0;
        return it->second;
    }

    std::vector<RegisterValue> sortedRegisters() const {
        std::vector<RegisterValue> out;
        for (const auto& kv : registers_) out.push_back({kv.first, kv.second});
        std::stable_sort(out.begin(), out.end(), [](const RegisterValue& a, const RegisterValue& b) {
            return a.name < b.name;
        });
        return out;
    }

    const std::vector<DisassemblyInstruction>& instructions() const { return instructions_; }

private:
    std::vector<DisassemblyInstruction> instructions_;
    std::map<std::string, std::uint64_t> registers_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    void clearCurrentIp() {
        for (auto& instruction : instructions_) instruction.isCurrentInstruction = false;
    }
};
