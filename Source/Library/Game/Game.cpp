﻿#include "Game/Game.h"

namespace library
{
    /*--------------------------------------------------------------------
      Global Variables
    --------------------------------------------------------------------*/
    HINSTANCE               g_hInst = nullptr;
    HWND                    g_hWnd = nullptr;
    D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
    ComPtr<ID3D11Device> g_pD3dDevice;
    ComPtr<ID3D11Device1> g_pD3dDevice1;
    ComPtr<ID3D11DeviceContext> g_pImmediateContext;
    ComPtr<ID3D11DeviceContext1> g_pImmediateContext1;
    ComPtr<IDXGISwapChain> g_pSwapChain;
    ComPtr<IDXGISwapChain1> g_pSwapChain1;
    ComPtr<ID3D11RenderTargetView> g_pRenderTargetView;
    /*--------------------------------------------------------------------
      Forward declarations
    --------------------------------------------------------------------*/
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
      Function definitions
    --------------------------------------------------------------------*/
    LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        switch (message)
        {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            // TRY: check close
            //if (MessageBox(hWnd, L"Really quit?", L"Check Closing", MB_OKCANCEL) == IDOK)
            //{
            //    //DestroyWindow(hWnd);
            //    PostQuitMessage(0);
            //}
            //// Else: User canceled. Do nothing.
            //break;

        //case WM_SIZE:
            // TRY: resizing

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

    /*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
      Function: InitWindow

      Summary:  Registers the window class and creates a window

      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                  Is a flag that says whether the main application window
                  will be minimized, maximized, or shown normally

        Returns:  HRESULT
                    Status code
    F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
    HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
    {
        // Register the window class
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = L"Lab01WindowClass";
        wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
        if (!RegisterClassEx(&wcex))
            return E_FAIL;

        // Create a window
        g_hInst = hInstance;
        RECT rc = { 0, 0, 800, 600 };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        g_hWnd = CreateWindow(L"Lab01WindowClass", L"Game Graphics Programming Lab01: Direct3D 11 Basics",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
            nullptr);
        if (!g_hWnd)
            return E_FAIL;

        // Show the window
        ShowWindow(g_hWnd, nCmdShow);

        return S_OK;
    }

    /*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
      Function: InitDevice

      Summary:  Create Direct3D device and swap chain

      Returns:  HRESULT
                  Status code
    F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
    HRESULT InitDevice()
    {
        // Create Direct3D 11 device and context
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(g_hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        DWORD createDeviceFlags = 0;
        #ifdef _DEBUG
            createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif
        
        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            g_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, g_pD3dDevice.GetAddressOf(), &g_featureLevel, g_pImmediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, g_pD3dDevice.GetAddressOf(), &g_featureLevel, g_pImmediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        // Obtain DXGI factory from device
        ComPtr<IDXGIFactory1> dxgiFactory = nullptr;
        {
            ComPtr<IDXGIDevice> dxgiDevice = nullptr;
            if (SUCCEEDED(g_pD3dDevice.As(&dxgiDevice)))
            {
                ComPtr<IDXGIAdapter> adapter = nullptr;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
                    //adapter->Release(); // ∵ ComPtr: Self-Release
                }
                //dxgiDevice->Release();
            }
        }
        if (FAILED(hr))
            return hr;

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;
        if (SUCCEEDED(dxgiFactory.As(&dxgiFactory2)))
        {
            if (SUCCEEDED(g_pD3dDevice.As(&g_pD3dDevice1)))
            {
                (void)g_pImmediateContext.As(&g_pImmediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd = {};
            sd.Width = width;
            sd.Height = height;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.BufferCount = 1;

            hr = dxgiFactory2->CreateSwapChainForHwnd(g_pD3dDevice.Get(), g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
            if (SUCCEEDED(hr))
            {
                hr = g_pSwapChain1.As(&g_pSwapChain);
            }

            //dxgiFactory2->Release(); 
        }
        else
        {
            DXGI_SWAP_CHAIN_DESC sd = {};
            sd.BufferCount = 1;
            sd.BufferDesc.Width = width;
            sd.BufferDesc.Height = height;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow = g_hWnd;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.Windowed = TRUE;

            hr = dxgiFactory->CreateSwapChain(g_pD3dDevice.Get(), &sd, &g_pSwapChain);
        }

        //dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER); // for resizing

        //dxgiFactory->Release(); 

        if (FAILED(hr))
            return hr;

        // Create render target view
        ComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
        hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
            return hr;

        hr = g_pD3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, g_pRenderTargetView.GetAddressOf());
        //pBackBuffer->Release();
        if (FAILED(hr))
        {
            return hr;
        }

        g_pImmediateContext->OMSetRenderTargets(1, g_pRenderTargetView.GetAddressOf(), nullptr);

        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;

        g_pImmediateContext->RSSetViewports(1, &vp);

        return S_OK;
    }

    /*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
      Function: CleanupDevice

      Summary:  Clean up the objects we've created
    F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
    void CleanupDevice()
    {
        if (g_pImmediateContext) g_pImmediateContext->ClearState();

        /*if (g_pRenderTargetView) g_pRenderTargetView->Release();
        if (g_pSwapChain1) g_pSwapChain1->Release();
        if (g_pSwapChain) g_pSwapChain->Release();
        if (g_pImmediateContext1) g_pImmediateContext1->Release();
        if (g_pImmediateContext) g_pImmediateContext->Release();
        if (g_pD3dDevice1) g_pD3dDevice1->Release();
        if (g_pD3dDevice) g_pD3dDevice->Release();*/
    }

    /*F+F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F+++F
      Function: Render

      Summary:  Render the frame
    F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F---F-F*/
    void Render()
    {
        // Just clear the backbuffer
        g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView.Get(), Colors::Gold);
        g_pSwapChain->Present(0, 0);
    }
}