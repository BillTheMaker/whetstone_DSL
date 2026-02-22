#pragma once
// Step 783: Actor-to-thread/async projection policy set.

#include <string>

#include <nlohmann/json.hpp>

#include "LogicActorPacketTypes.h"

struct ActorProjectionPolicyPacket {
    std::string policy;
    bool mailboxModeled = false;
    bool reviewRequired = false;
};

class ActorAsyncProjectionPolicy {
public:
    static ActorProjectionPolicyPacket choose(const LogicActorLoweringPacket& p, const std::string& targetLanguage) {
        ActorProjectionPolicyPacket out;
        out.policy = targetLanguage + "_actor_projection";
        out.mailboxModeled = (targetLanguage == "erlang" || targetLanguage == "elixir");
        out.reviewRequired = p.actorModel && !out.mailboxModeled;
        return out;
    }

    static nlohmann::json toJson(const ActorProjectionPolicyPacket& p) {
        return {{"policy", p.policy}, {"mailbox_modeled", p.mailboxModeled}, {"review_required", p.reviewRequired}};
    }
};
