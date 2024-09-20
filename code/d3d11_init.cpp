#ifndef D3D11_INIT_C
#define D3D11_INIT_C

static void
d3d_init_debug_stuff(){
    ID3D11InfoQueue* info;
    d3d_device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info);
    info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
    info->Release();

    IDXGIInfoQueue* dxgi_info;
    HRESULT hr = DXGIGetDebugInterface1(0, __uuidof(IDXGIInfoQueue), (void**)&dxgi_info);
    assert_hr(hr);
    dxgi_info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    dxgi_info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
    dxgi_info->Release();
}

static void
d3d_load_shader(String8 shader_path, D3D11_INPUT_ELEMENT_DESC* il, u32 layout_count, ID3D11VertexShader** d3d_vs, ID3D11PixelShader** d3d_ps, ID3D11InputLayout** d3d_il){
    // ---------------------------------------------------------------------------------
    // Vertex/Pixel Shader
    // ---------------------------------------------------------------------------------
#if DEBUG
    u32 shader_compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    u32 shader_compile_flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    ScratchArena scratch = begin_scratch();
    //String8 utf8_shader_path = str8_path_append(scratch.arena, path_shaders, shader_file);
    String16 utf16_shader_path = os_utf16_from_utf8(scratch.arena, shader_path);

    ID3DBlob* vs_blob, *ps_blob, *error;
    hr = D3DCompileFromFile((wchar*)utf16_shader_path.str, 0, 0, "vs_main", "vs_5_0", shader_compile_flags, 0, &vs_blob, &error);
    if(FAILED(hr)) {
        print("Error: failed D3DCompileFromFile()\n");
        print("\tMessage: %s\n", (char*)error->GetBufferPointer());
        assert_hr(hr);
    }
    hr = d3d_device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), 0, d3d_vs);
    assert_hr(hr);

    hr = D3DCompileFromFile((wchar*)utf16_shader_path.str, 0, 0, "ps_main", "ps_5_0", shader_compile_flags, 0, &ps_blob, &error);
    if(FAILED(hr)) {
        print("Error: failed D3DCompileFromFile()\n");
        print("\tMessage: %s\n", (char*)error->GetBufferPointer());
        assert_hr(hr);
    }

    hr = d3d_device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), 0, d3d_ps);
    assert_hr(hr);

    // ---------------------------------------------------------------------------------
    // Input Layout
    // ---------------------------------------------------------------------------------
    hr = d3d_device->CreateInputLayout(il, layout_count, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), d3d_il);
    assert_hr(hr);

    end_scratch(scratch);
};

static void
d3d_init(HWND window_handle, s32 width, s32 height){
    // ---------------------------------------------------------------------------------
    // Device + Context
    // ---------------------------------------------------------------------------------
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    u32 flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ID3D11Device* base_device;
    ID3D11DeviceContext* base_device_context;
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &base_device, nullptr, &base_device_context);
    assert_hr(hr);

    hr = base_device->QueryInterface(__uuidof(ID3D11Device1), (void**)(&d3d_device));
    assert_hr(hr);

    hr = base_device_context->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)(&d3d_context));
    assert_hr(hr);

    // ---------------------------------------------------------------------------------
    // Swap chain
    // ---------------------------------------------------------------------------------
    IDXGIDevice1* dxgiDevice;
    hr = d3d_device->QueryInterface(__uuidof(IDXGIDevice1), (void**)(&dxgiDevice));
    assert_hr(hr);

    IDXGIAdapter* dxgiAdapter;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    assert_hr(hr);

    IDXGIFactory2* dxgiFactory;
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)(&dxgiFactory));
    assert_hr(hr);

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    swapChainDesc.Width              = 0; // use window width
    swapChainDesc.Height             = 0; // use window height
    swapChainDesc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    swapChainDesc.Stereo             = FALSE;
    swapChainDesc.SampleDesc.Count   = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount        = 2;
    swapChainDesc.Scaling            = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags              = 0;

    dxgiFactory->CreateSwapChainForHwnd(d3d_device, window_handle, &swapChainDesc, 0, 0, &d3d_swapchain);

    // ---------------------------------------------------------------------------------
    // Frame Buffer
    // ---------------------------------------------------------------------------------
    hr = d3d_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d_framebuffer);
    assert_hr(hr);

    hr = d3d_device->CreateRenderTargetView(d3d_framebuffer, 0, &d3d_framebuffer_view);
    assert_hr(hr);

    // ---------------------------------------------------------------------------------
    // Rasterizer State
    // ---------------------------------------------------------------------------------
    D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_NONE;
    rasterizer_desc.FrontCounterClockwise = false;

    hr = d3d_device->CreateRasterizerState1(&rasterizer_desc, &d3d_rasterizer_state);
    assert_hr(hr);

    // ---------------------------------------------------------------------------------
    // Sampler State for textures
    // ---------------------------------------------------------------------------------
    D3D11_SAMPLER_DESC sampler_desc = {
        .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
        .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
        .MipLODBias = 2.0f,
        .MinLOD = 0.0f,
        .MaxLOD = FLT_MAX,
    };
    hr = d3d_device->CreateSamplerState(&sampler_desc, &d3d_sampler_state);
    assert_hr(hr);

    // ---------------------------------------------------------------------------------
    // Blend State
    // ---------------------------------------------------------------------------------
    D3D11_BLEND_DESC blend_desc = {0};
    blend_desc.AlphaToCoverageEnable = false;
    blend_desc.IndependentBlendEnable = false;
    blend_desc.RenderTarget[0].BlendEnable = true;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = d3d_device->CreateBlendState(&blend_desc, &d3d_blend_state);
    assert_hr(hr);

    // ---------------------------------------------------------------------------------
    // Viewport
    // ---------------------------------------------------------------------------------
    // NOTE: can be used to create split screens for 2+ players
    d3d_viewport = {0};
    d3d_viewport.TopLeftX = 0;
    d3d_viewport.TopLeftY = 0;
    d3d_viewport.Width = (f32)width;
    d3d_viewport.Height = (f32)height;
    d3d_viewport.MinDepth = 0;
    d3d_viewport.MaxDepth = 1.0;

    // ---------------------------------------------------------------------------------
    // Vertex Buffers
    // ---------------------------------------------------------------------------------
    {
        D3D11_BUFFER_DESC desc = {0};
        desc.ByteWidth = MB(8);
        desc.Usage     = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = d3d_device->CreateBuffer(&desc, 0, &d3d_vertex_buffer_8mb);
        assert_hr(hr);
    }

    // ---------------------------------------------------------------------------------
    // Index Buffers
    // ---------------------------------------------------------------------------------
    {
        D3D11_BUFFER_DESC desc = {0};
        desc.ByteWidth = MB(8);
        desc.Usage     = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = d3d_device->CreateBuffer(&desc, 0, &d3d_index_buffer);
        assert_hr(hr);
    }

    // ---------------------------------------------------------------------------------
    // Instance Buffers
    // ---------------------------------------------------------------------------------
    {
        D3D11_BUFFER_DESC desc = {0};
        desc.ByteWidth = MB(8);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = d3d_device->CreateBuffer(&desc, 0, &d3d_instance_buffer);
        assert_hr(hr);
    }

    // ---------------------------------------------------------------------------------
    // Constant Buffer
    // ---------------------------------------------------------------------------------
    {
        D3D11_BUFFER_DESC desc = {0};
        desc.ByteWidth = sizeof(ConstantBuffer);
        desc.Usage     = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = d3d_device->CreateBuffer(&desc, 0, &d3d_constant_buffer);
        assert_hr(hr);
    }

    // ---------------------------------------------------------------------------------
    // White Texture
    // ---------------------------------------------------------------------------------
    {
        D3D11_TEXTURE2D_DESC desc = {
            .Width = (u32)1,
            .Height = (u32)1,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
            .SampleDesc = {1, 0},
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE,
        };

        u32 white = 0xFFFFFFFF;
        D3D11_SUBRESOURCE_DATA data = {
            .pSysMem = &white,
            .SysMemPitch = sizeof(u32),
        };

        hr = d3d_device->CreateTexture2D(&desc, &data, &white_texture);
        assert_hr(hr);

        hr = d3d_device->CreateShaderResourceView(white_texture, 0, &white_shader_resource);
        assert_hr(hr);
        white_texture->Release();
    }

    // ---------------------------------------------------------------------------------
    // First Time Initialization For Convenience
    // ---------------------------------------------------------------------------------
    {
        d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, 0);
    }

    base_device->Release();
    base_device_context->Release();
    dxgiAdapter->Release();
    dxgiFactory->Release();
    dxgiDevice->Release();
}

