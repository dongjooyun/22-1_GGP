#include "Renderer/Renderer.h"

namespace library
{

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                 m_immediateContext, m_immediateContext1, m_swapChain,
                 m_swapChain1, m_renderTargetView, m_depthStencil,
                 m_depthStencilView, m_cbChangeOnResize, m_camera,
                 m_projection, m_renderables, m_vertexShaders,
                 m_pixelShaders].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice(nullptr)
        , m_d3dDevice1(nullptr)
        , m_immediateContext(nullptr)
        , m_immediateContext1(nullptr)
        , m_swapChain(nullptr)
        , m_swapChain1(nullptr)
        , m_renderTargetView(nullptr)
        , m_depthStencil(nullptr)
        , m_depthStencilView(nullptr)
        , m_cbChangeOnResize(nullptr)
        , m_cbLights(nullptr)
        , m_pszMainSceneName(PCWSTR())
        , m_padding()
        , m_camera(Camera(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f)))
        , m_projection(XMMatrixIdentity())
        , m_renderables(std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>())
        , m_models(std::unordered_map<PCWSTR, std::shared_ptr<Model>>())
        , m_aPointLights{ std::shared_ptr<PointLight>() }
        , m_vertexShaders(std::unordered_map<PCWSTR, std::shared_ptr<VertexShader>>())
        , m_pixelShaders(std::unordered_map<PCWSTR, std::shared_ptr<PixelShader>>())
        , m_scenes(std::unordered_map<std::wstring, std::shared_ptr<Scene>>())
    {}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                 m_d3dDevice1, m_immediateContext1, m_swapChain1,
                 m_swapChain, m_renderTargetView, m_cbChangeOnResize,
                 m_projection, m_cbLights, m_camera, m_vertexShaders,
                 m_pixelShaders, m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT uWidth = static_cast<UINT>(rc.right - rc.left);
        UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

        // Clip cursor to screen
        POINT LeftTop =
        {
            .x = rc.left,
            .y = rc.top - 30
        };
        POINT RightBottom =
        {
            .x = rc.right,
            .y = rc.bottom
        };

        ClientToScreen(hWnd, &LeftTop);
        ClientToScreen(hWnd, &RightBottom);

        rc =
        {
            .left = LeftTop.x,
            .top = LeftTop.y,
            .right = RightBottom.x,
            .bottom = RightBottom.y
        };

        ClipCursor(&rc);

        UINT uCreateDeviceFlags = 0u;
