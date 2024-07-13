#include "main.hpp"

// todo: move header includes here
#include "wave.cpp"
#include "bitmap.cpp"
#include "wasapi.cpp"
#include "d3d11_init.cpp"
#include "d3d11_render.cpp"
#include "font.cpp"
#include "console.cpp"
#include "command.cpp"
#include "game.cpp"

static void
init_paths(Arena* arena){
    build_path = os_application_path(global_arena);
    fonts_path = str8_path_append(global_arena, build_path, str8_literal("fonts"));
    shaders_path = str8_path_append(global_arena, build_path, str8_literal("shaders"));
    saves_path = str8_path_append(global_arena, build_path, str8_literal("saves"));
    sprites_path = str8_path_append(global_arena, build_path, str8_literal("sprites"));
    sounds_path = str8_path_append(global_arena, build_path, str8_literal("sounds"));
}

static void
memory_init(){
    memory.permanent_size = MB(500);
    memory.transient_size = GB(1);
    memory.size = memory.permanent_size + memory.transient_size;

    memory.base = os_virtual_alloc(memory.size);
    memory.permanent_base = memory.base;
    memory.transient_base = (u8*)memory.base + memory.permanent_size;
}

static Window
win32_window_create(const wchar* window_name, s32 width, s32 height){
    Window result = {0};

    WNDCLASSW window_class = {
        .style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC,
        .lpfnWndProc = win_message_handler_callback,
        .hInstance = GetModuleHandle(0),
        .hIcon = LoadIcon(0, IDI_APPLICATION),
        .hCursor = LoadCursor(0, IDC_ARROW),
        .lpszClassName = L"window class",
    };

    if(!RegisterClassW(&window_class)){
        return(result);
    }

    result.width = width;
    result.height = height;

    DWORD style = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, style, FALSE);
    s32 adjusted_w = rect.right - rect.left;
    s32 adjusted_h = rect.bottom - rect.top;

    result.handle = CreateWindowW(L"window class", window_name, style, CW_USEDEFAULT, CW_USEDEFAULT, adjusted_w, adjusted_h, 0, 0, GetModuleHandle(0), 0);
    if(!IsWindow(result.handle)){
        // todo: log error
    }
    assert(IsWindow(result.handle));

    return(result);
}