static void
d3d_release(){
    if(d3d_device) d3d_device->Release();
    if(d3d_context) d3d_context->Release();
    d3d_swapchain->SetFullscreenState(false, 0);
    if(d3d_swapchain) d3d_swapchain->Release();

    if(d3d_framebuffer) d3d_framebuffer->Release();
    if(d3d_depthbuffer) d3d_depthbuffer->Release();
    if(d3d_framebuffer_view) d3d_framebuffer_view->Release();
    if(d3d_depthbuffer_view) d3d_depthbuffer_view->Release();

    //d3d_texture->Release();
    //d3d_shader_resource->Release();

    if(d3d_2d_quad_vs) d3d_2d_quad_vs->Release();
    if(d3d_2d_quad_ps)  d3d_2d_quad_ps->Release();
    if(d3d_2d_quad_il)  d3d_2d_quad_il->Release();
    if(d3d_2d_textured_vs) d3d_2d_textured_vs->Release();
    if(d3d_2d_textured_ps)  d3d_2d_textured_ps->Release();
    if(d3d_2d_textured_il)  d3d_2d_textured_il->Release();

    if(d3d_vertex_buffer_8mb) d3d_vertex_buffer_8mb->Release();
    if(d3d_index_buffer)      d3d_index_buffer->Release();
    if(d3d_constant_buffer)   d3d_constant_buffer->Release();
    if(d3d_instance_buffer)   d3d_instance_buffer->Release();

    if(d3d_blend_state)        d3d_blend_state->Release();
    if(d3d_sampler_state)      d3d_sampler_state->Release();
    if(d3d_rasterizer_state)   d3d_rasterizer_state->Release();
    if(d3d_depthstencil_state) d3d_depthstencil_state->Release();

    //if(image_texture)  image_texture->Release();
    //if(ship_texture)   ship_texture->Release();
    //if(tree_texture)   tree_texture->Release();
    //if(circle_texture) circle_texture->Release();
    //if(bullet_texture) bullet_texture->Release();
    //if(test_texture)   test_texture->Release();

    if(image_shader_resource)  image_shader_resource->Release();
    if(ship_shader_resource)   ship_shader_resource->Release();
    if(tree_shader_resource)   tree_shader_resource->Release();
    if(circle_shader_resource) circle_shader_resource->Release();
    if(bullet_shader_resource) bullet_shader_resource->Release();
    if(test_shader_resource)   test_shader_resource->Release();
    if(white_shader_resource) white_shader_resource->Release();

#ifdef DEBUG
    IDXGIDebug1* pDxgiDebug;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDxgiDebug)))) {
        pDxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        pDxgiDebug->Release();
    }
#endif
}

#endif
