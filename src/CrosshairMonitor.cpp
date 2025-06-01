#include "CrosshairMonitor.h"
#include "RE/C/CrosshairPickData.h"
#include "RE/C/ConsoleLog.h"
#include "RE/A/Actor.h"
#include "RE/Skyrim.h"

// Add logger namespace
namespace logger = SKSE::log;

void CrosshairMonitor::Init() {
    lastCrosshairRef.reset();
    lastTarget.reset();
    lastTargetActor.reset();

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
    
    // Process the reference change
    ProcessReferenceChange(crosshairTarget);
    
    // Get interaction type for more detailed processing
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

void CrosshairMonitor::ProcessReferenceChange(RE::TESObjectREFR* newRef) {
    // Check if it's different from last reference
    RE::ObjectRefHandle currentHandle;
    if (newRef) {
        currentHandle = newRef->GetHandle();
    }
    
    // Detect changes in interaction type
    auto currentInteractionType = GetInteractionTypeForRef(newRef);
    
    // If either the reference or the interaction type has changed
    if (currentHandle != lastCrosshairRef || currentInteractionType != lastInteractionType) {
        // Update tracking variables
        lastCrosshairRef = currentHandle;
        lastInteractionType = currentInteractionType;
        
        // Update the UI with the new crosshair information
        /*auto* crosshairUI = CrosshairUI::GetSingleton();
        if (crosshairUI) {
            crosshairUI->UpdateCrosshairInfo(newRef);
        }*/
        
        // Only proceed if we have a valid reference
        if (newRef) {
            // Write to console based on interaction type (keep for debugging)
            PrintInteractionToConsole(newRef, currentInteractionType);
        }
        
        // Notify all registered callbacks
        for (const auto& callback : callbacks) {
            callback(newRef);
        }
    }
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
                if (crosshairData->target.get()) {
                    return crosshairData->target.get().get();
                }
            }
        #else
            // VR version - use the headset/gamepad target (index 2)
            if (crosshairData->target[RE::VR_DEVICE::kHeadset]) {
                if (crosshairData->target[RE::VR_DEVICE::kHeadset].get()) {
                    return crosshairData->target[RE::VR_DEVICE::kHeadset].get().get();
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
    
    auto flags = GetActivationFlagsForRef(ref);
    if (flags == 0) return InteractionType::kNone;
    
    // Check our synthetic activation flags
    
    // Talk flag (0x01)
    if (flags & 0x01) {
        if (ref && ref->GetBaseObject()->Is(RE::FormType::NPC)) {
            // Check if sneaking for pickpocket
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (player && player->IsSneaking()) {
                return InteractionType::kPickpocket;
            }
            return InteractionType::kTalk;
        }
    }
    
    // Container/Door flag (0x02)
    if (flags & 0x02) {
        if (ref) {
            auto* baseObj = ref->GetBaseObject();
            if (baseObj) {
                // Check if it's a dead body
                if (baseObj->Is(RE::FormType::ActorCharacter) && ref->IsDead()) {
                    return InteractionType::kSearch;
                }
                else if (baseObj->Is(RE::FormType::Door)) {
                    // Check if door is locked.
                    if (ref->IsLocked()) {
                        // Get Lock Level
                        auto lockLevel = ref->GetLockLevel();
                        if (lockLevel == RE::LOCK_LEVEL::kRequiresKey) {
                            // Check if player has key - for now just return open
                            return InteractionType::kOpen;
                        }
                        // Check if between 0 and 4 (very easy to very hard)
                        else if (lockLevel >= RE::LOCK_LEVEL::kVeryEasy && lockLevel <= RE::LOCK_LEVEL::kVeryHard) {
                            // Check if player has lockpicks
                            if (PlayerHasLockPicks()) {
                                return InteractionType::kLockpick;
                            }
                            else {
                                return InteractionType::kLockpickNone;
                            }
                        }
                    }
                    else {
                        // Door is not locked, just open it
                        return InteractionType::kOpen;
                    }
                }
                // Otherwise normal open
                return InteractionType::kOpen;
            }
        }
    }
    
    // Take/Read flag (0x04)
    if (flags & 0x04) {
        return InteractionType::kTake;
    }
    
    // Unlock flag (0x08)
    if (flags & 0x08) {
        return InteractionType::kLockpick;
    }
    
    // Furniture flag (0x10)
    if (flags & 0x10) {
        if (ref) {
            return InteractionType::kSit;
        }
    }
    
    // Harvest/Activate flag (0x20)
    if (flags & 0x20) {
        if (ref && ref->GetBaseObject()->Is(RE::FormType::Flora)) {
            return InteractionType::kHarvest;
        }
    }
    
    // Default generic activation
    return InteractionType::kActivate;
}

bool CrosshairMonitor::HasInteractionType(InteractionType type) {
    return GetInteractionType() == type;
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