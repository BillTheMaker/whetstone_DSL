// Step 1884 TDD: LanguageIdiomProfile — static ideal feature vectors per language
//
//  t1: allProfiles() returns exactly 8 profiles
//  t2: all 8 profiles have non-empty language name and all 5 feature dimensions defined
//  t3: Haskell → TailRecursive, DependentTypes, mutation=0.0
//  t4: Elixir → Actors concurrency, EventDriven I/O, TailRecursive
//  t5: Go → Channels concurrency, FlatLoop; Rust → SharedMemory, TreeRecursive

#include "LanguageIdiomProfile.h"
#include <iostream>
#include <string>
#include <algorithm>

using AFF = whetstone::ASTFeatures;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(eight_profiles_returned);
    auto profiles = whetstone::LanguageIdiomProfile::allProfiles();
    C(profiles.size() == 8, "expected 8 profiles, got " + std::to_string(profiles.size()));
    P();
}

void t2(){
    T(all_profiles_have_five_dimensions);
    auto profiles = whetstone::LanguageIdiomProfile::allProfiles();
    for (const auto& p2 : profiles) {
        C(!p2.language.empty(), "profile has empty language name");
        // All enums are defined (just check they exist via assignment)
        (void)p2.idealMutationRatio;
        (void)p2.idealRecursion;
        (void)p2.idealConcurrency;
        (void)p2.idealIO;
        (void)p2.idealTypeComplexity;
    }
    P();
}

void t3(){
    T(haskell_profile_correct);
    auto profiles = whetstone::LanguageIdiomProfile::allProfiles();
    auto it = std::find_if(profiles.begin(), profiles.end(),
                           [](const auto& p2){ return p2.language == "Haskell"; });
    C(it != profiles.end(), "Haskell profile not found");
    C(it->idealMutationRatio == 0.0f, "Haskell mutationRatio should be 0.0");
    C(it->idealRecursion == AFF::RecursionShape::TailRecursive,
      "Haskell should be TailRecursive");
    C(it->idealTypeComplexity == AFF::TypeComplexity::DependentTypes,
      "Haskell should have DependentTypes");
    P();
}

void t4(){
    T(elixir_profile_actors_event_tail);
    auto profiles = whetstone::LanguageIdiomProfile::allProfiles();
    auto it = std::find_if(profiles.begin(), profiles.end(),
                           [](const auto& p2){ return p2.language == "Elixir"; });
    C(it != profiles.end(), "Elixir profile not found");
    C(it->idealConcurrency == AFF::ConcurrencyPrimitive::Actors, "Elixir should use Actors");
    C(it->idealIO == AFF::IOPattern::EventDriven, "Elixir should have EventDriven I/O");
    C(it->idealRecursion == AFF::RecursionShape::TailRecursive, "Elixir should be TailRecursive");
    P();
}

void t5(){
    T(go_channels_rust_shared_memory);
    auto profiles = whetstone::LanguageIdiomProfile::allProfiles();
    auto goIt = std::find_if(profiles.begin(), profiles.end(),
                             [](const auto& p2){ return p2.language == "Go"; });
    auto rustIt = std::find_if(profiles.begin(), profiles.end(),
                               [](const auto& p2){ return p2.language == "Rust"; });
    C(goIt != profiles.end(), "Go profile not found");
    C(rustIt != profiles.end(), "Rust profile not found");
    C(goIt->idealConcurrency == AFF::ConcurrencyPrimitive::Channels, "Go should use Channels");
    C(goIt->idealRecursion == AFF::RecursionShape::FlatLoop, "Go should be FlatLoop");
    C(rustIt->idealConcurrency == AFF::ConcurrencyPrimitive::SharedMemory, "Rust should use SharedMemory");
    C(rustIt->idealRecursion == AFF::RecursionShape::TreeRecursive, "Rust should be TreeRecursive");
    P();
}

int main(){
    std::cout << "Step 1884: LanguageIdiomProfile\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
