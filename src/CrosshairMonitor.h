#pragma once

#include "RE/C/CrosshairPickData.h"
#include "RE/T/TESObjectREFR.h"
#include "RE/B/BGSKeywordForm.h"
#include "SKSE/Events.h"  
#include "SKSE/API.h"     
#include <string>

class CrosshairMonitor : public RE::BSTEventSink<SKSE::CrosshairRefEvent> {
    public:
        enum class InteractionType {
            kNone,
            kTalk,            // Talk to NPC
            kOpen,            // Open container/door
            kActivate,        // Generic activation
            kTake,            // Take item
            kHarvest,         // Harvest plants
            kSearch,          // Search container/corpse
            kSit,             // Sit in chair/bench
            kSleep,           // Sleep in bed
            kPickpocket,      // Pickpocket NPC
            kLockpick,        // Lockpick door/container
            kLockpickNone     // Lockpickable, but player has no lockpicks
        };

        static CrosshairMonitor* GetSingleton() {
            static CrosshairMonitor singleton;
            return &singleton;
        };

        void Init();

        RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* a_event, RE::BSTEventSource<SKSE::CrosshairRefEvent>*) override;
        static void ProcessReferenceChange(RE::TESObjectREFR* newRef);
        
        static bool IsLookingAtInteractable();
        static RE::TESObjectREFR* GetCrosshairReference();
        static uint32_t GetActivationFlags();
        static uint32_t GetActivationFlagsForRef(RE::TESObjectREFR* ref);
        static InteractionType GetInteractionType();
        static InteractionType GetInteractionTypeForRef(RE::TESObjectREFR* ref);
        static bool HasInteractionType(InteractionType type);
        static bool IsFormType(RE::FormType type);
        static bool PlayerHasLockPicks();
        
        using CrosshairChangeCallback = std::function<void(RE::TESObjectREFR* newRef)>;
        static void RegisterChangeCallback(CrosshairChangeCallback callback);

    private:
        CrosshairMonitor() = default;

        static void PrintInteractionToConsole(RE::TESObjectREFR* ref, InteractionType type);

        static inline RE::ObjectRefHandle lastCrosshairRef;
        static inline InteractionType lastInteractionType = InteractionType::kNone;
        static inline std::vector<CrosshairChangeCallback> callbacks;

        static inline RE::ObjectRefHandle lastTarget; // Last object looked at
        static inline RE::ObjectRefHandle lastTargetActor;
}