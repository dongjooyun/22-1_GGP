#include "Game/Game.h"

namespace library
{
    /*--------------------------------------------------------------------
      Global Variables
    --------------------------------------------------------------------*/
    /*--------------------------------------------------------------------
      TODO: Initialize global variables (remove the comment)
    --------------------------------------------------------------------*/
    HINSTANCE               g_hInst = nullptr;
    HWND                    g_hWnd = nullptr;
    D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11Device1* g_pd3dDevice1 = nullptr;
    ID3D11DeviceContext* g_pImmediateContext = nullptr;
    ID3D11DeviceContext1* g_pImmediateContext1 = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;
    IDXGISwapChain1* g_pSwapChain1 = nullptr;
    ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
    /*--------------------------------------------------------------------
      Forward declarations
    --------------------------------------------------------------------*/
    HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
    HRESULT InitDevice();
    void CleanupDevice();
    LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
    void Render();
    /*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      Function: WindowProc

      Summary:  Defines the behavior of the window—its appearance, how
                it interacts with the user, and so forth

      Args:     HWND hWnd
                  Handle to the window
                UINT uMsg
                  Message code
                WPARAM wParam
                  Additional data that pertains to the message
                LPARAM lParam
                  Additional data that pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    -----------------------------------------------------------------F-F*/
    LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    /*--------------------------------------------------------------------
      TODO: Function definitions (remove this comment)
    --------------------------------------------------------------------*/

}

// Lab00
//#include "Game/Game.h"
//
//namespace library
//{
//    /*--------------------------------------------------------------------
//      Function definitions
//    --------------------------------------------------------------------*/
//    void PrintHi()
//    {
//        OutputDebugString(L"hi\n");
//        MessageBox(nullptr, L"hi", L"Game Graphics Programming", MB_OK);
//    }
//}