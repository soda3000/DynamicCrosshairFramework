#pragma once
#include "CrosshairMonitor.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <map>
#include <string>

class CrosshairUI {
    public:
        static CrosshairUI* GetSingleton() {
            static CrosshairUI singleton;
            return &singleton;
        };

        bool Init();
        void Shutdown();
        void BeginFrame();
        void EndFrame();
        ID3D11ShaderResourceView* LoadTextureFromFile(const std::string& filePath);
        void ReleaseTexture(ID3D11ShaderResourceView* texture);
        void UpdateCrosshairType(CrosshairMonitor::InteractionType iType);

    private:
        CrosshairUI() = default;

        bool initialized = false;
        ID3D11Device* d3d_device = nullptr;
        ID3D11DeviceContext* d3d_context = nullptr;

        bool GetD3D11DeviceAndContext();
        CrosshairMonitor::InteractionType currentType = CrosshairMonitor::InteractionType::kNone;

        std::map<CrosshairMonitor::InteractionType, ImTextureID> crosshairTextures;

        // Dimensions
        float crosshairSize = 32.0f; // Default size
        ImVec2 screenCenter;

        void LoadTextures();
};