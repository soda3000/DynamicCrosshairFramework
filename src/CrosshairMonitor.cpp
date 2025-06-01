#include "CrosshairMonitor.h"
#include "RE/C/CrosshairPickData.h"
#include "RE/C/ConsoleLog.h"
#include "RE/A/Actor.h"
#include "RE/Skyrim.h"

// Add logger namespace
namespace logger = SKSE::log;

void CrosshairMonitor::Init() {
    auto crosshairEventSource = SKSE::GetCrosshairRefEventSource();
    if (crosshairEventSource) {
        // Use the singleton instance to register
        crosshairEventSource->AddEventSink(GetSingleton());
        logger::info("Successfully registered for crosshair events");
    } else {
        logger::error("Could not get crosshair event source");
    }
}

RE::BSEventNotifyControl CrosshairMonitor::ProcessEvent(const SKSE::CrosshairRefEvent* a_event, RE::BSTEventSource<SKSE::CrosshairRefEvent>*) {
    if (!a_event) { return RE::BSEventNotifyControl::kContinue; }

    RE::TESObjectREFR* crosshairTarget = a_event->crosshairRef.get();
    if (!crosshairTarget) { return RE::BSEventNotifyControl::kContinue; }

    CrosshairMonitor::InteractionType iType = GetInteractionTypeForRef(crosshairTarget);

    switch (iType) {
        case CrosshairMonitor::InteractionType::kTalk:
            // Display talk crosshair TODO
        case CrosshairMonitor::InteractionType::kOpen:
            // Display open crosshair TODO
        case CrosshairMonitor::InteractionType::kActivate:
            // Display activate crosshair TODO
        case CrosshairMonitor::InteractionType::kTake:
            // Display take crosshair TODO
        case CrosshairMonitor::InteractionType::kHarvest:
            // Display harvest crosshair TODO
        case CrosshairMonitor::InteractionType::kSearch:
            // Display search crosshair TODO
        case CrosshairMonitor::InteractionType::kSit:
            // Display sit crosshair TODO
        case CrosshairMonitor::InteractionType::kSleep:
            // Display sleep crosshair TODO
        case CrosshairMonitor::InteractionType::kPickpocket:
            // Display pickpocket crosshair TODO
        case CrosshairMonitor::InteractionType::kLockpick:
            // Display lockpick crosshair TODO
        case CrosshairMonitor::InteractionType::kLockpickNone:
            // Display lockpick none crosshair TODO
        case CrosshairMonitor::InteractionType::kRead:
            // Display read crosshair TODO
        case CrosshairMonitor::InteractionType::kDoor:
            // Display door crosshair TODO
        case CrosshairMonitor::InteractionType::kSteal:
            // Display steal crosshair TODO
        default: // and also kNone
            // Display default crosshair TODO
            break;
    };
    
    return RE::BSEventNotifyControl::kContinue;
}

uint32_t CrosshairMonitor::GetActivationFlags() {
    auto* ref = GetCrosshairReference();
    return GetActivationFlagsForRef(ref);
}

uint32_t CrosshairMonitor::GetActivationFlagsForRef(RE::TESObjectREFR* ref) {
    if (!ref) return 0;
    
    // Since GetActivateFlags doesn't exist, we'll return a synthetic value
    // based on the form type to simulate activation flags
    auto* baseObj = ref->GetBaseObject();
    if (!baseObj) return 0;
    
    uint32_t flags = 0;
    
    // Set synthetic flags based on form type
    auto formType = baseObj->GetFormType();
    
    if (formType == RE::FormType::NPC) {
        // Set talk flag for NPCs
        flags |= 0x01;  // Talk flag
        
        // For hostility checking, we'd need to cast to Actor and use appropriate methods
        // But for now, just use the NPC type
    } 
    if (formType == RE::FormType::Container) {
        flags |= 0x02;  // Container flag
    }
    if (formType == RE::FormType::Door) {
        flags |= 0x02;  // Use door flag (same as container)
        if (ref->IsLocked()) {
            flags |= 0x08;  // Unlock flag
        }
    }
    if (formType == RE::FormType::Book) {
        flags |= 0x04;  // Read flag
    }
    if (formType == RE::FormType::Furniture) {
        flags |= 0x10;  // Use object (furniture) flag
    }
    if (formType == RE::FormType::Flora || 
        formType == RE::FormType::Tree ||
        formType == RE::FormType::Activator) {
        flags |= 0x20;  // Harvest/Activate flag
    }
    
    return flags;
}

