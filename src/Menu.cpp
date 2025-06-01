#include "Menu.h"
#include <Windows.h>

namespace logger = SKSE::log;

Menu::~Menu() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

bool Menu::Init() {
    if (initialized) return true;

    auto* renderer = RE::BSGraphics::Renderer::GetSingleton();
    if (!renderer) {
        logger::error("Failed to get BSGraphics::Renderer singleton for menu.");
        return false;
    }
    
    REX::W32::ID3D11Device* device = RE::BSGraphics::Renderer::GetDevice();
    auto* rendererData = RE::BSGraphics::Renderer::GetRendererDataSingleton();
    if (!device || !rendererData || !rendererData->context) {
        logger::error("Failed to get device, renderer data, or context for menu.");
        return false;
    }

    device->AddRef();
    rendererData->context->AddRef();
    d3d_device = reinterpret_cast<ID3D11Device*>(device);
    d3d_context = reinterpret_cast<ID3D11DeviceContext*>(rendererData->context);

    auto* currentWindow = RE::BSGraphics::Renderer::GetCurrentRenderWindow();
    if (!currentWindow) {
        logger::error("Failed to get current render window for menu.");
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    
    io.MouseDrawCursor = true;
    io.WantCaptureMouse = true;
    io.WantCaptureKeyboard = true;
    
    ImGui::StyleColorsDark();
    
    if (!ImGui_ImplWin32_Init(currentWindow->hWnd)) {
        logger::error("Failed to initialize ImGui_ImplWin32 for menu.");
        return false;
    }

    if (!ImGui_ImplDX11_Init(d3d_device, d3d_context)) {
        logger::error("Failed to initialize ImGui_ImplDX11 for menu.");
        return false;
    }

    initialized = true;
    logger::info("Successfully initialized ImGui for menu.");
    return true;
}

void Menu::DrawMenu() {
    if (!this->menuToggle) {
        ImGui::GetIO().MouseDrawCursor = false;
        return;
    }
    
    ImGui::GetIO().MouseDrawCursor = true;
    
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Appearing);
    ImGui::Begin("Crosshair Configuration Menu", &this->menuToggle);
    
    // Dropdown for crosshair source
    const char* sourceItems[] = { "Images", "Icon Font", "Web Icon Pack" };
    int currentSourceIndex = static_cast<int>(currentSource);
    
    if (ImGui::Combo("Crosshair Source", &currentSourceIndex, sourceItems, IM_ARRAYSIZE(sourceItems))) {
        // Update the current source when selection changes
        currentSource = static_cast<CrosshairSource>(currentSourceIndex);
        
        // Log the change
        const char* sourceNames[] = { "Images", "Icon Font", "Web Icon Pack" };
        logger::info("Crosshair source changed to: {}", sourceNames[currentSourceIndex]);
    }
    
    // Additional UI based on selected source
    ImGui::Separator();
    
    switch (currentSource) {
        case CrosshairSource::Images:
            ImGui::Text("Image Settings");
            // Add image-specific settings here
            break;
            
        case CrosshairSource::IconFont:
            ImGui::Text("Icon Font Settings");
            // Add icon font-specific settings here
            break;
            
        case CrosshairSource::WebIconPack:
            ImGui::Text("Web Icon Pack Settings");
            // Add web icon pack-specific settings here
            break;
    }
    
    ImGui::End();
}

const ImGuiKey Menu::VirtualKeyToImGuiKey(WPARAM vkKey) {
    switch (vkKey) {
        case VK_F10:
            return ImGuiKey_F10;
        case VK_ESCAPE:
            return ImGuiKey_Escape;
        default:
            return ImGuiKey_Enter;
    }
}

inline const uint32_t Menu::DIKToVK(uint32_t DIK) {
    switch (DIK) {
        case DIK_DELETE:
            return VK_DELETE;
        default:
            return DIK;
    }
}
    
