#include "include/screen_shot/screen_shot_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>
#include <iostream>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>
#include "include/screen_shot/screen_shot_plugin.h"
#include "include/screen_shot/encodable_value.h"
#include <wincodec.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include "include/screen_shot/strsafe.h"
#include <map>
#include <memory>
#include <sstream>

namespace {

class ScreenShotPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  ScreenShotPlugin();

  virtual ~ScreenShotPlugin();

 private:
 enum SaveFormat
{
    SaveFormatDefault = 1,
    SaveFormatBmp = 0,
    SaveFormatJpg = 1,
    SaveFormatPng = 2,
    SaveFormatTiff = 3,
    SaveFormatGif = 4
};
 bool FileExists(LPTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);
    bool b = !!(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));

    return b;
}
REFGUID WicGuidFromSaveFormat(SaveFormat format)
{
    switch (format)
    {
        case SaveFormatBmp:
        {
            return GUID_ContainerFormatBmp;
        }

        case SaveFormatPng:
        {
            return GUID_ContainerFormatPng;
        }

        case SaveFormatGif:
        {
            return GUID_ContainerFormatGif;
        }

        case SaveFormatTiff:
        {
            return GUID_ContainerFormatTiff;
        }

        case SaveFormatJpg:
        {
            return GUID_ContainerFormatJpeg;
        }

        default:
        {
            return GUID_ContainerFormatJpeg;
        }
    }
}

 HRESULT CreateSaveFile(LPTSTR pszFile, HBITMAP hBMP) 
{
    int i = 1;
    WCHAR filePathWithExtension[MAX_PATH] = {};
    
    StringCchPrintfW(filePathWithExtension,
        _countof(filePathWithExtension),
        L"%s.jpg",
        pszFile);

    while (FileExists(filePathWithExtension))
    {
        StringCchPrintfW(filePathWithExtension,
        _countof(filePathWithExtension),
        L"%s %d.jpg",
        pszFile,
        i);

        i++;
    }
    *pszFile = *filePathWithExtension;

    IWICImagingFactory2* pFactory = nullptr;
    IWICBitmap* pBitmap = nullptr;
    IWICStream* pStream = nullptr;
    IWICBitmapEncoder* pEncoder = nullptr;
    IWICBitmapFrameEncode* pFrame = nullptr;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    // Create the WIC factory
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(
            CLSID_WICImagingFactory2,
            nullptr,
            CLSCTX_INPROC_SERVER,
            __uuidof(IWICImagingFactory2),
            reinterpret_cast<void**>(&pFactory));
    }

    BITMAP bm;
    GetObject(hBMP, sizeof(bm), &bm);
    SIZE size = {bm.bmWidth, bm.bmHeight};


    // Create an IWICBitmap from our hBmp
    if (SUCCEEDED(hr))
    {
        hr = pFactory->CreateBitmapFromHBITMAP(
            hBMP,
            NULL,
            WICBitmapUsePremultipliedAlpha,
            &pBitmap);
    }

    // Create the destination file stream and initialize from the filename
    if (SUCCEEDED(hr))
    {
        hr = pFactory->CreateStream(&pStream);
    }

    if (SUCCEEDED(hr))
    {
        hr = pStream->InitializeFromFilename(filePathWithExtension, GENERIC_WRITE);
    }

    // Create our encoder
    if (SUCCEEDED(hr))
    {
        hr = pFactory->CreateEncoder(
            WicGuidFromSaveFormat(SaveFormatJpg),
            &GUID_VendorMicrosoftBuiltIn,
            &pEncoder);
    }

    // Initialize the decoder to write to the file
    if (SUCCEEDED(hr))
    {
        hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
    }

    // Create a new frame and initialize
    if (SUCCEEDED(hr))
    {
        hr = pEncoder->CreateNewFrame(&pFrame, nullptr);
    }

    if (SUCCEEDED(hr))
    {
        hr = pFrame->Initialize(nullptr /*pIEncoderOptions*/);
    }

    // Set the size of the output
    if (SUCCEEDED(hr))
    {
        hr = pFrame->SetSize(size.cx, size.cy);
    }

    // Write our bitmap in the frame
    if (SUCCEEDED(hr))
    {
        // WICRect rc = { 0, 0, size.cx, size.cy };

        hr = pFrame->WriteSource(pBitmap, nullptr);
    }

    // Commit everything
    if (SUCCEEDED(hr))
    {
        hr = pFrame->Commit();
    }

    if (SUCCEEDED(hr))
    {
        hr = pEncoder->Commit();
    }

    // Cleanup
    if (pFrame != nullptr)
    {
        pFrame->Release();
    }

    if (pEncoder != nullptr)
    {
        pEncoder->Commit();
    }

    if (pStream != nullptr)
    {
        pStream->Commit(STGC_DEFAULT);
        
        pStream->Release();
    }

    if (pBitmap != nullptr)
    {
        pBitmap->Release();
    }

    if (pFactory != nullptr)
    {
        pFactory->Release();
    }

    return hr;
}
 HBITMAP tackScreenShot(int x1,int y1,int w,int h,std::string fileName,std::string imageName){

    // get screen dimensions
    x1 = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y1 = GetSystemMetrics(SM_YVIRTUALSCREEN);
    
   

    // copy screen to bitmap
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
    HGDIOBJ oldObj = SelectObject(hDC, hBitmap);
    BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);

    
    // save bitmap to clipboard
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_BITMAP, hBitmap);
    // 
  WCHAR g_currentSnipPath[MAX_PATH] = {};
  // convert from string to LPCWSTR
  std::wstring stemp = std::wstring(fileName.begin(), fileName.end());
    std::wstring pImageName = std::wstring(imageName.begin(), imageName.end());

  ExpandEnvironmentStrings(
            stemp.c_str(),
            g_currentSnipPath,
            _countof(g_currentSnipPath));
            // 
   WCHAR filePath[MAX_PATH] = {};
    StringCchPrintfW(filePath,
        _countof(filePath),
        pImageName.c_str(),
        g_currentSnipPath,
        "s", "d", "fa");
    CreateSaveFile(filePath,hBitmap);
    CloseClipboard();

    // clean up
    SelectObject(hDC, oldObj);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    DeleteObject(hBitmap);

    return hBitmap;
}

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