CrosshairMonitor::InteractionType CrosshairMonitor::GetInteractionType() {
    auto* ref = GetCrosshairReference();
    return GetInteractionTypeForRef(ref);
}

RE::TESObjectREFR* CrosshairMonitor::GetCrosshairReference() {
    auto* crosshairData = RE::CrosshairPickData::GetSingleton();
    if (crosshairData) {
        // Handle both VR and non-VR versions
        #if defined(EXCLUSIVE_SKYRIM_FLAT)
        // Non-VR version
            if (crosshairData->target) {
                RE::TESObjectREFR* ref = nullptr;
                if (crosshairData->target.get()) {
                    return ref;
                }
            }
        #else
            // VR version - use the headset/gamepad target (index 2)
            if (crosshairData->target[RE::VR_DEVICE::kHeadset]) {
                RE::TESObjectREFR* ref = nullptr;
                if (crosshairData->target[RE::VR_DEVICE::kHeadset].get()) {
                    return ref;
                }
            }
        #endif
    }
    return nullptr;
}

void CrosshairMonitor::RegisterChangeCallback(CrosshairChangeCallback callback) {
    callbacks.push_back(callback);
}

void CrosshairMonitor::PrintInteractionToConsole(RE::TESObjectREFR* ref, InteractionType type) {
    if (!ref) return;
    
    // Get the object name
    std::string objName = ref->GetName();
    if (objName.empty()) {
        objName = "Unknown Object";
    }
    
    // Construct message based on interaction type
    std::string message;
    
    switch (type) {
        case InteractionType::kTalk:
            message = "You can talk to " + objName;
            break;
            
        case InteractionType::kOpen:
            message = "You can open " + objName;
            break;
            
        case InteractionType::kTake:
            message = "You can take " + objName;
            break;
            
        case InteractionType::kActivate:
            message = "You can activate " + objName;
            break;
            
        case InteractionType::kHarvest:
            message = "You can harvest " + objName;
            break;
            
        case InteractionType::kSearch:
            message = "You can search " + objName;
            break;
            
        case InteractionType::kSit:
            message = "You can sit on " + objName;
            break;
            
        case InteractionType::kSleep:
            message = "You can sleep in " + objName;
            break;
            
        case InteractionType::kPickpocket:
            message = "You can pickpocket " + objName;
            break;
            
        case InteractionType::kLockpick:
            message = "You can lockpick " + objName;
            break;
        
        case InteractionType::kLockpickNone:
            message = "You are out of lockpicks";
            break;
            
        case InteractionType::kNone:
            // No interaction possible
            return;
            
        default:
            message = "Unknown interaction with " + objName;
            break;
    }
    
    // Print to console log instead of using Console::GetSingleton
    RE::ConsoleLog::GetSingleton()->Print(message.c_str());
}

