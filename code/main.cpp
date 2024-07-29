#include "main.hpp"

#include "bitmap.cpp"
#include "d3d11_init.cpp"
#include "d3d11_render.cpp"
#include "font.cpp"

static void
load_assets(Arena* arena){

    ScratchArena scratch = begin_scratch();

    Bitmap bm;
    bm = load_bitmap(scratch.arena, str8_literal("sprites/ship2.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Ship].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/circle.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Bullet].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/asteroid.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Asteroid].view, &bm);

    bm = load_bitmap(scratch.arena, str8_literal("sprites/flame1.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Flame1].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/flame2.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Flame2].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/flame3.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Flame3].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/flame4.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Flame4].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/flame5.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Flame5].view, &bm);

    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion1.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion1].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion2.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion2].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion3.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion3].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion4.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion4].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion5.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion5].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion6.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion6].view, &bm);

    end_scratch(scratch);
}

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

            // todo: dx/dy is probably wrong, not working as expected
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

    window = win32_window_create(L"Budgeteer", SCREEN_WIDTH, SCREEN_HEIGHT);
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
        tm->options_arena = push_arena(&tm->arena, MB(100));

        show_cursor(true);

        // BUDGET STUFFENINGS
        pm->category_pool    = push_pool(&pm->arena, sizeof(Category), 128);
        pm->row_pool         = push_pool(&pm->arena, sizeof(Row), 1024);
        pm->transaction_pool = push_pool(&pm->arena, sizeof(Transaction), 4096);
        pm->data_arena       = push_arena(&pm->arena, MB(1));

        pool_free_all(pm->category_pool);
        pool_free_all(pm->row_pool);
        pool_free_all(pm->transaction_pool);

        pm->categories = (Category*)pool_next(pm->category_pool);
        dll_clear(pm->categories);

        for(s32 i=0; i < Month_Count; ++i){
            TransactionMonth* t_month = pm->transaction_months + i;
            t_month->transactions = (Transaction*)pool_next(pm->transaction_pool);
            dll_clear(t_month->transactions);
        }
        pm->t_month = pm->transaction_months + pm->month_idx;

        // todo: look at this
        pm->category_options = push_array(tm->options_arena, String8, 1024);
        for(s32 i=0; i < 128; ++i){
            String8* option = pm->category_options + i;
            option->str = push_array(tm->options_arena, u8, 128);
            pm->options_count++;
        }
        *pm->category_options = str8(" \0", 2);

        deserialize_data();


        memory.initialized = true;
    }


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

        f64 second_elapsed = clock.get_seconds_elapsed(clock.get_os_timer(), frame_tick_start);
        if(second_elapsed > 1){
            FPS = ((f64)frame_count / second_elapsed);
            frame_tick_start = clock.get_os_timer();
            frame_count = 0;
        }

        //prs32("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f - simulations: %i\n", FPS, MSPF, clock.dt, accumulator, frame_time, second_elapsed, simulations);
        String8 fps = str8_formatted(tm->frame_arena, "FPS: %.2f", FPS);




        ImGui::Begin("Budgeteer");
        ImGui::Columns(2);

        ImGui::SeparatorText("Totals");
        ImGui::Text("Budget:");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start - input_padding);
        ImGui::PushItemWidth(75);
        if(budget[0] == 0){
            budget[0] = '0';
            budget[1] = '\0';
        }
        ImGui::InputText("##Budget", budget, 128, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll);
        ImGui::Text("Planned: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        ImGui::Text("%i", total_planned);
        ImGui::Text("Actual: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        ImGui::Text("%i", total_actual);
        ImGui::Text("Diff: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        if(total_diff < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("%i", total_diff);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("%i", total_diff);
        }

        ImGui::Text("Goal: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        if(total_goal < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("%i", total_goal);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("%i", total_goal);
        }

        ImGui::Text("Saved: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        if(total_saved < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("%i", total_saved);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("%i", total_saved);
        }

        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::SeparatorText("Plan");

        ImGui::SetCursorPosX(row_count_column_start + input_padding);
        ImGui::Text("#");
        ImGui::SameLine();
        ImGui::SetCursorPosX(category_column_start);
        ImGui::Text("Category");
        ImGui::SameLine();
        ImGui::SetCursorPosX(planned_column_start);
        ImGui::Text("Planned");
        ImGui::SameLine();
        ImGui::SetCursorPosX(actual_column_start);
        ImGui::Text("Actual");
        ImGui::SameLine();
        ImGui::SetCursorPosX(diff_column_start);
        ImGui::Text("Diff");
        ImGui::SameLine();
        ImGui::SetCursorPosX(plus_column_start);
        if(ImGui::Button("+##add_category_button")){
            Category* category = (Category*)pool_next(pm->category_pool);
            dll_push_back(pm->categories, category);
            category->rows = (Row*)pool_next(pm->row_pool);
            dll_clear(category->rows);

            pm->categories_count++;
        }

        custom_separator();

        //note: popluate with 0's
        Category* category = pm->categories;
        for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
            category = category->next;

            Row* row = category->rows;
            for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                row = row->next;

                if(row->planned[0] == 0){
                    row->planned[0] = '0';
                    row->planned[1] = '\0';
                }
            }
        }

        s32 planned = 0;
        s32 actual = 0;
        s32 diff = 0;
        //#####PLAN######
        category = pm->categories;
        for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
            category = category->next;

            ImGui::SetCursorPosX(collapse_column_start);
            ImGui::PushID(c_idx);
            if(category->draw_rows){
                if(ImGui::Button("V")){
                    category->draw_rows = false;
                }
            }
            else{
                if(ImGui::Button(">")){
                    if(category->row_count){
                        category->draw_rows = true;
                    }
                }
            }
            ImGui::PopID();
            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)){
                ImGui::SetDragDropPayload("DRAG_ROW", &c_idx, sizeof(s32));
                ImGui::Text("%s", category->name);
                ImGui::EndDragDropSource();
            }
            if(ImGui::BeginDragDropTarget()){
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_ROW")){
                    s32* payload_data = (s32*)payload->Data;
                    s32 from_idx = *payload_data;
                    if(from_idx != c_idx){
                        Category* c = pm->categories;
                        for(s32 i=0; i <= from_idx; ++i){
                            c = c->next;
                        }
                        dll_swap(c, category, Category);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(row_count_column_start + input_padding);
            if(category->draw_rows){
                ImGui::Text("-");
            }
            else{
                ImGui::Text("%i", category->row_count);
            }

            ImGui::SameLine();

            std::string input_id;
            ImGui::SetCursorPosX(category_column_start);
            ImGui::PushItemWidth(category_column_width);
            input_id = "##sub_category" + std::to_string(c_idx);
            ImGui::InputText(input_id.c_str(), category->name, 128, ImGuiInputTextFlags_AutoSelectAll);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::SetCursorPosX(planned_column_start + input_padding);
            std::string planned_str = std::to_string(category->planned);
            ImGui::Text(planned_str.c_str());

            ImGui::SameLine();
            ImGui::SetCursorPosX(actual_column_start + input_padding);
            std::string actual_str = std::to_string(category->actual);
            ImGui::Text(actual_str.c_str());

            ImGui::SameLine();
            ImGui::SetCursorPosX(diff_column_start);
            category->diff = category->planned - category->actual;
            if(category->diff < 0){
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGui::Text(std::to_string(category->diff).c_str());
                ImGui::PopStyleColor();
            }
            else{
                ImGui::Text(std::to_string(category->diff).c_str());
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(plus_column_start);
            ImGui::PushID(c_idx);
            if(ImGui::Button("+##add_row_button")){
                Row* r = (Row*)pool_next(pm->row_pool);
                dll_push_back(category->rows, r);

                category->draw_rows = true;
                category->row_count++;
                pm->total_rows_count++;
            }
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::SetCursorPosX(x_column_start);
            ImGui::PushID(c_idx);
            if(ImGui::Button("x##remove_category")){
                pm->total_rows_count -= category->row_count;
                --pm->categories_count;

                dll_remove(category);
                pool_free(pm->category_pool, category);
            }
            ImGui::PopID();


            category->planned = 0;
            category->actual = 0;
            Row* row = category->rows;
            for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                row = row->next;

                category->planned += std::atoi(row->planned);
                category->actual += row->actual;
                std::string s_id = std::to_string(r_idx + 1) + std::to_string(c_idx + 1);
                s32 uid = std::atoi(s_id.c_str());

                if(category->draw_rows){
                    ImGui::SetCursorPosX(row_count_column_start);
                    std::string num_button = std::to_string(r_idx + 1);
                    ImGui::PushID(uid);
                    ImGui::Button(num_button.c_str());
                    //ImGui::Button("-");
                    ImGui::PopID();

                    if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)){
                        ImGui::SetDragDropPayload("DRAG_ROW", &r_idx, sizeof(s32));
                        ImGui::Text("%s", row->name);
                        ImGui::EndDragDropSource();
                    }
                    if(ImGui::BeginDragDropTarget()){
                        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_ROW")){
                            s32* payload_data = (s32*)payload->Data;
                            s32 from_idx = *payload_data;
                            if(from_idx != r_idx){
                                Row* r = category->rows;
                                for(s32 i=0; i <= from_idx; ++i){
                                    r = r->next;
                                }
                                dll_swap(r, row, Row);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::SameLine();

                    std::string input_id;
                    ImGui::SetCursorPosX(category_column_start);
                    ImGui::PushItemWidth(category_column_width);
                    input_id = "##category" + std::to_string(r_idx) + std::to_string(c_idx);
                    ImGui::InputText(input_id.c_str(), row->name, 128, ImGuiInputTextFlags_AutoSelectAll);
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(planned_column_start);
                    ImGui::PushItemWidth(planned_column_width);
                    input_id = "##planned" + std::to_string(r_idx) + std::to_string(c_idx);
                    ImGui::InputText(input_id.c_str(), row->planned, 128, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll);

                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(actual_column_start + input_padding);
                    ImGui::PushItemWidth(actual_column_width);
                    ImGui::Text(std::to_string(row->actual).c_str());
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(diff_column_start);
                    s32 planned = std::atoi(row->planned);
                    s32 actual = row->actual;
                    if((planned - actual) < 0){
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                        row->diff = planned - actual;
                        ImGui::Text(std::to_string(row->diff).c_str());
                        ImGui::PopStyleColor();
                    }
                    else{
                        row->diff = planned - actual;
                        ImGui::Text(std::to_string(row->diff).c_str());
                    }

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(x_column_start);
                    ImGui::PushID(uid);
                    if(ImGui::Button("x##remove_row")){
                        --pm->total_rows_count;
                        --category->row_count;

                        dll_remove(row);
                        pool_free(pm->row_pool, row);
                    }
                    ImGui::PopID();
                }
            }
            planned += category->planned;
            actual += category->actual;
            diff += category->diff;
            custom_separator();

        }
        total_planned = planned;
        total_actual = actual;
        total_diff = diff;
        total_saved = atoi(budget) - total_actual;
        total_goal = atoi(budget) - total_planned;

        // note: collect options
        u32 count = 1;
        pm->options_count = 1;
        category = pm->categories;
        for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
            category = category->next;

            Row* row = category->rows;
            for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                row = row->next;

                String8* opt = pm->category_options + count;
                if(row->name[0] != '\0'){
                    if(!char_only_spaces(row->name)){ // don't include rows that are named only spaces
                        u32 length = char_length(row->name);
                        *opt = str8(row->name, length + 1);
                    }
                }
                else{
                    *opt = str8("", 0);
                }
                pm->options_count++;
                count++;
            }
        }

        //########COLUMN2######################################################################
        //##################
        //###TRANSACTIONS###
        //##################

        ImGui::NextColumn();
        ImGui::SeparatorText("Transactions");

        if(ImGui::BeginTabBar("##Month", ImGuiTabBarFlags_None)){
            if(ImGui::BeginTabItem("January")){
                pm->month_idx = Month_Jan;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("February")){
                pm->month_idx = Month_Feb;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("March")){
                pm->month_idx = Month_Mar;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("April")){
                pm->month_idx = Month_Apr;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("May")){
                pm->month_idx = Month_May;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("June")){
                pm->month_idx = Month_Jun;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("July")){
                pm->month_idx = Month_Jul;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("August")){
                pm->month_idx = Month_Aug;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("September")){
                pm->month_idx = Month_Sep;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("October")){
                pm->month_idx = Month_Oct;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("November")){
                pm->month_idx = Month_Nov;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("December")){
                pm->month_idx = Month_Dec;
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + date_column_start - 30 + input_padding);
        ImGui::Text("#");

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + date_column_start);
        ImGui::Text("Date");

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + amount_column_start);
        ImGui::Text("Amount");

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + description_column_start);
        ImGui::Text("Description");

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + category_select_column_start);
        ImGui::Text("Category");

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + plus_expense_column_start);
        if(ImGui::Button("+##add_transaction_button")){
            Transaction* trans = (Transaction*)pool_next(pm->transaction_pool);
            dll_push_back(pm->t_month->transactions, trans);

            if(pm->t_month->count == 0){
                String8 date = str8("01/01/2024\0", 11);
                memory_copy((void*)trans->date, (void*)date.str, date.size);
            }
            else{
                Transaction* last = trans->prev;
                memory_copy((void*)trans->date, (void*)last->date, (u32)11);
            }
            memory_copy((void*)trans->name, (void*)pm->category_options->str, pm->category_options->size);

            pm->t_month->count++;
        }
        custom_separator();

        //note: popluate amount's with 0's
        Transaction* trans = pm->t_month->transactions;
        for(s32 t_idx = 0; t_idx < pm->t_month->count; ++t_idx){
            trans = trans->next;

            if(trans->amount[0] == 0){
                trans->amount[0] = '0';
                trans->amount[1] = '\0';
            }
        }

        // note: render transactions
        pm->t_month = pm->transaction_months + pm->month_idx;
        trans = pm->t_month->transactions;
        for(s32 i=0; i < pm->t_month->count; ++i){
            trans = trans->next;

            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + date_column_start - 30);
            std::string num_button = std::to_string(i + 1);
            ImGui::PushID(i);
            ImGui::Button(num_button.c_str());
            ImGui::PopID();
            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)){
                ImGui::SetDragDropPayload("DRAG_ROW", &i, sizeof(s32));
                ImGui::Text("%i", i);
                ImGui::EndDragDropSource();
            }
            if(ImGui::BeginDragDropTarget()){
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_ROW")){
                    s32* payload_data = (s32*)payload->Data;
                    s32 from_idx = *payload_data;
                    if(from_idx != i){
                        Transaction* t = pm->t_month->transactions;
                        for(s32 i=0; i <= from_idx; ++i){
                            t = t->next;
                        }
                        dll_swap(t, trans, Transaction);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            std::string unique_id;
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + date_column_start);
            ImGui::PushItemWidth(date_column_width);
            unique_id = "##date" + std::to_string(i);
            ImGui::InputText(unique_id.c_str(), trans->date, 128, ImGuiInputTextFlags_CharsDecimal);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + amount_column_start);
            ImGui::PushItemWidth(amount_column_width);
            unique_id = "##amount" + std::to_string(i);
            ImGui::InputText(unique_id.c_str(), trans->amount, 128, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + description_column_start);
            ImGui::PushItemWidth(description_column_width);
            unique_id = "##description" + std::to_string(i);
            ImGui::InputText(unique_id.c_str(), trans->description, 128);

            ImGui::PopItemWidth();

            // note: SELECTION BOX
            ImGui::SameLine();
            ImGui::PushItemWidth(category_select_column_width);

            // note: make selection box not transparent
            ImVec4 popup_bg_color = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
            popup_bg_color.w = 1.0f; // Set alpha to 1.0 (fully opaque)
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetColorU32(popup_bg_color));

            ImVec4 frame_bg_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
            bool found = false;
            char space[] = " ";
            if(!char_cmp(trans->name, space)){
                category = pm->categories;
                for(s32 c_idx = 0; c_idx < pm->categories_count && !found; ++c_idx){
                    category = category->next;

                    Row* row = category->rows;
                    for(s32 r_idx = 0; r_idx < category->row_count && !found; ++r_idx){
                        row = row->next;
                        u32 l1 = char_length(trans->name);
                        u32 l2 = char_length(row->name);
                        if(l1 == l2){
                            if(char_cmp(trans->name, row->name)){
                                found = true;
                            }
                        }
                    }
                }
                if(!found){
                    frame_bg_color.x = 1;
                    frame_bg_color.y = 0;
                    frame_bg_color.z = 0;
                }
            }
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(frame_bg_color));


            unique_id = "##category_select" + std::to_string(i);
            if(ImGui::BeginCombo(unique_id.c_str(), trans->name)){
                for (int n = 0; n < pm->options_count; n++){
                    String8 itr_option = pm->category_options[n];
                    String8 trans_option = str8(trans->name, char_length(trans->name));

                    if(itr_option.size == 0){
                        continue;
                    }

                    const bool is_selected = str8_compare(itr_option, trans_option);
                    if(ImGui::Selectable((char*)itr_option.str, is_selected)){
                        memory_copy((void*)trans->name, (void*)itr_option.str, itr_option.size + 1);
                    }

                    if(is_selected){
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopStyleColor(2);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + plus_expense_column_start);
            std::string deleteButtonLabel = "x##remove_transaction" + std::to_string(i);
            if(ImGui::Button(deleteButtonLabel.c_str())){
                pm->t_month->count--;

                dll_remove(trans);
                pool_free(pm->transaction_pool, trans);
            }
        }

        // note: collect transactions amounts
        category = pm->categories;
        for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
            category = category->next;

            Row* row = category->rows;
            for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                row = row->next;
                row->actual = 0;

                trans = pm->t_month->transactions;
                for(s32 t_idx = 0; t_idx < pm->t_month->count; ++t_idx){
                    trans = trans->next;

                    u32 l1 = char_length(trans->name);
                    u32 l2 = char_length(row->name);
                    if(l1 == l2){
                        if(char_cmp(row->name, trans->name)){
                            s32 amount = atoi(trans->amount);
                            row->actual += amount;
                        }
                    }
                }

            }
        }

        ImGui::End();

        //ImGui::ShowDemoWindow(); // Show demo window! :)

        // draw everything
        draw_commands(tm->render_command_arena);

        {
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        d3d_swapchain->Present(1, 0);

        arena_free(tm->frame_arena);
        arena_free(tm->render_command_arena);
		simulations = 0;
        total_frames++;
        //end_profiler();
    }

    if(should_quit){
        serialize_data();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    d3d_release();
    end_profiler();

    return(0);
}

