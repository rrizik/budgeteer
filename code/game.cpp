#ifndef GAME_C
#define GAME_C


static void
update_game(Window* window, Memory* memory, Events* events){

    //----constant buffer----
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;
    d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    ConstantBuffer2D* constants = (ConstantBuffer2D*)mapped_subresource.pData;
    constants->screen_res = make_v2s32(window->width, window->height);
    d3d_context->Unmap(d3d_constant_buffer, 0);
}

#endif

