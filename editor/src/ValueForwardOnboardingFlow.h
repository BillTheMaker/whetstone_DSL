#pragma once
// Step 584: Value-Forward Onboarding Flow

#include <string>
#include <vector>

enum class OnboardingCardType {
    MCPDepth,
    ConstrainedExecution,
    VerificationLoop,
    WorkflowRouting
};

struct OnboardingCard {
    std::string cardId;
    OnboardingCardType type = OnboardingCardType::MCPDepth;
    std::string title;
    std::string message;
    bool completed = false;
};

struct OnboardingFlowState {
    std::string profileId;
    bool active = false;
    std::size_t currentIndex = 0;
    std::vector<OnboardingCard> cards;
};

class ValueForwardOnboardingFlow {
public:
    static bool start(const std::string& profileId,
                      OnboardingFlowState* state,
                      std::string* error) {
        if (!state || !error) return false;
        error->clear();
        if (profileId.empty()) {
            *error = "profile_id_missing";
            return false;
        }

        state->profileId = profileId;
        state->active = true;
        state->currentIndex = 0;
        state->cards = defaultCards();
        return true;
    }

    static bool completeCurrent(OnboardingFlowState* state, std::string* error) {
        if (!state || !error) return false;
        error->clear();
        if (!state->active) return fail(error, "flow_inactive");
        if (state->currentIndex >= state->cards.size()) return fail(error, "card_index_invalid");
        state->cards[state->currentIndex].completed = true;
        return true;
    }

    static bool next(OnboardingFlowState* state, std::string* error) {
        if (!state || !error) return false;
        error->clear();
        if (!state->active) return fail(error, "flow_inactive");
        if (state->currentIndex >= state->cards.size()) return fail(error, "card_index_invalid");
        if (!state->cards[state->currentIndex].completed) return fail(error, "current_card_not_completed");

        if (state->currentIndex + 1 >= state->cards.size()) {
            state->active = false;
            return true;
        }
        ++state->currentIndex;
        return true;
    }

    static float progress(const OnboardingFlowState& state) {
        if (state.cards.empty()) return 0.0f;
        int completed = 0;
        for (const auto& card : state.cards) if (card.completed) ++completed;
        return static_cast<float>(completed) / static_cast<float>(state.cards.size());
    }

    static std::vector<std::string> valueHighlights(const OnboardingFlowState& state) {
        std::vector<std::string> highlights;
        for (const auto& card : state.cards) {
            if (!card.completed) continue;
            if (card.type == OnboardingCardType::MCPDepth) highlights.push_back("mcp_tool_depth");
            if (card.type == OnboardingCardType::ConstrainedExecution) highlights.push_back("safe_constrained_ops");
            if (card.type == OnboardingCardType::VerificationLoop) highlights.push_back("verify_before_merge");
            if (card.type == OnboardingCardType::WorkflowRouting) highlights.push_back("transparent_routing_gates");
        }
        return highlights;
    }

private:
    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    static std::vector<OnboardingCard> defaultCards() {
        return {
            {"onboarding-1", OnboardingCardType::MCPDepth, "Deep Tool Reach", "Discover multi-tool MCP depth.", false},
            {"onboarding-2", OnboardingCardType::ConstrainedExecution, "Safe Execution", "Apply constrained operations with clear guardrails.", false},
            {"onboarding-3", OnboardingCardType::VerificationLoop, "Built-in Verification", "Run build/test loops before finalization.", false},
            {"onboarding-4", OnboardingCardType::WorkflowRouting, "Explainable Routing", "Understand review gates and decision routing.", false}
        };
    }
};
