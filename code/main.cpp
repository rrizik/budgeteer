#include "main.hpp"


static void
init_paths(Arena* arena){
    build_path = os_application_path(global_arena);
    saves_path = str8_path_append(global_arena, build_path, str8_literal("saves"));
}

static void
memory_init(){
    memory.permanent_size = MB(500);
    memory.transient_size = GB(1);
    memory.size = memory.permanent_size + memory.transient_size;

    memory.base = os_alloc(memory.size);
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

        tm->frame_arena = push_arena(&tm->arena, MB(100));
        tm->options_arena = push_arena(&tm->arena, MB(100));

        show_cursor(true);

        // create pools
        pm->category_pool    = push_pool(&pm->arena, sizeof(Category), 128);
        pm->row_pool         = push_pool(&pm->arena, sizeof(Row), 1024);
        pm->transaction_pool = push_pool(&pm->arena, sizeof(Transaction), 4096);
        pm->data_arena       = push_arena(&pm->arena, MB(1));

        // setup free list from pools
        pool_free_all(pm->category_pool);
        pool_free_all(pm->row_pool);
        pool_free_all(pm->transaction_pool);

        // setup sentinel node or categories
        pm->categories = (Category*)pool_next(pm->category_pool);
        dll_clear(pm->categories);

        // setup sentinel node for month transactions
        for(s32 i=0; i < Month_Count; ++i){
            MonthInfo* month = pm->months + i;
            month->transactions = (Transaction*)pool_next(pm->transaction_pool);
            dll_clear(month->transactions);
        }
        pm->month = pm->months + pm->month_tab_idx;

        // give selection list memory
        pm->selection_list = push_array(tm->options_arena, String8, 1024);
        for(s32 i=0; i < 128; ++i){
            String8* option = pm->selection_list + i;
            option->str = push_array(tm->options_arena, u8, 128);
        }
        *pm->selection_list = str8(" \0", 2);
        pm->default_path = os_application_path(&pm->arena);

        for(u32 i=0; i < 32; ++i){
            pm->date_names[i].data = push_array(global_arena, u8, 128);
            pm->amount_names[i].data = push_array(global_arena, u8, 128);
            pm->desc_names[i].data = push_array(global_arena, u8, 128);
        }

        pm->budget.data = push_array(global_arena, u8, 128);

        load_config();
        deserialize_data();
        pm->tab_flags[pm->month_tab_idx] = ImGuiTabItemFlags_SetSelected;

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


        f64 second_elapsed = clock.get_seconds_elapsed(clock.get_os_timer(), frame_tick_start);
        if(second_elapsed > 1){
            FPS = ((f64)frame_count / second_elapsed);
            frame_tick_start = clock.get_os_timer();
            frame_count = 0;
        }

        //prs32("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f - simulations: %i\n", FPS, MSPF, clock.dt, accumulator, frame_time, second_elapsed, simulations);
        String8 fps = str8_formatted(tm->frame_arena, "FPS: %.2f", FPS);




        // set window (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), and fix it
        //ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        //ImVec2 window_size = ImVec2(SCREEN_WIDTH, SCREEN_HEIGHT);
        //ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

        ScratchArena scratch = begin_scratch();
        ImGui::Begin("Budgeteer");
        ImGui::Columns(2);

        ImGui::SeparatorText("Totals");
        ImGui::Text("Budget:");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start - input_padding);
        ImGui::PushItemWidth(75);
        //if(pm->budget[0] == 0){
        //    pm->budget[0] = '0';
        //    pm->budget[1] = '\0';
        //}
        ImGui::InputText("##Budget", (char*)pm->budget.str, 128, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll);
        ImGui::Text("Planned: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        ImGui::Text("%.2f", pm->total_planned);
        ImGui::Text("Actual: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        ImGui::Text("%.2f", pm->total_actual);
        ImGui::Text("Diff: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        if(pm->total_diff < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("%.2f", pm->total_diff);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("%.2f", pm->total_diff);
        }

        custom_separator();
        ImGui::Text("Goal: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        if(pm->total_goal < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("%.2f", pm->total_goal);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("%.2f", pm->total_goal);
        }

        ImGui::Text("Saved: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        if(pm->total_saved < pm->total_goal){
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("%.2f", pm->total_saved);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("%.2f", pm->total_saved);
        }
        custom_separator();

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

        f32 planned = 0;
        f32 actual = 0;
        f32 diff = 0;
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


            ImGui::SetCursorPosX(category_column_start);
            ImGui::PushItemWidth(category_column_width);
            String8 unique_id = str8_formatted(scratch.arena, "##category%i", c_idx);
            ImGui::InputText((char*)unique_id.data, category->name, 128, ImGuiInputTextFlags_AutoSelectAll);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::SetCursorPosX(planned_column_start + input_padding);
            String8 planned_str = str8_formatted(scratch.arena, "%.2f", category->planned);
            ImGui::Text((char*)planned_str.data);

            ImGui::SameLine();
            ImGui::SetCursorPosX(actual_column_start + input_padding);
            String8 actual_str = str8_formatted(scratch.arena, "%.2f", category->actual);
            ImGui::Text((char*)actual_str.data);

            ImGui::SameLine();
            ImGui::SetCursorPosX(diff_column_start);
            category->diff = category->planned - category->actual;
            String8 category_diff = str8_formatted(scratch.arena, "%.2f", category->diff);
            if(category->diff < 0){
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
            ImGui::Text((char*)category_diff.data);
            if(category->diff < 0){
                ImGui::PopStyleColor();
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

                category->planned += atof(row->planned);
                category->actual += row->actual;
                String8 s_id = str8_formatted(scratch.arena, "%i%i", r_idx + 1, c_idx + 1);
                s32 uid = atoi((char*)s_id.data);

                if(category->draw_rows){
                    ImGui::SetCursorPosX(row_count_column_start);
                    ImGui::PushID(uid);
                    String8 num_button = str8_formatted(scratch.arena, "%i", r_idx + 1);
                    ImGui::Button((char*)num_button.data);
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
                                // todo: why do I do this?
                                for(s32 i=0; i <= from_idx; ++i){
                                    r = r->next;
                                }
                                dll_swap(r, row, Row);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::SameLine();

                    ImGui::SetCursorPosX(category_column_start);
                    ImGui::PushItemWidth(category_column_width);
                    String8 input_id = str8_formatted(scratch.arena, "##sub_category%i%i", r_idx, c_idx);
                    ImGui::InputText((char*)input_id.data, row->name, 128, ImGuiInputTextFlags_AutoSelectAll);
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(planned_column_start);
                    ImGui::PushItemWidth(planned_column_width);
                    String8 planned_id = str8_formatted(scratch.arena, "##planned%i%i", r_idx, c_idx);
                    ImGui::InputText((char*)planned_id.data, row->planned, 128, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll);

                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(actual_column_start + input_padding);
                    ImGui::PushItemWidth(actual_column_width);
                    String8 row_actual = str8_formatted(scratch.arena, "%.2f", row->actual);
                    ImGui::Text((char*)row_actual.data);
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(diff_column_start);
                    f32 planned = atof(row->planned);
                    f32 actual = row->actual;
                    row->diff = planned - actual;
                    if((planned - actual) < 0){
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    }
                    String8 row_diff = str8_formatted(scratch.arena, "%.2f", row->diff);
                    ImGui::Text((char*)row_diff.data);
                    if((planned - actual) < 0){
                        ImGui::PopStyleColor();
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
        pm->total_planned = planned;
        pm->total_actual = actual;
        pm->total_diff = diff;
        pm->total_saved = atof((char*)pm->budget.str) - pm->total_actual;
        pm->total_goal = atof((char*)pm->budget.str) - pm->total_planned;

        // note: collect selection options
        pm->selection_count = 1;
        category = pm->categories;
        for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
            category = category->next;

            Row* row = category->rows;
            for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                row = row->next;

                String8* selection = pm->selection_list + pm->selection_count;
                if(row->name[0] != '\0'){
                    if(!char_only_spaces(row->name)){ // don't include rows that are named only spaces
                        u32 length = char_length(row->name);
                        String8 cat_part = str8_format(tm->frame_arena, "%s: ", category->name);
                        String8 name_part = str8(row->name, length + 1); // + 1 to include 0 terminater
                        String8 full = str8_concatenate(tm->frame_arena, cat_part, name_part);
                        *selection = full;
                    }
                }
                else{
                    *selection = str8("", 0);
                }
                pm->selection_count++;
            }
        }

        //########COLUMN2######################################################################
        //##################
        //###TRANSACTIONS###
        //##################

        ImGui::NextColumn();
        ImGui::SeparatorText("Transactions");

        if(ImGui::BeginTabBar("##Month", ImGuiTabBarFlags_None)){
            ImVec4 active_color = ImGui::GetStyleColorVec4(ImGuiCol_TabActive);
            active_color = ImVec4(0.0f, active_color.y * 0.8f, active_color.z * 0.4f, active_color.w);

            ImVec4 hover_color = ImGui::GetStyleColorVec4(ImGuiCol_TabHovered);
            hover_color = ImVec4(0.0f, hover_color.y * 0.4f, hover_color.z * 0.8f, hover_color.w);

            ImGui::PushStyleColor(ImGuiCol_TabActive, active_color);
            ImGui::PushStyleColor(ImGuiCol_TabHovered, hover_color);

            if(ImGui::BeginTabItem("January", 0, pm->tab_flags[0])){
                pm->month_tab_idx = Month_Jan;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("February", 0, pm->tab_flags[1])){
                pm->month_tab_idx = Month_Feb;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("March", 0, pm->tab_flags[2])){
                pm->month_tab_idx = Month_Mar;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("April", 0, pm->tab_flags[3])){
                pm->month_tab_idx = Month_Apr;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("May", 0, pm->tab_flags[4])){
                pm->month_tab_idx = Month_May;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("June", 0, pm->tab_flags[5])){
                pm->month_tab_idx = Month_Jun;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("July", 0, pm->tab_flags[6])){
                pm->month_tab_idx = Month_Jul;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("August", 0, pm->tab_flags[7])){
                pm->month_tab_idx = Month_Aug;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("September", 0, pm->tab_flags[8])){
                pm->month_tab_idx = Month_Sep;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("October", 0, pm->tab_flags[9])){
                pm->month_tab_idx = Month_Oct;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("November", 0, pm->tab_flags[10])){
                pm->month_tab_idx = Month_Nov;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("December", 0, pm->tab_flags[11])){
                pm->month_tab_idx = Month_Dec;
                ImGui::EndTabItem();
            }
            ImGui::PopStyleColor(2);
            ImGui::EndTabBar();
        }


        ImGui::Spacing();
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
            dll_push_back(pm->month->transactions, trans);

            if(pm->month->transactions_count == 0){
                String8 date = str8("01/01/2024\0", 11);
                memcpy((void*)trans->date, (void*)date.str, date.size);
            }
            else{
                Transaction* last = trans->prev;
                memcpy((void*)trans->date, (void*)last->date, (u32)11);
            }
            memcpy((void*)trans->selection, (void*)pm->selection_list->str, pm->selection_list->size);

            pm->month->transactions_count++;
        }
        ImGui::SameLine();
        if(ImGui::Button("Load CSV##load_csv")){
            char* file = tinyfd_openFileDialog("Open CSV File", (char*)pm->default_path.str, 0, 0, 0, 0);
            if(file){
                String8 file_path = str8(file, char_length(file));

                if(str8_compare_nocase(str8_path_extension(file_path), str8_literal(".csv"))){
                    pm->default_path = str8_path_pop(&pm->arena, file_path, '\\');
                    load_csv(file_path);
                }
            }
        }
        custom_separator();

        //note: popluate amount's with 0's
        pm->month = pm->months + pm->month_tab_idx;
        Transaction* trans = pm->month->transactions;
        for(s32 t_idx = 0; t_idx < pm->month->transactions_count; ++t_idx){
            trans = trans->next;

            if(trans->amount[0] == 0){
                trans->amount[0] = '0';
                trans->amount[1] = '\0';
            }
        }

        // note: render transactions
        trans = pm->month->transactions;
        for(s32 t_idx=0; t_idx < pm->month->transactions_count; ++t_idx){
            trans = trans->next;

            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + date_column_start - 30);
            ImGui::PushID(t_idx);
            String8 num_button = str8_formatted(scratch.arena, "%i", t_idx + 1);
            ImGui::Button((char*)num_button.data);
            ImGui::PopID();
            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)){
                ImGui::SetDragDropPayload("DRAG_ROW", &t_idx, sizeof(s32));
                ImGui::Text("%i", t_idx);
                ImGui::EndDragDropSource();
            }
            if(ImGui::BeginDragDropTarget()){
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_ROW")){
                    s32* payload_data = (s32*)payload->Data;
                    s32 from_idx = *payload_data;
                    if(from_idx != t_idx){
                        Transaction* t = pm->month->transactions;
                        for(s32 i=0; i <= from_idx; ++i){
                            t = t->next;
                        }
                        dll_swap(t, trans, Transaction);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + date_column_start);
            ImGui::PushItemWidth(date_column_width);
            String8 date_id = str8_formatted(scratch.arena, "##date%i", t_idx);
            ImGui::InputText((char*)date_id.data, trans->date, 128, ImGuiInputTextFlags_CharsDecimal);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + amount_column_start);
            ImGui::PushItemWidth(amount_column_width);
            String8 amount_id = str8_formatted(scratch.arena, "##amount%i", t_idx);
            ImGui::InputText((char*)amount_id.data, trans->amount, 128, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + description_column_start);
            ImGui::PushItemWidth(description_column_width);
            String8 desc_id = str8_formatted(scratch.arena, "##description%i", t_idx);
            ImGui::InputText((char*)desc_id.data, trans->description, 128);

            ImGui::PopItemWidth();

            // note: selection box
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(category_select_column_width);

                // make selection box not transparent
                ImVec4 popup_bg_color = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
                popup_bg_color.w = 1.0f; // Set alpha to 1.0 (fully opaque)
                ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetColorU32(popup_bg_color));

                // color selection red if not found in category names
                ImVec4 frame_bg_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
                bool found = false;
                char space[] = " ";
                if(!char_compare(trans->selection, space)){
                    category = pm->categories;
                    for(s32 c_idx = 0; c_idx < pm->categories_count && !found; ++c_idx){
                        category = category->next;

                        Row* row = category->rows;
                        for(s32 r_idx = 0; r_idx < category->row_count && !found; ++r_idx){
                            row = row->next;
                            String8 trans_selection = str8(trans->selection, char_length(trans->selection));

                            String8 cat_part = str8_format(tm->frame_arena, "%s: ", category->name);
                            String8 name_part = str8(row->name, char_length(row->name));
                            String8 cat_row_name = str8_concatenate(tm->frame_arena, cat_part, name_part);

                            //u32 row_length = char_length(row->name);
                            if(cat_row_name.count == trans_selection.count){
                                if(str8_compare(trans_selection, cat_row_name)){
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

                // populate selection box with options
                String8 combo_id = str8_formatted(scratch.arena, "##category_select%i", t_idx);
                if(ImGui::BeginCombo((char*)combo_id.data, trans->selection)){
                    for (int n = 0; n < pm->selection_count; n++){
                        String8 selection_item = pm->selection_list[n];
                        String8 trans_selection = str8(trans->selection, char_length(trans->selection));

                        if(selection_item.size == 0){
                            continue;
                        }

                        const bool is_selected = str8_compare(selection_item, trans_selection);
                        if(ImGui::Selectable((char*)selection_item.str, is_selected)){
                            memcpy((void*)trans->selection, (void*)selection_item.str, selection_item.size + 1);
                        }

                        if(is_selected){
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopStyleColor(2);
                ImGui::PopItemWidth();
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + plus_expense_column_start);
            String8 delete_id = str8_formatted(scratch.arena, "x##remove_transaction%i", t_idx);
            if(ImGui::Button((char*)delete_id.data)){
                pm->month->transactions_count--;

                dll_remove(trans);
                pool_free(pm->transaction_pool, trans);
            }
            if(t_idx == 0){
                ImGui::SameLine();
                if(ImGui::Button("x all##x_all")){
                    Transaction* t = pm->month->transactions;
                    for(s32 t_idx=0; t_idx < pm->month->transactions_count; ++t_idx){
                        t = t->next;
                        dll_remove(t);
                        pool_free(pm->transaction_pool, t);
                        t = pm->month->transactions;
                    }
                    dll_clear(pm->month->transactions);
                    pm->month->transactions_count = 0;
                }
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

                trans = pm->month->transactions;
                for(s32 t_idx = 0; t_idx < pm->month->transactions_count; ++t_idx){
                    trans = trans->next;

                    u32 t_length = char_length(trans->selection);
                    String8 trans_selection = str8(trans->selection, t_length);

                    u32 r_length = char_length(row->name);
                    String8 cat_part = str8_format(tm->frame_arena, "%s: ", category->name);
                    String8 name_part = str8(row->name, r_length);
                    String8 full = str8_concatenate(tm->frame_arena, cat_part, name_part);
                    //if(r_l == t_l && r_l > 0){
                        if(str8_compare(full, trans_selection)){
                        //if(char_compare(row->name, trans->selection)){
                            f32 amount = atof(trans->amount);
                            row->actual += amount;
                        }
                    //}
                }

            }
        }

        end_scratch(scratch);
        ImGui::End();

        // todo: do this once
        for(s32 i=0; i < 12; ++i){
            pm->tab_flags[i] = 0;
        }

        //ImGui::ShowDemoWindow(); // Show demo window! :)

        {
            d3d_context->ClearRenderTargetView(d3d_framebuffer_view, BACKGROUND_COLOR.e);
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            d3d_swapchain->Present(1, 0);
        }

        arena_free(tm->frame_arena);
		simulations = 0;
        total_frames++;
        //end_profiler();
    }

    if(should_quit){
        if(!os_file_exists(saves_path)){
            os_dir_create(saves_path);
        }
        serialize_data();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    d3d_release();
    end_profiler();

    return(0);
}