// static
void ScreenShotPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "screen_shot",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<ScreenShotPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

ScreenShotPlugin::ScreenShotPlugin() {}

ScreenShotPlugin::~ScreenShotPlugin() {}

void ScreenShotPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("screenshot") == 0) {
    const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
       std::string license;
       int x1=0;
       int y1=0;
       int w=0;
       int h=0;
       std::string path = "";
       std::string imageName = "";
         if (arguments)
      {
        auto value_x1 = arguments->find(flutter::EncodableValue("x1"));
        auto value_y1 = arguments->find(flutter::EncodableValue("y1"));
        auto value_w = arguments->find(flutter::EncodableValue("w"));
        auto value_h = arguments->find(flutter::EncodableValue("h"));
        auto value_path = arguments->find(flutter::EncodableValue("path"));
                auto value_imageName = arguments->find(flutter::EncodableValue("imageName"));

        if (value_x1 != arguments->end())
        {
          x1 = std::get<int>(value_x1->second);
        }
         if (value_y1 != arguments->end())
        {
          y1 = std::get<int>(value_y1->second);
        }
         if (value_w != arguments->end())
        {
          w = std::get<int>(value_w->second);
        }
          if (value_h != arguments->end())
        {
          h = std::get<int>(value_h->second);
        }
         if (value_path != arguments->end())
        {
          path = std::get<std::string>(value_path->second);
        }
         if (value_imageName != arguments->end())
        {
          imageName = std::get<std::string>(value_imageName->second);
        }
        
      }
            result->Success(flutter::EncodableValue(tackScreenShot(x1,y1,w,h,path,imageName)));

  } else {
    result->NotImplemented();
  }
}

}  // namespace

void ScreenShotPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  ScreenShotPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