void Menu::ProcessInputEventQueue() {
    ImGuiIO& io = ImGui::GetIO();
    for (auto& event : _keyEventQueue) {
        if (event.eventType == RE::INPUT_EVENT_TYPE::kChar) {
            io.AddInputCharacter(event.keyCode);
            continue;
        }

        if (event.device == RE::INPUT_DEVICE::kMouse) {
            if (event.keyCode > 5) {
                event.keyCode = 5;
            }
            io.AddMouseButtonEvent(event.keyCode, event.IsPressed());
        }

        if (event.device == RE::INPUT_DEVICE::kKeyboard) {
            uint32_t key = DIKToVK(event.keyCode);
            if (key == event.keyCode) {
                key = MapVirtualKeyEx(event.keyCode, MAPVK_VSC_TO_VK_EX, GetKeyboardLayout(0));
            }
            if (!event.IsPressed()) {
                if (key == toggleKey) {
                    this->menuToggle = !this->menuToggle;
                }
                if (key == VK_ESCAPE && this->menuToggle) {
                    this->menuToggle = false;
                }
            }
            io.AddKeyEvent(VirtualKeyToImGuiKey(key), event.IsPressed());
        }
    }

    _keyEventQueue.clear();
}

void Menu::addToEventQueue(KeyEvent e) {
    _keyEventQueue.emplace_back(e);
}

void Menu::ProcessInputEvents(RE::InputEvent* const* a_event) {
    for (auto it = *a_event; it; it = it->next) {
        if (it->GetEventType() != RE::INPUT_EVENT_TYPE::kButton && it->GetEventType() != RE::INPUT_EVENT_TYPE::kChar) {
            continue;
        }

        auto event = it->GetEventType() == RE::INPUT_EVENT_TYPE::kButton ? KeyEvent(static_cast<RE::ButtonEvent*>(it)) : KeyEvent(static_cast<CharEvent*>(it));
        addToEventQueue(event);            
    }
}

bool Menu::ShouldSwallowInput() {
    return this->menuToggle;
}

const char* Menu::KeyIdToString(uint32_t key)
{
	if (key >= 256)
		return "";

	static const char* keyboard_keys_international[256] = {
		"", "Left Mouse", "Right Mouse", "Cancel", "Middle Mouse", "X1 Mouse", "X2 Mouse", "", "Backspace", "Tab", "", "", "Clear", "Enter", "", "",
		"Shift", "Control", "Alt", "Pause", "Caps Lock", "", "", "", "", "", "", "Escape", "", "", "", "",
		"Space", "Page Up", "Page Down", "End", "Home", "Left Arrow", "Up Arrow", "Right Arrow", "Down Arrow", "Select", "", "", "Print Screen", "Insert", "Delete", "Help",
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "", "", "", "", "", "",
		"", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
		"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Left Windows", "Right Windows", "Apps", "", "Sleep",
		"Numpad 0", "Numpad 1", "Numpad 2", "Numpad 3", "Numpad 4", "Numpad 5", "Numpad 6", "Numpad 7", "Numpad 8", "Numpad 9", "Numpad *", "Numpad +", "", "Numpad -", "Numpad Decimal", "Numpad /",
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16",
		"F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "", "", "", "", "", "", "", "",
		"Num Lock", "Scroll Lock", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
		"Left Shift", "Right Shift", "Left Control", "Right Control", "Left Menu", "Right Menu", "Browser Back", "Browser Forward", "Browser Refresh", "Browser Stop", "Browser Search", "Browser Favorites", "Browser Home", "Volume Mute", "Volume Down", "Volume Up",
		"Next Track", "Previous Track", "Media Stop", "Media Play/Pause", "Mail", "Media Select", "Launch App 1", "Launch App 2", "", "", "OEM ;", "OEM +", "OEM ,", "OEM -", "OEM .", "OEM /",
		"OEM ~", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
		"", "", "", "", "", "", "", "", "", "", "", "OEM [", "OEM \\", "OEM ]", "OEM '", "OEM 8",
		"", "", "OEM <", "", "", "", "", "", "", "", "", "", "", "", "", "",
		"", "", "", "", "", "", "Attn", "CrSel", "ExSel", "Erase EOF", "Play", "Zoom", "", "PA1", "OEM Clear", ""
	};

	return keyboard_keys_international[key];
}