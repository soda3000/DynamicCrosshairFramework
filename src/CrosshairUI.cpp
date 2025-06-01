#include "CrosshairUI.h"
#include "SKSE/Interfaces.h"
#include "RE/Skyrim.h"
#include "RE/R/Renderer.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <wincodec.h>

namespace logger = SKSE::log;

bool CrosshairUI::Init() {
    if (initialized) return true;

    auto* renderer = RE::BSGraphics::Renderer::GetSingleton();
    if (!renderer) {
        logger::error("Failed to get BSGraphics::Renderer singleton for crosshair UI.");
        return false;
    }
    
    REX::W32::ID3D11Device* device = RE::BSGraphics::Renderer::GetDevice();
    auto* rendererData = RE::BSGraphics::Renderer::GetRendererDataSingleton();
    if (!device || !rendererData || !rendererData->context) {
        logger::error("Failed to get device, renderer data, or context for crosshair UI.");
        return false;
    }

    device->AddRef();
    rendererData->context->AddRef();
    d3d_device = reinterpret_cast<ID3D11Device*>(device);
    d3d_context = reinterpret_cast<ID3D11DeviceContext*>(rendererData->context);

    auto* currentWindow = RE::BSGraphics::Renderer::GetCurrentRenderWindow();
    if (!currentWindow) {
        logger::error("Failed to get current render window for crosshair UI.");
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    
    io.MouseDrawCursor = false;
    io.WantCaptureMouse = false;
    io.WantCaptureKeyboard = false;
    
    ImGui::StyleColorsDark();
    
    if (!ImGui_ImplWin32_Init(currentWindow->hWnd)) {
        logger::error("Failed to initialize ImGui_ImplWin32 for crosshair UI.");
        return false;
    }

    if (!ImGui_ImplDX11_Init(d3d_device, d3d_context)) {
        logger::error("Failed to initialize ImGui_ImplDX11 for crosshair UI.");
        return false;
    }

    initialized = true;
    logger::info("Successfully initialized ImGui for crosshair UI.");
    return true;
}

void CrosshairUI::Shutdown() {
    if (!initialized) return;
    
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    
    initialized = false;
    d3d_device = nullptr;
    d3d_context = nullptr;
    logger::info("Successfully shutdown ImGui for crosshair UI.");
}

void CrosshairUI::BeginFrame() {
    if (!initialized) return;
    
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void CrosshairUI::EndFrame() {
    if (!initialized) return;
    
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

ID3D11ShaderResourceView* CrosshairUI::LoadTextureFromFile(const std::string& filePath) {
    logger::info("Loading texture from file: {}...", filePath);
    if (!d3d_device) {
        logger::error("Failed to load texture: D3D device is not initialized.");
        return nullptr;
    }
    
    std::wstring wFilePath(filePath.begin(), filePath.end());
    logger::info("Converted string to wide path, attempting WIC factory creation...");

    Microsoft::WRL::ComPtr<IWICImagingFactory> wicFactory;
    HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
    if (FAILED(hr)) {
        logger::error("Failed to create WIC factory: 0x{:08X}", static_cast<uint32_t>(hr));
        return nullptr;
    }
    logger::info("Successfully created WIC factory, attempting to create decoder...");
    
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    hr = wicFactory->CreateDecoderFromFilename(wFilePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) {
        logger::error("Failed to create decoder for file: {} - HRESULT: 0x{:08X}", filePath, static_cast<uint32_t>(hr));
        logger::error("This usually means the file doesn't exist or isn't accessible");
        return nullptr;
    }
    logger::info("Successfully created decoder, attempting to create frame...");
    
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        logger::error("Failed to get frame from image.");
        return nullptr;
    }
    logger::info("Successfully created frame, attempting to convert format...");
    
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        logger::error("Failed to create format converter.");
        return nullptr;
    }
    hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) {
        logger::error("Failed to initialize format converter.");
        return nullptr;
    }
    logger::info("Successfully initialized format converter, attempting to get image dimensions.");
    
    UINT width, height;
    hr = converter->GetSize(&width, &height);
    if (FAILED(hr)) {
        logger::error("Failed to get image dimensions.");
        return nullptr;
    }
    logger::info("Successfully got image dimensions, attempting to copy pixel data.");
    
    std::vector<BYTE> pixels(width * height * 4);
    hr = converter->CopyPixels(nullptr, width * 4, static_cast<UINT>(pixels.size()), pixels.data());
    if (FAILED(hr)) {
        logger::error("Failed to copy pixel data.");
        return nullptr;
    }
    logger::info("Successfully copied pixel data, attempting to create D3D11 texture.");
    
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = width * 4;
    
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    hr = d3d_device->CreateTexture2D(&textureDesc, &initData, &texture);
    if (FAILED(hr)) {
        logger::error("Failed to create D3D11 texture.");
        return nullptr;
    }
    logger::info("Successfully created D3D11 texture, attempting to create shader resource view.");
    
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    
    ID3D11ShaderResourceView* srv = nullptr;
    hr = d3d_device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv);
    if (FAILED(hr)) {
        logger::error("Failed to create shader resource view.");
        return nullptr;
    }
    logger::info("Successfully created shader resource view.");
    return srv;
}

void CrosshairUI::ReleaseTexture(ID3D11ShaderResourceView* texture) {
    if (texture) {
        texture->Release();
    }
}