static void
show_cursor(bool show){
    if(show){
        while(ShowCursor(1) < 0);
    }
    else{
        while(ShowCursor(0) >= 0);
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param){
    begin_timed_function();

    if(ImGui_ImplWin32_WndProcHandler(hwnd, message, w_param, l_param)){
        return(true);
    }

    LRESULT result = 0;

    switch(message){
        case WM_CLOSE:
        case WM_QUIT:
        case WM_DESTROY:{
            Event event;
            event.type = QUIT;
            events_add(&events, event);
        } break;

        case WM_MOUSEMOVE:{
            Event event;
            event.type = MOUSE; // TODO: maybe have this be a KEYBOARD event
            event.mouse_pos.x = (s32)(s16)(l_param & 0xFFFF);
            event.mouse_pos.y = (s32)(s16)(l_param >> 16);

            // todo: dx/y is probably wrong, not working as expected
            s32 dx = event.mouse_pos.x - (SCREEN_WIDTH/2);
            s32 dy = event.mouse_pos.y - (SCREEN_HEIGHT/2);
            event.mouse_dx = (f32)dx / (f32)(SCREEN_WIDTH/2);
            event.mouse_dy = (f32)dy / (f32)(SCREEN_HEIGHT/2);

            events_add(&events, event);
        } break;

        case WM_MOUSEWHEEL:{
            Event event;
            event.type = KEYBOARD;
            event.mouse_wheel_dir = GET_WHEEL_DELTA_WPARAM(w_param) > 0? 1 : -1;
            events_add(&events, event);
        } break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:{
            Event event;
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_LEFT;

            bool pressed = false;
            if(message == WM_LBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            events_add(&events, event);
        } break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:{
            Event event;
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_RIGHT;

            bool pressed = false;
            if(message == WM_RBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            events_add(&events, event);
        } break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:{
            Event event;
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_MIDDLE;

            bool pressed = false;
            if(message == WM_MBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            events_add(&events, event);
        } break;

        case WM_CHAR:{
            u64 keycode = w_param;

            if(keycode > 31){
                Event event;
                event.type = TEXT_INPUT;
                event.keycode = keycode;
                events_add(&events, event);
            }

        } break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:{
            Event event;
            event.type = KEYBOARD;
            event.keycode = w_param;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed = 1;
            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;
        case WM_SYSKEYUP:
        case WM_KEYUP:{
            Event event;
            event.type = KEYBOARD;
            event.keycode = w_param;

            event.key_pressed = 0;
            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = false; }
            if(w_param == VK_SHIFT)   { shift_pressed = false; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = false; }
        } break;
        default:{
            result = DefWindowProcW(hwnd, message, w_param, l_param);
        } break;
    }
    return(result);
}

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type){
    begin_profiler();


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    window = win32_window_create(L"Roids", SCREEN_WIDTH, SCREEN_HEIGHT);
    if(!window.handle){
        print("Error: Could not create window\n");
        return(0);
    }

    init_paths(global_arena);
    random_seed(0, 1);

    d3d_init(window.handle, window.width, window.height);
#if DEBUG
    d3d_init_debug_stuff();
#endif

    ImGui_ImplWin32_Init(window.handle);
    ImGui_ImplDX11_Init(d3d_device, d3d_context);

    memory_init();
    init_clock(&clock);
    HRESULT hr = audio_init(2, 48000, 32);
    assert_hr(hr);

    events_init(&events);

    f64 FPS = 0;
    f64 MSPF = 0;
    u64 total_frames = 0;
    u64 frame_count = 0;
	u32 simulations = 0;
    f64 time_elapsed = 0;
    f64 accumulator = 0.0;

    clock.dt =  1.0/240.0;
    u64 last_ticks = clock.get_os_timer();
    u64 frame_tick_start = clock.get_os_timer();

    assert(sizeof(PermanentMemory) < memory.permanent_size);
    assert(sizeof(TransientMemory) < memory.transient_size);
    pm = (PermanentMemory*)memory.permanent_base;
    tm = (TransientMemory*)memory.transient_base;

    if(!memory.initialized){
        // consider: maybe move this memory stuff to memory_init()
        init_arena(&pm->arena, (u8*)memory.permanent_base + sizeof(PermanentMemory), memory.permanent_size - sizeof(PermanentMemory));
        init_arena(&tm->arena, (u8*)memory.transient_base + sizeof(TransientMemory), memory.transient_size - sizeof(TransientMemory));

        tm->render_command_arena = push_arena(&tm->arena, MB(100));
        tm->frame_arena = push_arena(&tm->arena, MB(100));
        tm->asset_arena = push_arena(&tm->arena, MB(100));

        show_cursor(true);

        init_camera();
        init_console(&pm->arena, FontAsset_Arial);
        init_console_commands();

        pm->current_font = FontAsset_Arial;
        pm->font = &tm->assets.fonts[FontAsset_Arial];

        // setup free entities array in reverse order
        entities_clear();

        //u8* a = push_array(&pm->arena, u8, 128);
        for(u32 i=0; i<SUB_CATEGORIES_MAX; ++i){
            String8* str = pm->sub_categories + i;
            str->str = push_array(&pm->arena, u8, 128);
            str->size = 128;
        }

        //for(u32 i=0; i<1024; ++i){
        //    Row* row = rows + i;
        //    row->text.str = push_array(&pm->arena, u8, 128);
        //    if(i == 0){
        //        row->text.str[0] = '1';
        //    }
        //    if(i == 1){
        //        row->text.str[0] = '2';
        //    }
        //    if(i == 2){
        //        row->text.str[0] = '3';
        //    }
        //    if(i == 3){
        //        row->text.str[0] = '4';
        //    }
        //    row->text.size = 128;
        //}

        //categories.push_back({"1", "Home", "", "", "0"});

        //rows.push_back({"1", "Rent", "1000", "100", "0"});
        //rows.push_back({"2", "Cat", "500", "0", "0"});
        //rows.push_back({"3", "Groceries", "300", "400", "0"});
        //pm->sub_categories_count = 3;
        //for(u32 i=4; i<pm->sub_categories_count; ++i){
        //    rows.push_back({std::to_string(i), "", "", "", ""});
        //}

        memory.initialized = true;
    }


    f32 s = 0;
    should_quit = false;
    while(!should_quit){
        begin_timed_scope("while(!should_quit)");
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        MSG message;
        while(PeekMessageW(&message, window.handle, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        while(!events_empty(&events)){
            Event event = events_next(&events);

            if(event.type == KEYBOARD){
                if(event.keycode == KeyCode_ESCAPE){
                    should_quit = true;
                }
            }
            if(event.type == QUIT){
                should_quit = true;
            }
        }

        u64 now_ticks = clock.get_os_timer();
        f64 frame_time = clock.get_seconds_elapsed(now_ticks, last_ticks);
        MSPF = 1000/1000/((f64)clock.frequency / (f64)(now_ticks - last_ticks));
        last_ticks = now_ticks;

        // command arena
        draw_clear_color(tm->render_command_arena, BACKGROUND_COLOR);
        // todo: also use flags here

        Font* font = &tm->assets.fonts[pm->current_font];

        f64 second_elapsed = clock.get_seconds_elapsed(clock.get_os_timer(), frame_tick_start);
        if(second_elapsed > 1){
            FPS = ((f64)frame_count / second_elapsed);
            frame_tick_start = clock.get_os_timer();
            frame_count = 0;
        }

        //print("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f - simulations: %i\n", FPS, MSPF, clock.dt, accumulator, frame_time, second_elapsed, simulations);
        String8 fps = str8_formatted(tm->frame_arena, "FPS: %.2f", FPS);

        // draw everything
        draw_commands(tm->render_command_arena);




        ImGui::Begin("Draggable Rows Example");

        if (ImGui::Button("+")) {
            pm->categories_count++;
            categories.push_back({std::to_string(pm->categories_count), "", 0, 0, ""});
        }

        ImGui::SameLine();
        ImGui::SetCursorPosX(column2_start);
        ImGui::Text("Category");
        ImGui::SameLine();
        ImGui::SetCursorPosX(column3_start);
        ImGui::Text("Planned");
        ImGui::SameLine();
        ImGui::SetCursorPosX(column4_start);
        ImGui::Text("Actual");
        ImGui::SameLine();
        ImGui::SetCursorPosX(column5_start);
        ImGui::Text("Diff");

        ImGui::Separator();
        for (s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
            Category& category = categories[c_idx];

            ImGui::SetCursorPosX(column1_start);
            ImGui::PushID(c_idx);
            if(category.draw_rows){
                if(ImGui::Button("V")){
                    category.draw_rows = false;
                }
            }
            else{
                if(ImGui::Button(">")){
                    if(category.row_count){
                        category.draw_rows = true;
                    }
                }
            }
            ImGui::PopID();
            // Handle drag and drop
            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("DRAG_ROW", &c_idx, sizeof(int));
                ImGui::Text("%s", category.input);
                ImGui::EndDragDropSource();
            }
            if(ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_ROW")) {
                    int* payload_data = (int*)payload->Data;
                    int from_index = *payload_data;
                    if(from_index != c_idx) {
                        std::swap(categories[from_index].row_number, categories[c_idx].row_number);
                        std::swap(categories[from_index], categories[c_idx]);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();

            std::string input_id;
            ImGui::SetCursorPosX(column2_start);
            ImGui::PushItemWidth(column2_width);
            input_id = "##input" + std::to_string(c_idx);
            ImGui::InputText(input_id.c_str(), category.input, 128);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::SetCursorPosX(column3_start);
            std::string planned = std::to_string(category.planned);
            ImGui::Text(planned.c_str());

            ImGui::SameLine();
            ImGui::SetCursorPosX(column4_start);
            std::string actual = std::to_string(category.actual);
            ImGui::Text(actual.c_str());

            ImGui::SameLine();
            ImGui::SetCursorPosX(column5_start);
            category.diff = std::to_string(category.planned - category.actual);
            ImGui::Text(category.diff.c_str());

            ImGui::SameLine();
            ImGui::SetCursorPosX(column6_start);
            ImGui::PushID(c_idx);
            if (ImGui::Button("+")) {
                category.row_count++;
                category.rows.push_back({std::to_string(category.row_count), "", "", "", ""});
                category.draw_rows = true;
            }
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::SetCursorPosX(column7_start);
            ImGui::PushID(c_idx);
            if(ImGui::Button("x")){
                categories.erase(categories.begin() + c_idx);
                --pm->categories_count;
                category.row_count = 0;
                for (int idx = 0; idx < pm->categories_count; ++idx){
                    categories[idx].row_number = std::to_string(idx + 1);
                }
            }
            ImGui::PopID();

            category.planned = 0;
            category.actual = 0;
            for(s32 r_idx = 0; r_idx < category.row_count; ++r_idx){
                Row& row = category.rows[r_idx];

                category.planned += std::atoi(row.planned);
                category.actual += std::atoi(row.actual);
                std::string s_id = std::to_string(r_idx + 1) + std::to_string(c_idx + 1);
                s32 uid = std::atoi(s_id.c_str());

                if(category.draw_rows){
                    ImGui::SetCursorPosX(row_column1_start);
                    ImGui::PushID(uid);
                    ImGui::Button("-");
                    ImGui::PopID();

                    // Handle drag and drop
                    if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("DRAG_ROW", &r_idx, sizeof(int));
                        ImGui::Text("%s", row.input);
                        ImGui::EndDragDropSource();
                    }
                    if(ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_ROW")) {
                            int* payload_data = (int*)payload->Data;
                            int from_index = *payload_data;
                            if(from_index != r_idx) {
                                std::swap(category.rows[from_index].row_number, category.rows[r_idx].row_number);
                                std::swap(category.rows[from_index], category.rows[r_idx]);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::SameLine();

                    std::string input_id;
                    ImGui::SetCursorPosX(row_column2_start);
                    ImGui::PushItemWidth(row_column2_width);
                    input_id = "##input" + std::to_string(r_idx) + std::to_string(c_idx);
                    ImGui::InputText(input_id.c_str(), row.input, 128);
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(row_column3_start);
                    ImGui::PushItemWidth(row_column3_width);
                    input_id = "##planned" + std::to_string(r_idx) + std::to_string(c_idx);
                    ImGui::InputText(input_id.c_str(), row.planned, 128, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter, InputTextCallback);
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(row_column4_start);
                    ImGui::PushItemWidth(row_column4_width);
                    input_id = "##actual" + std::to_string(r_idx) + std::to_string(c_idx);
                    ImGui::InputText(input_id.c_str(), row.actual, 128, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter, InputTextCallback);
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(row_column5_start);
                    int planned = std::atoi(row.planned);
                    int actual = std::atoi(row.actual);
                    row.diff = std::to_string(planned - actual);
                    ImGui::Text(row.diff.c_str());

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(row_column6_start);
                    ImGui::PushID(uid);
                    if(ImGui::Button("x")){
                        print("OK\n");
                        category.rows.erase(category.rows.begin() + r_idx);
                        --category.row_count;
                        for (int idx = 0; idx < category.row_count; ++idx){
                            category.rows[idx].row_number = std::to_string(idx + 1);
                        }
                    }
                    ImGui::PopID();
                }
            }
            ImGui::Separator();
        }

        ImGui::End();



        //ImGui::ShowDemoWindow(); // Show demo window! :)

        {
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        console_draw();
        d3d_swapchain->Present(1, 0);

        arena_free(tm->frame_arena);
        arena_free(tm->render_command_arena);
		simulations = 0;
        total_frames++;
        //end_profiler();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    d3d_release();
    end_profiler();
    audio_release();

    return(0);
}