#ifdef _DEBUG
        uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

        D3D_DRIVER_TYPE aDriverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT uNumDriverTypes = ARRAYSIZE(aDriverTypes);

        D3D_FEATURE_LEVEL aFeatureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT uNumFeatureLevels = ARRAYSIZE(aFeatureLevels);

        for (UINT driverTypeIndex = 0u; driverTypeIndex < uNumDriverTypes; ++driverTypeIndex)
        {
            m_driverType = aDriverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, aFeatureLevels, uNumFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &aFeatureLevels[1], uNumFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to D3D11CreateDevice failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Obtain DXGI factory from device
        ComPtr<IDXGIFactory1> dxgiFactory = nullptr;
        {
            ComPtr<IDXGIDevice> dxgiDevice = nullptr;
            if (SUCCEEDED(m_d3dDevice.As(&dxgiDevice)))
            {
                ComPtr<IDXGIAdapter> adapter = nullptr;
                if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter)))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
                }
            }
        }
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to DXGI factory setting failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;
        if (SUCCEEDED(dxgiFactory.As(&dxgiFactory2)))
        {
            // DirectX 11.1 or later
            if (SUCCEEDED(m_d3dDevice.As(&m_d3dDevice1)))
            {
                m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = uWidth,
                .Height = uHeight,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1u,
                               .Quality = 0u},
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, &m_swapChain1);
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc = {.Width = uWidth,
                               .Height = uHeight,
                               .RefreshRate = {.Numerator = 60u,
                                               .Denominator = 1u},
                               .Format = DXGI_FORMAT_R8G8B8A8_UNORM},
                .SampleDesc = {.Count = 1u,
                               .Quality = 0u},
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, &m_swapChain);
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateSwapChain failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
        hr = m_swapChain->GetBuffer(0u, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to GetBuffer failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateRenderTargetView failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = uWidth,
            .Height = uHeight,
            .MipLevels = 1u,
            .ArraySize = 1u,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1u,
                           .Quality = 0u},
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0u,
            .MiscFlags = 0u
        };
        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateDepthStencil failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0u}
        };
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateDepthStencilView failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1u, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(uWidth),
            .Height = static_cast<FLOAT>(uHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };
        m_immediateContext->RSSetViewports(1u, &vp);

        // Initialize the vertex shader
        std::unordered_map<PCWSTR, std::shared_ptr<VertexShader>>::iterator vertexShader;
        for (vertexShader = m_vertexShaders.begin(); vertexShader != m_vertexShaders.end(); ++vertexShader)
        {
            vertexShader->second->Initialize(m_d3dDevice.Get());
        }

        // Initialize the pixel shader
        std::unordered_map<PCWSTR, std::shared_ptr<PixelShader>>::iterator pixelShader;
        for (pixelShader = m_pixelShaders.begin(); pixelShader != m_pixelShaders.end(); ++pixelShader)
        {
            pixelShader->second->Initialize(m_d3dDevice.Get());
        }

        // Initialize the renderables
        std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>::iterator renderable;
        for (renderable = m_renderables.begin(); renderable != m_renderables.end(); ++renderable)
        {
            renderable->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        }

        // Initialize the models
        std::unordered_map<PCWSTR, std::shared_ptr<Model>>::iterator model;
        for (model = m_models.begin(); model != m_models.end(); ++model)
        {
            model->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        }

        // Create the projection matrix
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0u
        };
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateChangeOnResizeBuffer failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 100.0f);
        CBChangeOnResize cbChangeOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0u, nullptr, &cbChangeOnResize, 0u, 0u);

        // Create the camera constant buffer
        m_camera.Initialize(m_d3dDevice.Get());

        // Create lights constant buffer
        D3D11_BUFFER_DESC bdLight =
        {
            .ByteWidth = sizeof(CBLights),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0u
        };
        if (FAILED(m_d3dDevice->CreateBuffer(&bdLight, nullptr, m_cbLights.GetAddressOf())))
        {
            MessageBox(
                nullptr,
                L"Call to CreateCBLightsBuffer failed!",
                L"Game Graphics Programming",
                NULL
            );
        }

        // Initialize the voxels of scenes
        std::unordered_map<std::wstring, std::shared_ptr<Scene>>::iterator scene;
        for (scene = m_scenes.begin(); scene != m_scenes.end(); ++scene)
        {
            for (UINT i = 0u; i < scene->second->GetVoxels().size(); ++i)
            {
                scene->second->GetVoxels()[i]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            }
        }

        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddRenderable
      Summary:  Add a renderable object
      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Shared pointer to the renderable object
      Modifies: [m_renderables].
      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
    {
        if (m_renderables.find(pszRenderableName) != m_renderables.end())
        {
            MessageBox(nullptr, L"Cannot add renderable!", L"Error", NULL);
            return E_FAIL;
        }

        m_renderables.insert(std::make_pair(pszRenderableName, renderable));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddModel

      Summary:  Add a model object

      Args:     PCWSTR pszModelName
                  Key of the model object
                const std::shared_ptr<Model>& pModel
                  Shared pointer to the model object

      Modifies: [m_models].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddModel(_In_ PCWSTR pszModelName, _In_ const std::shared_ptr<Model>& pModel)
    {
        if (m_models.find(pszModelName) != m_models.end())
        {
            MessageBox(nullptr, L"Cannot add model!", L"Error", NULL);
            return E_FAIL;
        }

        m_models.insert(std::make_pair(pszModelName, pModel));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPointLight

      Summary:  Add a point light

      Args:     size_t index
                  Index of the point light
                const std::shared_ptr<PointLight>& pointLight
                  Shared pointer to the point light object

      Modifies: [m_aPointLights].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
    {
        if (index >= NUM_LIGHTS)
        {
            MessageBox(nullptr, L"Cannot add pointlight!", L"Error", NULL);
            return E_FAIL;
        }
        else
        {
            m_aPointLights[index] = pPointLight;
            return S_OK;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddVertexShader

      Summary:  Add the vertex shader into the renderer

      Args:     PCWSTR pszVertexShaderName
                  Key of the vertex shader
                const std::shared_ptr<VertexShader>&
                  Vertex shader to add

      Modifies: [m_vertexShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
    {
        if (m_vertexShaders.find(pszVertexShaderName) != m_vertexShaders.end())
        {
            MessageBox(nullptr, L"Cannot add vertex shader!", L"Error", NULL);
            return E_FAIL;
        }
        else
        {
            m_vertexShaders.insert(std::make_pair(pszVertexShaderName, vertexShader));
            return S_OK;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPixelShader

      Summary:  Add the pixel shader into the renderer

      Args:     PCWSTR pszPixelShaderName
                  Key of the pixel shader
                const std::shared_ptr<PixelShader>&
                  Pixel shader to add

      Modifies: [m_pixelShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
    {
        if (m_pixelShaders.find(pszPixelShaderName) != m_pixelShaders.end())
        {
            MessageBox(nullptr, L"Cannot add pixel shader!", L"Error", NULL);
            return E_FAIL;
        }
        else
        {
            m_pixelShaders.insert(std::make_pair(pszPixelShaderName, pixelShader));
            return S_OK;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene

      Summary:  Add a scene

      Args:     PCWSTR pszSceneName
                  Key of a scene
                const std::filesystem::path& sceneFilePath
                  File path to initialize a scene

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, const std::filesystem::path& sceneFilePath)
    {
        if (m_scenes.find(pszSceneName) != m_scenes.end())
        {
            MessageBox(nullptr, L"Cannot add scene!", L"Error", NULL);
            return E_FAIL;
        }
        else
        {
            m_scenes.insert(std::make_pair(pszSceneName, std::make_shared<Scene>(sceneFilePath)));
            return S_OK;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene

      Summary:  Set the main scene

      Args:     PCWSTR pszSceneName
                  Name of the scene to set as the main scene

      Modifies: [m_pszMainSceneName].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (m_scenes.find(pszSceneName) == m_scenes.end())
        {
            MessageBox(nullptr, L"Cannot set main scene!", L"Error", NULL);
            return E_FAIL;
        }

        m_pszMainSceneName = pszSceneName;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput

      Summary:  Add the pixel shader into the renderer and initialize it

      Args:     const DirectionsInput& directions
                  Data structure containing keyboard input data
                const MouseRelativeMovement& mouseRelativeMovement
                  Data structure containing mouse relative input data

      Modifies: [m_camera].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables and models each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        // Update the renderables
        std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>::iterator renderable;
        for (renderable = m_renderables.begin(); renderable != m_renderables.end(); ++renderable)
        {
            renderable->second->Update(deltaTime);
        }

        // Update the models
        std::unordered_map<PCWSTR, std::shared_ptr<Model>>::iterator model;
        for (model = m_models.begin(); model != m_models.end(); ++model)
        {
            model->second->Update(deltaTime);
        }

        // Update the lights
        for (UINT i = 0u; i < NUM_LIGHTS; ++i)
        {
            m_aPointLights[i]->Update(deltaTime);
        }

        // Update the voxels of main scene
        for (UINT i = 0u; i < m_scenes[m_pszMainSceneName]->GetVoxels().size(); ++i)
        {
            m_scenes[m_pszMainSceneName]->GetVoxels()[i]->Update(deltaTime);
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render()
    {
        // Clear the back buffer
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

        // Clear the depth buffer to 1.0 (max depth)
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

        // Update camera constant buffer
        CBChangeOnCameraMovement cbChangeOnCameraMovement =
        {
            .View = XMMatrixTranspose(m_camera.GetView()),
        };
        XMStoreFloat4(&cbChangeOnCameraMovement.CameraPosition, m_camera.GetEye());
        m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0u, nullptr, &cbChangeOnCameraMovement, 0u, 0u);

        // Update lights constant buffer
        CBLights cbLights = {};
        for (UINT i = 0u; i < NUM_LIGHTS; ++i)
        {
            cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
            cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
        }
        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cbLights, 0u, 0u);

        // For all renderables
        std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>::iterator renderable;
        for (renderable = m_renderables.begin(); renderable != m_renderables.end(); ++renderable)
        {
            // Set the vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, renderable->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

            // Set the index buffer
            m_immediateContext->IASetIndexBuffer(renderable->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Set the input layout
            m_immediateContext->IASetInputLayout(renderable->second->GetVertexLayout().Get());

            // Update renderable constant buffer
            CBChangesEveryFrame cbChangesEveryFrame =
            {
                .World = XMMatrixTranspose(renderable->second->GetWorldMatrix()),
                .OutputColor = renderable->second->GetOutputColor()
            };
            m_immediateContext->UpdateSubresource(renderable->second->GetConstantBuffer().Get(), 0u, nullptr, &cbChangesEveryFrame, 0u, 0u);

            // Set the vertex shader and constant buffers
            m_immediateContext->VSSetShader(renderable->second->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, renderable->second->GetConstantBuffer().GetAddressOf());

            // Set the pixel shader and constant buffers
            m_immediateContext->PSSetShader(renderable->second->GetPixelShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, renderable->second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            if (renderable->second->HasTexture())
            {
                for (UINT i = 0u; i < renderable->second->GetNumMeshes(); ++i)
                {
                    const UINT materialIndex = renderable->second->GetMesh(i).uMaterialIndex;
                    if (renderable->second->GetMaterial(materialIndex).pDiffuse)
                    {
                        // Set texture resource view of the renderable into the pixel shader
                        m_immediateContext->PSSetShaderResources(0u, 1u, renderable->second->GetMaterial(materialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());

                        // Set sampler state of the renderable into the pixel shader
                        m_immediateContext->PSSetSamplers(0u, 1u, renderable->second->GetMaterial(materialIndex).pDiffuse->GetSamplerState().GetAddressOf());
                    }

                    // Render the triangles
                    m_immediateContext->DrawIndexed(renderable->second->GetMesh(i).uNumIndices,
                        renderable->second->GetMesh(i).uBaseIndex,
                        renderable->second->GetMesh(i).uBaseVertex);
                }
            }
            else
            {
                // Render the triangles
                m_immediateContext->DrawIndexed(renderable->second->GetNumIndices(), 0u, 0);
            }
        }

        // For all voxels in main scene
        std::vector<std::shared_ptr<Voxel>>::iterator voxel;
        for (voxel = m_scenes[m_pszMainSceneName]->GetVoxels().begin(); voxel != m_scenes[m_pszMainSceneName]->GetVoxels().end(); ++voxel)
        {
            // Set the vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, voxel->get()->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

            // Set the instance buffer
            uStride = sizeof(InstanceData);
            m_immediateContext->IASetVertexBuffers(1u, 1u, voxel->get()->GetInstanceBuffer().GetAddressOf(), &uStride, &uOffset);

            // Set the index buffer
            m_immediateContext->IASetIndexBuffer(voxel->get()->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Set the input layout
            m_immediateContext->IASetInputLayout(voxel->get()->GetVertexLayout().Get());

            // Update voxel constant buffer
            CBChangesEveryFrame cbChangesEveryFrame =
            {
                .World = XMMatrixTranspose(voxel->get()->GetWorldMatrix()),
                .OutputColor = voxel->get()->GetOutputColor()
            };
            m_immediateContext->UpdateSubresource(voxel->get()->GetConstantBuffer().Get(), 0u, nullptr, &cbChangesEveryFrame, 0u, 0u);

            // Set the vertex shader and constant buffers
            m_immediateContext->VSSetShader(voxel->get()->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, voxel->get()->GetConstantBuffer().GetAddressOf());

            // Set the pixel shader and constant buffers
            m_immediateContext->PSSetShader(voxel->get()->GetPixelShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, voxel->get()->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            // Render the triangles
            m_immediateContext->DrawIndexedInstanced(voxel->get()->GetNumIndices(), voxel->get()->GetNumInstances(), 0u, 0, 0u);
        }

        // For all models
        std::unordered_map<PCWSTR, std::shared_ptr<Model>>::iterator model;
        for (model = m_models.begin(); model != m_models.end(); ++model)
        {
            // Set the vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0u;
            m_immediateContext->IASetVertexBuffers(0u, 1u, model->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

            uStride = sizeof(AnimationData);
            m_immediateContext->IASetVertexBuffers(1u, 1u, model->second->GetAnimationBuffer().GetAddressOf(), &uStride, &uOffset);

            // Set the index buffer
            m_immediateContext->IASetIndexBuffer(model->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            // Set primitive topology
            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Set the input layout
            m_immediateContext->IASetInputLayout(model->second->GetVertexLayout().Get());

            // Update renderable constant buffer
            CBChangesEveryFrame cbChangesEveryFrame =
            {
                .World = XMMatrixTranspose(model->second->GetWorldMatrix()),
                .OutputColor = model->second->GetOutputColor()
            };
            m_immediateContext->UpdateSubresource(model->second->GetConstantBuffer().Get(), 0u, nullptr, &cbChangesEveryFrame, 0u, 0u);

            // Update skinning constant buffer
            CBSkinning cbSkinning = {};
            for (UINT i = 0; i < model->second->GetBoneTransforms().size(); ++i)
            {
                cbSkinning.BoneTransforms[i] = XMMatrixTranspose(model->second->GetBoneTransforms()[i]);
            }
            m_immediateContext->UpdateSubresource(model->second->GetSkinningConstantBuffer().Get(), 0u, nullptr, &cbSkinning, 0u, 0u);

            // Set the vertex shader and constant buffers
            m_immediateContext->VSSetShader(model->second->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, model->second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(4u, 1u, model->second->GetSkinningConstantBuffer().GetAddressOf());

            // Set the pixel shader and constant buffers
            m_immediateContext->PSSetShader(model->second->GetPixelShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, model->second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            if (model->second->HasTexture())
            {
                for (UINT i = 0u; i < model->second->GetNumMeshes(); ++i)
                {
                    const UINT materialIndex = model->second->GetMesh(i).uMaterialIndex;
                    if (model->second->GetMaterial(materialIndex).pDiffuse)
                    {
                        // Set texture resource view of the renderable into the pixel shader
                        m_immediateContext->PSSetShaderResources(0u, 1u, model->second->GetMaterial(materialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());

                        // Set sampler state of the renderable into the pixel shader
                        m_immediateContext->PSSetSamplers(0u, 1u, model->second->GetMaterial(materialIndex).pDiffuse->GetSamplerState().GetAddressOf());
                    }

                    // Render the triangles
                    m_immediateContext->DrawIndexed(model->second->GetMesh(i).uNumIndices,
                        model->second->GetMesh(i).uBaseIndex,
                        model->second->GetMesh(i).uBaseVertex);
                }
            }
            else
            {
                // Render the triangles
                m_immediateContext->DrawIndexed(model->second->GetNumIndices(), 0u, 0);
            }
        }

        // Present the information rendered to the back buffer to the front buffer
        m_swapChain->Present(0u, 0u);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfRenderable

      Summary:  Sets the vertex shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
    {
        if (m_renderables.find(pszRenderableName) == m_renderables.end() || m_vertexShaders.find(pszVertexShaderName) == m_vertexShaders.end())
        {
            MessageBox(nullptr, L"Cannot set vertex shader of renderable!", L"Error", NULL);
            return E_FAIL;
        }

        m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfRenderable

      Summary:  Sets the pixel shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
    {
        if (m_renderables.find(pszRenderableName) == m_renderables.end() || m_pixelShaders.find(pszPixelShaderName) == m_pixelShaders.end())
        {
            MessageBox(nullptr, L"Cannot set pixel shader of renderable!", L"Error", NULL);
            return E_FAIL;
        }

        m_renderables[pszRenderableName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfModel

      Summary:  Sets the pixel shader for a model

      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszVertexShaderName)
    {
        if (m_models.find(pszModelName) == m_models.end() || m_vertexShaders.find(pszVertexShaderName) == m_vertexShaders.end())
        {
            MessageBox(nullptr, L"Cannot set vertex shader of model!", L"Error", NULL);
            return E_FAIL;
        }

        m_models[pszModelName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfModel

      Summary:  Sets the pixel shader for a model

      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszPixelShaderName)
    {
        if (m_models.find(pszModelName) == m_models.end() || m_pixelShaders.find(pszPixelShaderName) == m_pixelShaders.end())
        {
            MessageBox(nullptr, L"Cannot set pixel shader of model!", L"Error", NULL);
            return E_FAIL;
        }

        m_models[pszModelName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfScene

      Summary:  Sets the vertex shader for the voxels in a scene

      Args:     PCWSTR pszSceneName
                  Key of the scene
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName)
    {
        if (m_scenes.find(pszSceneName) == m_scenes.end() || m_vertexShaders.find(pszVertexShaderName) == m_vertexShaders.end())
        {
            MessageBox(nullptr, L"Cannot set vertex shader of scene!", L"Error", NULL);
            return E_FAIL;
        }

        for (UINT i = 0u; i < m_scenes[pszSceneName]->GetVoxels().size(); ++i)
        {
            m_scenes[pszSceneName]->GetVoxels()[i]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfScene

      Summary:  Sets the pixel shader for the voxels in a scene

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName)
    {
        if (m_scenes.find(pszSceneName) == m_scenes.end() || m_pixelShaders.find(pszPixelShaderName) == m_pixelShaders.end())
        {
            MessageBox(nullptr, L"Cannot set pixel shader of scene!", L"Error", NULL);
            return E_FAIL;
        }

        for (UINT i = 0u; i < m_scenes[pszSceneName]->GetVoxels().size(); ++i)
        {
            m_scenes[pszSceneName]->GetVoxels()[i]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType

      Summary:  Returns the Direct3D driver type

      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_driverType;
    }
}