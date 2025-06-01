#pragma once

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"
#include <d3d11.h>
#ifndef DIRECTINPUT_VERSION
#	define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

class Menu {
    public:
        ~Menu();

        static Menu* GetSingleton() {
            static Menu singleton;
            return &singleton;
        }

        bool initialized = false;

        bool Init();
        void DrawMenu();
        
        // Crosshair source options
        enum class CrosshairSource {
            Images,
            IconFont,
            WebIconPack
        };
        CrosshairSource currentSource = CrosshairSource::Images;

        void ProcessInputEvents(RE::InputEvent* const* a_event);
        bool ShouldSwallowInput();
        void ProcessInputEventQueue(); // Moved to public

        uint32_t toggleKey = VK_F10;
        bool menuToggle = false; // Added to track menu visibility

    private:
    
        Menu() = default;

        ID3D11Device* d3d_device = nullptr;
        ID3D11DeviceContext* d3d_context = nullptr;

        const char* KeyIdToString(uint32_t a_keyId);
        const ImGuiKey VirtualKeyToImGuiKey(WPARAM vkKey);

        class CharEvent : public RE::InputEvent {
            public:
                uint32_t keyCode;
        };

        struct KeyEvent {
            explicit KeyEvent(const RE::ButtonEvent* a_event) :
                keyCode(a_event->GetIDCode()),
                device(a_event->GetDevice()),
                eventType(a_event->GetEventType()),
                value(a_event->Value()),
                heldDownSecs(a_event->HeldDuration()) {}

            explicit KeyEvent(const CharEvent* a_event) :
                keyCode(a_event->keyCode),
                device(a_event->GetDevice()),
                eventType(a_event->GetEventType()) {}

            [[nodiscard]] constexpr bool IsPressed() const noexcept { return value > 0.0F; }
            [[nodiscard]] constexpr bool IsRepeating() const noexcept { return heldDownSecs > 0.0F; }
            [[nodiscard]] constexpr bool IsDown() const noexcept { return IsPressed() && (heldDownSecs == 0.0F); }
            [[nodiscard]] constexpr bool IsHeld() const noexcept { return IsPressed() && IsRepeating(); }
            [[nodiscard]] constexpr bool IsUp() const noexcept { return (value == 0.0F) && IsRepeating(); }

            uint32_t keyCode;
            RE::INPUT_DEVICE device;
            RE::INPUT_EVENT_TYPE eventType;
            float value = 0;
            float heldDownSecs = 0;
        };
        const uint32_t DIKToVK(uint32_t DIK);
        std::vector<KeyEvent> _keyEventQueue{};
        void addToEventQueue(KeyEvent e);
};