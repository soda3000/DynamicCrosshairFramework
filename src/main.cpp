#include <SKSE/SKSE.h>
#include <SKSE/API.h>
#include <SKSE/Events.h>
#include <SKSE/Logger.h>
#include <SKSE/Trampoline.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "CrosshairMonitor.h"
#include "CrosshairUI.h"
#include "Menu.h"

#define DLLEXPORT __declspec(dllexport)
using namespace std;
namespace logger = SKSE::log;

namespace {
    constexpr const char*    G_PLUGIN_NAME = "DynamicCrosshairFramework";
    constexpr std::uint16_t  G_PLUGIN_VERSION_MAJOR = 0;
    constexpr std::uint16_t  G_PLUGIN_VERSION_MINOR = 1;
    constexpr std::uint16_t  G_PLUGIN_VERSION_PATCH = 0;
}; // End anonymous namespace

void MessageListener(SKSE::MessagingInterface::Message* msg) {
    switch (msg->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            logger::info("Game data loaded. Initializing UI components...");

            // Initialize CrosshairUI
            auto crosshairUI = CrosshairUI::GetSingleton();
            if (!crosshairUI->Init()) {
                logger::error("Failed to initialize CrosshairUI after data load");
            } else {
                logger::info("CrosshairUI initialized successfully after data load");
            }

            // Initialize Menu
            auto menu = Menu::GetSingleton();
            if (!menu->Init()) {
                logger::error("Failed to initialize Menu after data load");
            } else {
                logger::info("Menu initialized successfully after data load");
            }
            break;
    }
}


void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) {
        SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    }
    auto logFilePath = *logsFolder / "DynamicCrosshairFramework.log";
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SetupLog();
    logger::info("{} v{}.{}.{} loaded", G_PLUGIN_NAME, G_PLUGIN_VERSION_MAJOR, G_PLUGIN_VERSION_MINOR, G_PLUGIN_VERSION_PATCH);

    auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging) {
        logger::critical("Failed to get messaging interface. UI components cannot be initialized.");
        return false; // If messaging is unavailable, UI initialization will fail.
    }

    // Register message listener
    messaging->RegisterListener(MessageListener);
    logger::info("Registered message listener for UI initialization.");

    // Initialize CrosshairMonitor (can stay here if it doesn't need kDataLoaded)
    CrosshairMonitor::Init();
    logger::info("CrosshairMonitor initialized successfully");

    return true;
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
    SKSE::PluginVersionData v;
    v.PluginName(G_PLUGIN_NAME);
    v.PluginVersion({G_PLUGIN_VERSION_MAJOR, G_PLUGIN_VERSION_MINOR, G_PLUGIN_VERSION_PATCH});
    v.UsesAddressLibrary();
    v.UsesNoStructs(); // Ensure this is appropriate for your plugin, if you use SKSE structures, set it accordingly or remove.
    return v;
    }();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo) {
    pluginInfo->name = SKSEPlugin_Version.pluginName;
    pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
    pluginInfo->version = SKSEPlugin_Version.pluginVersion;
    return true;
}

void PresentCallback(IDXGISwapChain* /*a_swapChain*/, UINT /*a_syncInterval*/, UINT /*a_flags*/) {
    auto menu = Menu::GetSingleton();
    if (menu && menu->initialized) {
        // THIS IS WHERE IT'S CALLED:
        menu->ProcessInputEventQueue(); // Process queued inputs before drawing

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        menu->DrawMenu(); // Draw the actual menu

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}