// Get interaction type for a specific reference
CrosshairMonitor::InteractionType CrosshairMonitor::GetInteractionTypeForRef(RE::TESObjectREFR* ref) {
    if (!ref) { return InteractionType::kNone; }
    
    auto* baseObj = ref->GetBaseObject();
    if (!baseObj) { return InteractionType::kNone; }
    
    auto formType = baseObj->GetFormType();
    // Form types:
    // Scroll - 17 SCRL
    // Activator - 18 ACTI
    // TalkingActivator - 19 TACT
    // Armor - 1A ARMO
    // Book - 1B BOOK
    // Container - 1C CONT
    // Door - 1D DOOR
    // Ingredient - 1E INGR
    // Light - 1F LIGH
    // Misc - 20 MISC
    // Flora - 27 FLOR
    // Furniture - 28 FURN
    // Weapon - 29 WEAP
    // Ammo - 2A AMMO
    // NPC - 2B NPC_
    // Leveled NPC - 2C LVLN
    // Key Master - 2D KEYM
    // Alchemy Item - 2E ALCH
    // Note - 30 NOTE
    // SoulGem - 34 SLGM
    // Leveled Item - 35 LVLI
    // ActorCharacter - 3E ACHR
    switch (formType) {
        case RE::FormType::Scroll:
        case RE::FormType::Armor:
        case RE::FormType::Ingredient:
        case RE::FormType::Misc:
        case RE::FormType::Weapon:
        case RE::FormType::Ammo:
        case RE::FormType::KeyMaster:
        case RE::FormType::AlchemyItem:
        case RE::FormType::SoulGem:
        case RE::FormType::LeveledItem:
            if (RE::PlayerCharacter::GetSingleton()->WouldBeStealing(ref)) {
                return InteractionType::kSteal;
               }
            else {
                return InteractionType::kTake;
            }
        case RE::FormType::Activator:
        case RE::FormType::TalkingActivator:
        case RE::FormType::Light:
            return InteractionType::kActivate;
        case RE::FormType::Book:
        case RE::FormType::Note:
            return InteractionType::kRead;
        case RE::FormType::Container:
            return InteractionType::kSearch;
        case RE::FormType::Door:
        {
            if (ref->IsLocked()) {
                if (ref->GetLockLevel() == RE::LOCK_LEVEL::kRequiresKey) {
                    if (ref->GetLock()->key && RE::PlayerCharacter::GetSingleton()->GetItemCount(ref->GetLock()->key) > 0) {
                        return InteractionType::kUseKey;
                    }
                    else {
                        return InteractionType::kRequiresKey;
                    }
                }
                if (PlayerHasLockPicks()) {
                    return InteractionType::kLockpick;
                }
                else {
                    return InteractionType::kLockpickNone;
                }
            }
            return InteractionType::kDoor;
        }
        case RE::FormType::Flora:
            return InteractionType::kHarvest;
        case RE::FormType::NPC: // check if sneaking, if hostile, if companion.
        case RE::FormType::LeveledNPC:
        case RE::FormType::ActorCharacter:
        {
            auto* actor = ref->As<RE::Actor>();
            if (RE::PlayerCharacter::GetSingleton()->IsSneaking()) {
                if (actor->IsPlayerTeammate()) {
                    return InteractionType::kTalk;
                }
                else {
                    return InteractionType::kPickpocket;
                }
            }
            else {
                if (actor->IsHostileToActor(RE::PlayerCharacter::GetSingleton())) {
                    return InteractionType::kNone;
                }
                else {
                    return InteractionType::kTalk;
                }
            }
        }
        default:
            return InteractionType::kNone;
    }
}

bool CrosshairMonitor::IsFormType(RE::FormType type) {
    auto* ref = GetCrosshairReference();
    if (!ref) return false;
    
    auto* baseObj = ref->GetBaseObject();
    return baseObj && baseObj->Is(type);
}

bool CrosshairMonitor::PlayerHasLockPicks() {
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return false;
    } 

    // Get lockpick form (Form ID 0x0000000A)
    auto lpForm = RE::TESForm::LookupByID(0x0000000A);
    if (!lpForm) {
        return false;
    }

    // Get the inv object for lockpick form type
    auto boundObject = lpForm->As<RE::TESBoundObject>();
    if (!boundObject) {
        return false;
    }

    // Check if player has any lockpicks
    std::int32_t count = player->GetItemCount(boundObject);
    return count > 0;
}