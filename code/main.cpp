#include "main.h"

static void
initialize_years_and_transactions(void){
    time_t t = time(0);
    struct tm *tm_info = localtime(&t);
    pm->current_year = tm_info->tm_year + 1900;

    if(!config_year_idx_deserialized){
        pm->year_idx = 0;
    }

    // Setup year numbers before and after current year.
    //for(s32 idx=0; idx < MAX_YEAR_COUNT/2; ++idx){
    //    Year* year = pm->years + idx;
    //    year->number = pm->current_year + idx;
    //}
    //for(s32 idx=MAX_YEAR_COUNT-1; idx >= MAX_YEAR_COUNT/2; --idx){
    //    Year* year = pm->years + idx;
    //    year->number = pm->current_year - (MAX_YEAR_COUNT - idx);
    //}
    for(s32 idx=MAX_YEAR_COUNT/2; idx < MAX_YEAR_COUNT; ++idx){
        Year* year = pm->years + idx;
        year->number = pm->current_year + (idx - (MAX_YEAR_COUNT/2));
    }
    for(s32 idx=MAX_YEAR_COUNT/2; idx >= 0; --idx){
        Year* year = pm->years + idx;
        year->number = pm->current_year + (idx - (MAX_YEAR_COUNT/2));
    }

    for(s32 idx=0; idx < MAX_YEAR_COUNT; ++idx){
        Year* year = pm->years + idx;

        // Setup sentinel nodes for month transactions.
        for(s32 i=0; i < Month_Count; ++i){
            MonthInfo* month = year->months + i;
            month->transactions = (Transaction*)pool_next(pm->transaction_pool);
            dll_clear(month->transactions);
        }

        deserialize_year(year);

    }

    pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
}

static void
tooltip(String8 str){
    if(show_tooltips){
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal|ImGuiHoveredFlags_NoSharedDelay))
           ImGui::SetTooltip((char*)str.data, ImGui::GetStyle().HoverDelayNormal);
    }
}

static void
change_resolution(Window* window, f32 width, f32 height) {
    window->width = width;
    window->height = height;
    window->aspect_ratio = window->width/window->height;

    s32 style = GetWindowLong(window->handle, GWL_STYLE);
    RECT rect = {0, 0, (s32)width, (s32)height};
    AdjustWindowRect(&rect, (DWORD)style, FALSE);

    u32 flags = 0;
    if(window->type == WindowType_Fullscreen){
        flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
    }
    if(window->type == WindowType_Windowed){
        flags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
    }
    SetWindowPos(window->handle, 0,
                 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                 flags);

}

static bool
handle_controller_events(Event event){
    if(event.type == EventType_MOUSE){
        controller.mouse.x  = event.mouse_x;
        controller.mouse.y  = event.mouse_y;
        controller.mouse.dx = event.mouse_dx;
        controller.mouse.dy = event.mouse_dy;
        //controller.mouse.edge_left   = event.mouse_edge_left;
        //controller.mouse.edge_right  = event.mouse_edge_right;
        //controller.mouse.edge_top    = event.mouse_edge_top;
        //controller.mouse.edge_bottom = event.mouse_edge_bottom;
    }
    if(event.type == EventType_KEYBOARD){
        controller.mouse.wheel_dir = event.mouse_wheel_dir;
        // todo(rr): change this to
        // controller.button[event.keycode].pressed = event.key_pressed;
        // and check for repeat for held.
        if(event.key_pressed){
            controller.button[event.keycode].pressed = true;
            controller.button[event.keycode].held = true;
        }
        if(event.key_released){
            controller.button[event.keycode].released = true;
            controller.button[event.keycode].held = false;
        }
        controller.shift_pressed = event.shift_pressed;
        controller.ctrl_pressed = event.ctrl_pressed;
        controller.alt_pressed = event.alt_pressed;
    }
    return(false);
}

static void
init_paths(Arena* arena){
    build_path = os_application_path(global_arena);
    saves_path = str8_path_append(global_arena, build_path, str8_literal("saves"));
    if(!os_path_exists(saves_path)){
        os_dir_create(saves_path);
    }
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
win32_window_create(const wchar* window_name, s32 x, s32 y, s32 width, s32 height, bool maximized){
    Window result = {0};
    result.type = WindowType_Windowed;

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
    result.rect.x0 = x;
    result.rect.y0 = y;
    result.rect.x1 = x + width;
    result.rect.y1 = y + height;
    result.aspect_ratio = result.width/result.height;
    result.maximized = maximized;

    // adjust window size to exclude client area
    DWORD style = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
    if(maximized){
        style |= WS_MAXIMIZE;
    }

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, style, FALSE);
    s32 adjusted_w = rect.right - rect.left;
    s32 adjusted_h = rect.bottom - rect.top;

    result.handle = CreateWindowW(L"window class", window_name, style, x, y, adjusted_w, adjusted_h, 0, 0, GetModuleHandle(0), 0);
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
            event.type = EventType_QUIT;
            events_add(&events, event);
        } break;

        case WM_PAINT:{
            //do_one_frame();

            ValidateRect(hwnd, 0);
        } break;

        case WM_MOVE:{
            RECT rect;
            GetWindowRect(window.handle, &rect);
            window.rect.x0 = rect.left;
            window.rect.y0 = rect.top;
        } break;

        case WM_SIZE:{
            if(w_param == SIZE_MAXIMIZED){
                window.maximized = true;

                WINDOWPLACEMENT wp;
                if(GetWindowPlacement(window.handle, &wp)){
                    window_restored_rect.left = wp.rcNormalPosition.left;
                    window_restored_rect.top = wp.rcNormalPosition.top;
                    window_restored_rect.right = wp.rcNormalPosition.right;
                    window_restored_rect.bottom = wp.rcNormalPosition.bottom;
                }
            }
            else if(w_param == SIZE_RESTORED){
                window.maximized = false;
            }
            RECT rect;
            GetClientRect(hwnd, &rect);

            f32 new_width = (f32)(rect.right - rect.left);
            f32 new_height = (f32)(rect.bottom - rect.top);
            change_resolution(&window, new_width, new_height);
            d3d_resize_window(new_width, new_height);

        } break;

        case WM_MOUSEMOVE:{
            Event event;
            event.type = EventType_MOUSE; // TODO: maybe have this be a KEYBOARD event
            event.mouse_x = (s32)(s16)(l_param & 0xFFFF);
            event.mouse_y = (s32)(s16)(l_param >> 16);

            // calc dx/dy and normalize from -1:1
            f32 dx = event.mouse_x - controller.mouse.x;
            f32 dy = event.mouse_y - controller.mouse.y;
            v2 delta_normalized = normalize_v2(make_v2(dx, dy));
            event.mouse_dx = delta_normalized.x;
            event.mouse_dy = delta_normalized.y;

            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;

        case WM_MOUSEWHEEL:{
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.mouse_wheel_dir = GET_WHEEL_DELTA_WPARAM(w_param) > 0? 1 : -1;
            if(event.mouse_wheel_dir > 0){
                event.keycode = MOUSE_WHEEL_UP;
            }
            else{
                event.keycode = MOUSE_WHEEL_DOWN;
            }

            event.key_pressed = true;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:{
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.keycode = MOUSE_BUTTON_LEFT;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed  = message == WM_LBUTTONDOWN ? true : false;
            event.key_released = message == WM_LBUTTONUP   ? true : false;

            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:{
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.keycode = MOUSE_BUTTON_RIGHT;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed  = message == WM_RBUTTONDOWN ? true : false;
            event.key_released = message == WM_RBUTTONUP   ? true : false;

            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:{
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.keycode = MOUSE_BUTTON_MIDDLE;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed  = message == WM_MBUTTONDOWN ? true : false;
            event.key_released = message == WM_MBUTTONUP   ? true : false;

            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:{
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.keycode = w_param;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed = true;
            event.key_released = false;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;
        case WM_SYSKEYUP:
        case WM_KEYUP:{
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.keycode = w_param;

            event.key_pressed = false;
            event.key_released = true;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = false; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = false; }
            if(w_param == VK_MENU)    { alt_pressed   = false; }
        } break;

        case WM_CHAR:{
            u64 keycode = w_param;

            if(keycode > 31){
                Event event;
                event.type = EventType_TEXT_INPUT;
                event.keycode = keycode;
                events_add(&events, event);
            }

        } break;
        default:{
            result = DefWindowProcW(hwnd, message, w_param, l_param);
        } break;
    }
    return(result);
}

static void
draw_entire_ui(void){
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImVec2 window_size = ImVec2(window.width, window.height);
        ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

        Year* year = pm->year;
        MonthInfo* month = year->months + pm->month_tab_idx;

        ImGui::Begin("Budgeteer", 0, ImGuiWindowFlags_NoResize|
                                     ImGuiWindowFlags_NoCollapse|
                                     ImGuiWindowFlags_NoMove|
                                     ImGuiWindowFlags_NoTitleBar);
        ImGui::Checkbox("Show Tooltips", &show_tooltips);
#if DEBUG
        ImGui::SameLine();
        if(ImGui::Button("DEBUG##debug")){
            debug_show_window = !debug_show_window;
            // note: feels like a dumb solution to size the window once at the start of a session
            if(debug_show_window && debug_size_window){
                debug_size_window = false;
                ImGui::SetNextWindowPos(ImVec2(200, 100), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
            }
        }

        if(debug_show_window){
            ImGui::Begin("My Window", &debug_show_window); // second param lets user close
            Arena* scratch_one = scratch_pool[0];
            Arena* scratch_two = scratch_pool[1];
            Arena* scratch_three = scratch_pool[2];

            ImGui::PushFont(my_font12);
            if(debug_show_scratch){
                fmt = str8_fmt(tm->frame_arena, "%c##debug_show_scratch", icon_lookup[Icon_Collapse]);
                if(ImGui::Button((char*)fmt.str)){
                    debug_show_scratch = false;
                }
            }
            else{
                fmt = str8_fmt(tm->frame_arena, "%c##debug_show_scratch", icon_lookup[Icon_Expand]);
                if(ImGui::Button((char*)fmt.str)){
                    debug_show_scratch = true;
                }
            }
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Text("Scratch Memory");

            if(debug_show_scratch){
                ImGui::Text("   Scratch Begin/End: %i/%i", begin_scratch_count, end_scratch_count);
                ImGui::Text("   Scratch One:");
                ImGui::Text("       %i/%i (%.2f%%)", scratch_one->at, scratch_one->size, 100*((f32)scratch_one->at/(f32)scratch_one->size));
                ImGui::Text("   Scratch Two:");
                ImGui::Text("       %i/%i (%.2f%%)", scratch_two->at, scratch_two->size, 100*((f32)scratch_two->at/(f32)scratch_two->size));
                ImGui::Text("   Scratch Three:");
                ImGui::Text("       %i/%i (%.2f%%)", scratch_three->at, scratch_three->size, 100*((f32)scratch_three->at/(f32)scratch_three->size));
            }
            custom_separator();

            ImGui::PushFont(my_font12);
            if(debug_show_pm_memory){
                fmt = str8_fmt(tm->frame_arena, "%c##debug_show_pm_memory", icon_lookup[Icon_Collapse]);
                if(ImGui::Button((char*)fmt.str)){
                    debug_show_pm_memory = false;
                }
            }
            else{
                fmt = str8_fmt(tm->frame_arena, "%c##debug_show_pm_memory", icon_lookup[Icon_Expand]);
                if(ImGui::Button((char*)fmt.str)){
                    debug_show_pm_memory = true;
                }
            }
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Text("Permanent Memory");

            if(debug_show_pm_memory){
                ImGui::Text("   Arena:");
                ImGui::Text("       %i/%i (%.2f%%)", pm->arena.at, pm->arena.size, 100*((f32)pm->arena.at/(f32)pm->arena.size));
                ImGui::Text("       Pools:");
                ImGui::Text("           category_group_pool:");
                ImGui::Text("               size: %i", pm->category_group_pool->size);
                ImGui::Text("               chunk_size: %i", pm->category_group_pool->chunk_size);
                ImGui::Text("               chunk_total: %i", pm->category_group_pool->chunk_total);
                ImGui::Text("               chunk_at: %i", pm->category_group_pool->chunk_at);
                ImGui::Text("           category_pool:");
                ImGui::Text("               size: %i", pm->category_pool->size);
                ImGui::Text("               chunk_size: %i", pm->category_pool->chunk_size);
                ImGui::Text("               chunk_total: %i", pm->category_pool->chunk_total);
                ImGui::Text("               chunk_at: %i", pm->category_pool->chunk_at);
                ImGui::Text("           transaction_pool:");
                ImGui::Text("               size: %i", pm->transaction_pool->size);
                ImGui::Text("               chunk_size: %i", pm->transaction_pool->chunk_size);
                ImGui::Text("               chunk_total: %i", pm->transaction_pool->chunk_total);
                ImGui::Text("               chunk_at: %i", pm->transaction_pool->chunk_at);
                ImGui::Text("           csv_profile_pool_pool:");
                ImGui::Text("               size: %i", pm->csv_profile_pool->size);
                ImGui::Text("               chunk_size: %i", pm->csv_profile_pool->chunk_size);
                ImGui::Text("               chunk_total: %i", pm->csv_profile_pool->chunk_total);
                ImGui::Text("               chunk_at: %i", pm->csv_profile_pool->chunk_at);
                ImGui::Text("           data_arena:");
                ImGui::Text("               %i/%i (%.2f%%)", pm->arena.at, pm->arena.size, 100*((f32)pm->arena.at/(f32)pm->arena.size));
            }
            custom_separator();

            ImGui::PushFont(my_font12);
            if(debug_show_tm_memory){
                fmt = str8_fmt(tm->frame_arena, "%c##debug_show_tm_memory", icon_lookup[Icon_Collapse]);
                if(ImGui::Button((char*)fmt.str)){
                    debug_show_tm_memory = false;
                }
            }
            else{
                fmt = str8_fmt(tm->frame_arena, "%c##debug_show_tm_memory", icon_lookup[Icon_Expand]);
                if(ImGui::Button((char*)fmt.str)){
                    debug_show_tm_memory = true;
                }
            }
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Text("Transient Memory");

            if(debug_show_tm_memory){
                ImGui::Text("   Arena:");
                ImGui::Text("       %i/%i", tm->arena.at, tm->arena.size);
                ImGui::Text("       frame_arena:");
                ImGui::Text("           %i/%i (%.2f%%)", tm->frame_arena->at, tm->frame_arena->size, 100*((f32)tm->frame_arena->at/(f32)tm->frame_arena->size));
                ImGui::Text("       options_arena:");
                ImGui::Text("           %i/%i (%.2f%%)", tm->options_arena->at, tm->options_arena->size, 100*((f32)tm->options_arena->at/(f32)tm->options_arena->size));

            }
            if(ImGui::Button("Generate Merchant List")){
                for(s32 year_idx=0; year_idx < MAX_YEAR_COUNT; ++year_idx){
                    Year* year = pm->years + year_idx;
                    for(s32 month_idx=0; month_idx < Month_Count; ++month_idx){
                        MonthInfo* month = year->months + month_idx;
                        month->transaction_visible_count = 0;
                        for(Transaction* trans = month->transactions->next; trans != month->transactions; trans = trans->next){
                            String8 trans_description_str = str8_cstring(trans->description);
                            String8 trans_category_str = str8_cstring(trans->category);

                            bool found = false;
                            for(Merchant* merch = pm->merchants; merch != 0; merch = merch->next){
                                String8 merch_description = str8_cstring(merch->description);
                                if(str8_compare(merch_description, trans_description_str)){
                                    trans->merchant_id = merch->id;
                                    found = true;
                                    break;
                                }
                            }
                            if(!found){
                                Merchant* merch = push_struct(&pm->arena, Merchant);
                                sll_push_front(pm->merchants, merch);

                                if(trans_category_str.count){
                                    str8_copy_to_char(merch->category, trans_category_str, MERCH_CATEGORY_SIZE);
                                }
                                str8_copy_to_char(merch->description, trans_description_str, MERCH_DESCRIPTION_SIZE);
                                str8_copy_to_char(merch->category, empty_category, empty_category.count);
                                merch->id = merchant_id++;

                                trans->merchant_id = merch->id;
                            }
                        }
                    }
                }
            }


            ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
            ImGui::End();
        }
        // END DEBUG WINDOW
#endif

        ScratchArena scratch = begin_scratch();
        ImGui::Columns(2);
        ImGui::BeginChild("Child1", ImVec2(0, 0), true);

        ImGui::Text("Budget:");
        ImGui::SameLine();
        ImGui::PushItemWidth(75);
        //if(pm->budget[0] == 0){
        //    pm->budget[0] = '0';
        //    pm->budget[1] = '\0';
        //}
        ImGui::InputText("##Budget", pm->budget, 128, ImGuiInputTextFlags_CharsDecimal |
                                                      ImGuiInputTextFlags_AutoSelectAll);
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        // TOTALS
        // TOTALS
        // TOTALS
        ImGui::BeginChild("Child2", ImVec2(0, 0), ImGuiChildFlags_Border |
                                                   ImGuiChildFlags_AutoResizeY);
        ImGui::Columns(4);
        if(ImGui::BeginTabBar("##monthmonth", ImGuiTabBarFlags_None)){

            ImGui::PushStyleColor(ImGuiCol_TabActive, active_color);
            ImGui::PushStyleColor(ImGuiCol_TabHovered, hover_color);

            if(ImGui::BeginTabItem(month_names[pm->month_tab_idx], 0)){
                ImGui::EndTabItem();
            }
            ImGui::PopStyleColor(2);
            ImGui::EndTabBar();
        }
        ImGui::SeparatorText("Monthly Totals");
        ImGui::Text("Planned: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        ImGui::Text("$%.2f", month->totals.planned);
        ImGui::Text("Spent: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        ImGui::Text("$%.2f", month->totals.spent);
        ImGui::Text("Diff: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        if(month->totals.diff < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("$%.2f", month->totals.diff);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", month->totals.diff);
        }
        custom_separator();
        ImGui::Text("Goal: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        if(month->totals.goal < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("$%.2f", month->totals.goal);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", month->totals.goal);
        }
        ImGui::Text("Saved: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(totals_number_start);
        if(month->totals.saved < month->totals.goal){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("$%.2f", month->totals.saved);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", month->totals.saved);
        }

        ImGui::NextColumn();
        if(ImGui::BeginTabBar("##quarter", ImGuiTabBarFlags_None)){

            ImGui::PushStyleColor(ImGuiCol_TabActive, active_color);
            ImGui::PushStyleColor(ImGuiCol_TabHovered, hover_color);

            if(ImGui::BeginTabItem("Q1", 0, pm->quarter_tab_flags[0])){
                pm->quarter_tab_idx = 0;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Q2", 0, pm->quarter_tab_flags[1])){
                pm->quarter_tab_idx = 1;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Q3", 0, pm->quarter_tab_flags[2])){
                pm->quarter_tab_idx = 2;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Q4", 0, pm->quarter_tab_flags[3])){
                pm->quarter_tab_idx = 3;
                ImGui::EndTabItem();
            }
            ImGui::PopStyleColor(2);
            ImGui::EndTabBar();
        }
        ImGui::SeparatorText("Quarterly Totals");
        Totals* totals = pm->quarter_totals + pm->quarter_tab_idx;
        ImGui::Text("Planned: ");
        ImGui::SameLine();
        f32 x_pos = ImGui::GetCursorPosX();
        ImGui::SetCursorPosX(x_pos);
        ImGui::Text("$%.2f", totals->planned);
        ImGui::Text("Spent: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        ImGui::Text("$%.2f", totals->spent);
        ImGui::Text("Diff: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        if(totals->diff < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("$%.2f", totals->diff);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", totals->diff);
        }
        custom_separator();
        ImGui::Text("Goal: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        if(totals->goal < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("$%.2f", totals->goal);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", totals->goal);
        }
        ImGui::Text("Saved: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        if(totals->saved < totals->goal){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("%.2f", totals->saved);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", totals->saved);
        }

        ImGui::NextColumn();
        if(ImGui::BeginTabBar("##biannual", ImGuiTabBarFlags_None)){

            ImGui::PushStyleColor(ImGuiCol_TabActive, active_color);
            ImGui::PushStyleColor(ImGuiCol_TabHovered, hover_color);

            if(ImGui::BeginTabItem("Q1/Q2", 0, pm->biannual_tab_flags[0])){
                pm->biannual_tab_idx = 0;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Q3/Q4", 0, pm->biannual_tab_flags[1])){
                pm->biannual_tab_idx = 1;
                ImGui::EndTabItem();
            }
            ImGui::PopStyleColor(2);
            ImGui::EndTabBar();
        }
        ImGui::SeparatorText("BiAnnual Totals");
        totals = pm->biannual_totals + pm->biannual_tab_idx;
        ImGui::Text("Planned: ");
        ImGui::SameLine();
        x_pos = ImGui::GetCursorPosX();
        ImGui::SetCursorPosX(x_pos);

        ImGui::Text("$%.2f", totals->planned);
        ImGui::Text("Spent: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        ImGui::Text("$%.2f", totals->spent);
        ImGui::Text("Diff: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        if(totals->diff < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("$%.2f", totals->diff);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", totals->diff);
        }
        custom_separator();
        ImGui::Text("Goal: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        if(totals->goal < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("$%.2f", totals->goal);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", totals->goal);
        }
        ImGui::Text("Saved: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        if(totals->saved < totals->goal){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("$%.2f", totals->saved);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", totals->saved);
        }

        ImGui::NextColumn();
        if(ImGui::BeginTabBar("##yeartabbar", ImGuiTabBarFlags_None)){

            ImGui::PushStyleColor(ImGuiCol_TabActive, active_color);
            ImGui::PushStyleColor(ImGuiCol_TabHovered, hover_color);

            fmt = str8_fmt(scratch.arena, "%i", pm->year->number);
            if(ImGui::BeginTabItem((char*)fmt.str, 0)){
                ImGui::EndTabItem();
            }
            ImGui::PopStyleColor(2);
            ImGui::EndTabBar();
        }
        //ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::SeparatorText("Annual Totals");
        totals = &pm->annual_totals;
        ImGui::Text("Planned: ");
        ImGui::SameLine();
        x_pos = ImGui::GetCursorPosX();
        ImGui::SetCursorPosX(x_pos);
        ImGui::Text("$%.2f", totals->planned);
        ImGui::Text("Spent: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        ImGui::Text("$%.2f", totals->spent);
        ImGui::Text("Diff: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        if(totals->diff < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("$%.2f", totals->diff);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", totals->diff);
        }
        custom_separator();
        ImGui::Text("Goal: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        if(totals->goal < 0){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("$%.2f", totals->goal);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", totals->goal);
        }
        ImGui::Text("Saved: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_pos);
        if(totals->saved < totals->goal){
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            ImGui::Text("$%.2f", totals->saved);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", totals->saved);
        }
        ImGui::EndChild();
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        //#####MONTH PLAN######
        ImGui::BeginChild("Child3", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar|
                                                        ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        ImGui::PushFont(my_font12);
        if(pm->draw_month_plan){
            fmt = str8_fmt(tm->frame_arena, "%c##draw_month", icon_lookup[Icon_Collapse]);
            if(ImGui::Button((char*)fmt.str)){
                pm->draw_month_plan = false;
            }
        }
        else{
            fmt = str8_fmt(tm->frame_arena, "%c##draw_month", icon_lookup[Icon_Expand]);
            if(ImGui::Button((char*)fmt.str)){
                pm->draw_month_plan = true;
            }
        }
        ImGui::PopFont();
        tooltip(str8_literal("Collapse/Expand Month Plan."));

        ImGui::SameLine();
        ImGui::SeparatorText("Month Plan");

        if(pm->draw_month_plan){
            ImGui::SetCursorPosX(category_count_column_start + input_padding);
            ImGui::Text("#");
            ImGui::SameLine();
            ImGui::SetCursorPosX(category_group_column_start);
            ImGui::Text("Category Group");
            ImGui::SameLine();
            ImGui::SetCursorPosX(planned_column_start);
            ImGui::Text("Planned");
            ImGui::SameLine();
            ImGui::SetCursorPosX(spent_column_start);
            ImGui::Text("Spent");
            ImGui::SameLine();
            ImGui::SetCursorPosX(diff_column_start);
            ImGui::Text("Diff");
            ImGui::SameLine();
            ImGui::SetCursorPosX(plus_column_start);
            if(ImGui::Button("+##add_category_group_button")){
                if(pm->category_groups_count < MAX_CATEGORY_GROUP_COUNT){
                    CategoryGroup* category_group = (CategoryGroup*)pool_next(pm->category_group_pool);
                    dll_push_back_old(pm->month_category_groups, category_group);

                    category_group->categories = (Category*)pool_next(pm->category_pool);
                    dll_clear(category_group->categories);

                    pm->category_groups_count++;
                }
                else{
                    // todo(rr): STATUSBAR ERROR
                }
            }
            tooltip(str8_literal("Add New Category Group."));

            CategoryGroup* category_group = pm->month_category_groups;
            for(s32 c_idx = 0; c_idx < pm->category_groups_count; ++c_idx){
                custom_separator();
                category_group = category_group->next;

                ImGui::SetCursorPosX(collapse_column_start);
                ImGui::PushID(c_idx);
                ImGui::PushFont(my_font12);
                if(category_group->draw_categories){
                    fmt = str8_fmt(tm->frame_arena, "%c##", icon_lookup[Icon_Collapse]);
                    if(ImGui::Button((char*)fmt.str)){
                        category_group->draw_categories = false;
                    }
                }
                else{
                    fmt = str8_fmt(tm->frame_arena, "%c##", icon_lookup[Icon_Expand]);
                    if(ImGui::Button((char*)fmt.str)){
                        if(category_group->category_count){
                            category_group->draw_categories = true;
                        }
                    }
                }
                ImGui::PopFont();

                tooltip(str8_literal("Collapse/Expand Or Drag/Swap Category Group."));
                ImGui::PopID();

                if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)){
                    ImGui::SetDragDropPayload("DRAG_CATEGORY", &c_idx, sizeof(s32));
                    ImGui::Text("%s", category_group->name);
                    ImGui::EndDragDropSource();
                }
                if(ImGui::BeginDragDropTarget()){
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_CATEGORY")){
                        s32* payload_data = (s32*)payload->Data;
                        s32 from_idx = *payload_data;
                        if(from_idx != c_idx){
                            CategoryGroup* cg = pm->month_category_groups;
                            for(s32 i=0; i <= from_idx; ++i){
                                cg = cg->next;
                            }
                            dll_swap(cg, category_group, CategoryGroup);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::SameLine();
                ImGui::SetCursorPosX(category_count_column_start + input_padding);
                if(category_group->draw_categories){
                    ImGui::Text("-");
                }
                else{
                    ImGui::Text("%i", category_group->category_count);
                }

                ImGui::SameLine();


                ImGui::SetCursorPosX(category_group_column_start);
                ImGui::PushItemWidth(category_group_column_width);
                fmt = str8_fmt(scratch.arena, "##category_group%i", c_idx);
                ImGui::InputText((char*)fmt.data, category_group->name, CATEGORY_NAME_SIZE, ImGuiInputTextFlags_AutoSelectAll);
                ImGui::PopItemWidth();

                ImGui::SameLine();
                ImGui::SetCursorPosX(planned_column_start + input_padding);
                fmt = str8_fmt(scratch.arena, "$%.2f", category_group->planned);
                ImGui::Text((char*)fmt.data);

                ImGui::SameLine();
                ImGui::SetCursorPosX(spent_column_start + input_padding);
                fmt = str8_fmt(scratch.arena, "$%.2f", category_group->spent);
                ImGui::Text((char*)fmt.data);

                ImGui::SameLine();
                ImGui::SetCursorPosX(diff_column_start);
                category_group->diff = category_group->planned - category_group->spent;
                fmt = str8_fmt(scratch.arena, "$%.2f", category_group->diff);
                if(category_group->diff < 0){
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
                }
                ImGui::Text((char*)fmt.data);
                if(category_group->diff < 0){
                    ImGui::PopStyleColor();
                }

                ImGui::SameLine();
                ImGui::SetCursorPosX(plus_column_start);
                ImGui::PushID(c_idx);
                if(ImGui::Button("+##add_category_button")){
                    if(pm->total_categories_count < MAX_CATEGORY_COUNT){
                        Category* c = (Category*)pool_next(pm->category_pool);
                        dll_push_back_old(category_group->categories, c);

                        category_group->draw_categories = true;
                        category_group->category_count++;
                        pm->total_categories_count++;
                    }
                    else{
                        // todo(rr): STATUSBAR ERROR
                    }
                }
                tooltip(str8_literal("Add New Category."));
                ImGui::PopID();

                ImGui::SameLine();
                ImGui::SetCursorPosX(x_column_start);
                ImGui::PushID(c_idx);
                if(ImGui::Button("x##remove_category_group")){
                    pm->total_categories_count -= category_group->category_count;
                    --pm->category_groups_count;

                    dll_remove_old(category_group);
                    pool_free(pm->category_group_pool, category_group);
                }
                tooltip(str8_literal("Delete All Sub-Categories."));
                ImGui::PopID();

                ImGui::SameLine();
                ImGui::SetCursorPosX(m_column_start);
                ImGui::PushID(c_idx);
                if(category_group->muted){
                    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(RED.r, RED.g, RED.b, 255));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.1f, 0.0f, 1.0f));
                }
                else{
                    ImGui::PushStyleColor(ImGuiCol_Button, pm->default_button_color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pm->default_button_hovered_color);
                }
                if(ImGui::Button("m##mute_category_group")){
                    category_group->muted = !category_group->muted;
                    if(category_group->muted){
                        Category* category = category_group->categories;
                        for(s32 r_idx = 0; r_idx < category_group->category_count; ++r_idx){
                            category = category->next;
                            category->muted = true;
                        }
                    }
                    else{
                        Category* category = category_group->categories;
                        for(s32 r_idx = 0; r_idx < category_group->category_count; ++r_idx){
                            category = category->next;
                            category->muted = false;
                        }
                    }
                }
                tooltip(str8_literal("Mute All Sub-Categories."));
                ImGui::PopID();
                ImGui::PopStyleColor(2);

                // render categories per category_group
                Category* category = category_group->categories;
                for(s32 r_idx = 0; r_idx < category_group->category_count; ++r_idx){
                    category = category->next;

                    // note: Popluate empty planned with 0's for visual appeal.
                    // They need to be null terminated because C style strings yuck.
                    if(category->planned[0] == 0){
                        category->planned[0] = '0';
                        category->planned[1] = '\0';
                    }

                    fmt = str8_fmt(scratch.arena, "%i%i", r_idx + 1, c_idx + 1);
                    s32 uid = atoi((char*)fmt.data);

                    if(category_group->draw_categories){
                        ImGui::SetCursorPosX(category_count_column_start);
                        ImGui::PushID(uid);
                        fmt = str8_fmt(scratch.arena, "%i", r_idx + 1);
                        ImGui::Button((char*)fmt.data);
                        ImGui::PopID();

                        if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)){
                            ImGui::SetDragDropPayload("DRAG_CATEGORY", &r_idx, sizeof(s32));
                            ImGui::Text("%s", category->name);
                            ImGui::EndDragDropSource();
                        }
                        tooltip(str8_literal("Drag/Swap Category."));

                        if(ImGui::BeginDragDropTarget()){
                            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_CATEGORY")){
                                s32* payload_data = (s32*)payload->Data;
                                s32 from_idx = *payload_data;
                                if(from_idx != r_idx){
                                    Category* c = category_group->categories;
                                    // note(rr): iterate to the correct node, since we cant index with linked lists
                                    for(s32 i=0; i <= from_idx; ++i){
                                        c = c->next;
                                    }
                                    dll_swap(c, category, Category);
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }

                        ImGui::SameLine();

                        ImGui::SetCursorPosX(category_group_column_start);
                        ImGui::PushItemWidth(category_group_column_width);
                        fmt = str8_fmt(scratch.arena, "##sub_category_group%i%i", r_idx, c_idx);
                        ImGui::InputText((char*)fmt.data, category->name, CATEGORY_NAME_SIZE, ImGuiInputTextFlags_AutoSelectAll);
                        ImGui::PopItemWidth();

                        ImGui::SameLine();
                        ImGui::SetCursorPosX(planned_column_start);
                        ImGui::PushItemWidth(planned_column_width);
                        fmt = str8_fmt(scratch.arena, "##planned%i%i", r_idx, c_idx);
                        ImGui::InputText((char*)fmt.data, category->planned, CATEGORY_PLANNED_SIZE, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll);

                        ImGui::PopItemWidth();

                        ImGui::SameLine();
                        ImGui::SetCursorPosX(spent_column_start + input_padding);
                        ImGui::PushItemWidth(spent_column_width);
                        fmt = str8_fmt(scratch.arena, "$%.2f", category->spent);
                        ImGui::Text((char*)fmt.data);
                        ImGui::PopItemWidth();

                        ImGui::SameLine();
                        ImGui::SetCursorPosX(diff_column_start);
                        f32 planned = atof(category->planned);
                        if((planned - category->spent) < 0){
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
                        }
                        fmt = str8_fmt(scratch.arena, "$%.2f", category->diff);
                        ImGui::Text((char*)fmt.data);
                        if((planned - category->spent) < 0){
                            ImGui::PopStyleColor();
                        }

                        ImGui::SameLine();
                        ImGui::SetCursorPosX(x_column_start);
                        ImGui::PushID(uid);
                        if(ImGui::Button("x##remove_category")){
                            --pm->total_categories_count;
                            --category_group->category_count;

                            dll_remove_old(category);
                            pool_free(pm->category_pool, category);
                        }
                        tooltip(str8_literal("Delete Category."));
                        ImGui::PopID();

                        ImGui::SameLine();
                        ImGui::SetCursorPosX(m_column_start);
                        ImGui::PushID(uid);
                        if(category->muted){
                            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(RED.r, RED.g, RED.b, 255));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.1f, 0.0f, 1.0f));
                        }
                        else{
                            ImGui::PushStyleColor(ImGuiCol_Button, pm->default_button_color);
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pm->default_button_hovered_color);
                        }
                        if(ImGui::Button("m##mute_category")){
                            category->muted = !category->muted;
                        }
                        tooltip(str8_literal("Mute Category."));
                        ImGui::PopID();
                        ImGui::PopStyleColor(2);
                    }
                }
            }
        }
        custom_separator();

        //#####QUATER PLAN######
        ImGui::PushFont(my_font12);
        if(pm->draw_quarter_plan){
            fmt = str8_fmt(tm->frame_arena, "%c##draw_quarter", icon_lookup[Icon_Collapse]);
            if(ImGui::Button((char*)fmt.str)){
                pm->draw_quarter_plan = false;
            }
        }
        else{
            fmt = str8_fmt(tm->frame_arena, "%c##draw_quarter", icon_lookup[Icon_Expand]);
            if(ImGui::Button((char*)fmt.str)){
                pm->draw_quarter_plan = true;
            }
        }
        ImGui::PopFont();
        tooltip(str8_literal("Collapse/Expand Quarter Plan."));
        ImGui::SameLine();
        ImGui::SeparatorText("Quarter Plan");

        if(pm->draw_quarter_plan){
            ImGui::SetCursorPosX(category_count_column_start + input_padding);
            ImGui::Text("#");
            ImGui::SameLine();
            ImGui::SetCursorPosX(category_group_column_start);
            ImGui::Text("Q1 CategoryGroup");
            ImGui::SameLine();
            ImGui::SetCursorPosX(planned_column_start);
            ImGui::Text("Planned");
            ImGui::SameLine();
            ImGui::SetCursorPosX(spent_column_start);
            ImGui::Text("Saved");
            ImGui::SameLine();
            ImGui::SetCursorPosX(diff_column_start);
            ImGui::Text("Diff");
            ImGui::SameLine();
            ImGui::SetCursorPosX(plus_column_start);
            if(ImGui::Button("+##add_quarter_category_group_button")){
                //CategoryGroup* category_group = (CategoryGroup*)pool_next(pm->category_group_pool);
                //dll_push_back_old(pm->quarter_category_groups, category_group);
                //category_group->categories = (Category*)pool_next(pm->category_pool);
                //dll_clear(category_group->categories);

                //pm->quarter_category_groups_count++;
            }
            tooltip(str8_literal("Add New Category Group."));
        }
        custom_separator();

        ImGui::PushFont(my_font12);
        if(pm->draw_biannual_plan){
            fmt = str8_fmt(tm->frame_arena, "%c##draw_biannual", icon_lookup[Icon_Collapse]);
            if(ImGui::Button((char*)fmt.str)){
                pm->draw_biannual_plan = false;
            }
        }
        else{
            fmt = str8_fmt(tm->frame_arena, "%c##draw_biannual", icon_lookup[Icon_Expand]);
            if(ImGui::Button((char*)fmt.str)){
                pm->draw_biannual_plan = true;
            }
        }
        ImGui::PopFont();

        //#####BIANNUAL PLAN######
        tooltip(str8_literal("Collapse/Expand Bi-Annual Plan."));
        ImGui::SameLine();
        ImGui::SeparatorText("BiAnnual Plan");

        if(pm->draw_biannual_plan){
            ImGui::SetCursorPosX(category_count_column_start + input_padding);
            ImGui::Text("#");
            ImGui::SameLine();
            ImGui::SetCursorPosX(category_group_column_start);
            ImGui::Text("Q1 CategoryGroup");
            ImGui::SameLine();
            ImGui::SetCursorPosX(planned_column_start);
            ImGui::Text("Planned");
            ImGui::SameLine();
            ImGui::SetCursorPosX(spent_column_start);
            ImGui::Text("Saved");
            ImGui::SameLine();
            ImGui::SetCursorPosX(diff_column_start);
            ImGui::Text("Diff");
            ImGui::SameLine();
            ImGui::SetCursorPosX(plus_column_start);
            if(ImGui::Button("+##add_biannual_category_group_button")){
                //CategoryGroup* category_group = (CategoryGroup*)pool_next(pm->category_group_pool);
                //dll_push_back_old(pm->biannual_category_groups, category_group);
                //category_group->categories = (Category*)pool_next(pm->category_pool);
                //dll_clear(category_group->categories);

                //pm->biannual_category_groups_count++;
            }
            tooltip(str8_literal("Add New Category Group."));
        }
        custom_separator();

        ImGui::PushFont(my_font12);
        if(pm->draw_annual_plan){
            fmt = str8_fmt(tm->frame_arena, "%c##annual_plan", icon_lookup[Icon_Collapse]);
            if(ImGui::Button((char*)fmt.str)){
                pm->draw_annual_plan = false;
            }
        }
        else{
            fmt = str8_fmt(tm->frame_arena, "%c##annual_plan", icon_lookup[Icon_Expand]);
            if(ImGui::Button((char*)fmt.str)){
                pm->draw_annual_plan = true;
            }
        }
        ImGui::PopFont();

        //#####ANNUAL PLAN######
        tooltip(str8_literal("Collapse/Expand Annual Plan."));
        ImGui::SameLine();
        ImGui::SeparatorText("Annual Plan");

        if(pm->draw_annual_plan){
            ImGui::SetCursorPosX(category_count_column_start + input_padding);
            ImGui::Text("#");
            ImGui::SameLine();
            ImGui::SetCursorPosX(category_group_column_start);
            ImGui::Text("Q1 CategoryGroup");
            ImGui::SameLine();
            ImGui::SetCursorPosX(planned_column_start);
            ImGui::Text("Planned");
            ImGui::SameLine();
            ImGui::SetCursorPosX(spent_column_start);
            ImGui::Text("Saved");
            ImGui::SameLine();
            ImGui::SetCursorPosX(diff_column_start);
            ImGui::Text("Diff");
            ImGui::SameLine();
            ImGui::SetCursorPosX(plus_column_start);
            if(ImGui::Button("+##add_annual_category_group_button")){
                //CategoryGroup* category_group = (CategoryGroup*)pool_next(pm->category_group_pool);
                //dll_push_back_old(pm->annual_category_groups, category_group);
                //category_group->categories = (Category*)pool_next(pm->category_pool);
                //dll_clear(category_group->categories);

                //pm->annual_category_groups_count++;
            }
            tooltip(str8_literal("Add New Category Group."));

            //CategoryGroup* category_group = pm->annual_category_groups;
            //for(s32 c_idx = 0; c_idx < pm->annual_category_groups_count; ++c_idx){
            //    category_group = category_group->next;

            //    ImGui::SetCursorPosX(collapse_column_start);
            //    ImGui::PushID(c_idx);
            //    if(category_group->draw_categories){
            //        if(ImGui::Button("V")){
            //            category_group->draw_categories = false;
            //        }
            //    }
            //    else{
            //        if(ImGui::Button(">")){
            //            if(category_group->category_count){
            //                category_group->draw_categories = true;
            //            }
            //        }
            //    }
            //    ImGui::PopID();
            //    tooltip(str8_literal("Collapse/Expand Or Drag/Swap Category Group."));

            //    if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)){
            //        ImGui::SetDragDropPayload("DRAG_CATEGORY", &c_idx, sizeof(s32));
            //        ImGui::Text("%s", category_group->name);
            //        ImGui::EndDragDropSource();
            //    }
            //    if(ImGui::BeginDragDropTarget()){
            //        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_CATEGORY")){
            //            s32* payload_data = (s32*)payload->Data;
            //            s32 from_idx = *payload_data;
            //            if(from_idx != c_idx){
            //                CategoryGroup* cg = pm->category_groups;
            //                for(s32 i=0; i <= from_idx; ++i){
            //                    cg = cg->next;
            //                }
            //                dll_swap(c, category_group, CategoryGroup);
            //            }
            //        }
            //        ImGui::EndDragDropTarget();
            //    }

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(category_count_column_start + input_padding);
            //    if(category_group->draw_categories){
            //        ImGui::Text("-");
            //    }
            //    else{
            //        ImGui::Text("%i", category_group->category_count);
            //    }

            //    ImGui::SameLine();


            //    ImGui::SetCursorPosX(category_group_column_start);
            //    ImGui::PushItemWidth(category_group_column_width);
            //    String8 unique_id = str8_fmt(scratch.arena, "##category_group%i", c_idx);
            //    ImGui::InputText((char*)unique_id.data, category_group->name, CAT_NAME_SIZE, ImGuiInputTextFlags_AutoSelectAll);
            //    ImGui::PopItemWidth();

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(planned_column_start + input_padding);
            //    String8 planned_str = str8_fmt(scratch.arena, "%.2f", category_group->planned);
            //    ImGui::Text((char*)planned_str.data);

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(spent_column_start + input_padding);
            //    String8 spent_str = str8_fmt(scratch.arena, "%.2f", category_group->spent);
            //    ImGui::Text((char*)spent_str.data);

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(diff_column_start);
            //    category_group->diff = category_group->planned - category_group->spent;
            //    String8 category_group_diff = str8_fmt(scratch.arena, "%.2f", category_group->diff);
            //    if(category_group->diff < 0){
            //        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            //    }
            //    ImGui::Text((char*)category_group_diff.data);
            //    if(category_group->diff < 0){
            //        ImGui::PopStyleColor();
            //    }

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(plus_column_start);
            //    ImGui::PushID(c_idx);
            //    if(ImGui::Button("+##add_category_button")){
            //        Category* c = (Category*)pool_next(pm->category_pool);
            //        dll_push_back_old(category_group->categories, c);

            //        category_group->draw_categories = true;
            //        category_group->category_count++;
            //        //pm->total_categories_count++;
            //    }
            //    ImGui::PopID();

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(x_column_start);
            //    ImGui::PushID(c_idx);
            //    if(ImGui::Button("x##remove_category_group")){
            //        //pm->total_categories_count -= category_group->category_count;
            //        --pm->annual_category_groups_count;

            //        dll_remove_old(category_group);
            //        pool_free(pm->category_group_pool, category_group);
            //    }
            //    ImGui::PopID();

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(m_column_start);
            //    ImGui::PushID(c_idx);
            //    if(category_group->muted){
            //        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
            //        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.1f, 0.0f, 1.0f));
            //    }
            //    else{
            //        ImGui::PushStyleColor(ImGuiCol_Button, pm->default_button_color);
            //        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pm->default_button_hovered_color);
            //    }
            //    // todo: review this, this stuff might not have categories
            //    if(ImGui::Button("m##mute_category_group")){
            //        category_group->muted = !category_group->muted;
            //        if(category_group->muted){
            //            Category* category = category_group->categories;
            //            for(s32 r_idx = 0; r_idx < category_group->category_count; ++r_idx){
            //                category = category->next;
            //                category->muted = true;
            //            }
            //        }
            //        else{
            //            Category* category = category_group->categories;
            //            for(s32 r_idx = 0; r_idx < category_group->category_count; ++r_idx){
            //                category = category->next;
            //                category->muted = false;
            //            }
            //        }
            //    }
            //    ImGui::PopID();
            //    ImGui::PopStyleColor(2);
            //    custom_separator();
            //}
        }
        custom_separator();
        // END OF CATEGORYS
        ImGui::EndChild();
        ImGui::EndChild();

        // note: construct selection options as a combination of category and category_group.
        {
            pm->category_list_count = 1;
            CategoryGroup* category_group = pm->month_category_groups;
            for(s32 c_idx = 0; c_idx < pm->category_groups_count; ++c_idx){
                category_group = category_group->next;

                Category* category = category_group->categories;
                for(s32 r_idx = 0; r_idx < category_group->category_count; ++r_idx){
                    category = category->next;

                    String8* category_item = pm->category_list + pm->category_list_count;
                    if(category->name[0] != '\0'){
                        if(!char_only_spaces(category->name)){ // don't include categories that are named only spaces
                            u32 length = char_length(category->name);
                            String8 cat_part = str8_format(tm->frame_arena, "%s: ", category_group->name);
                            String8 name_part = str8(category->name, length + 1); // + 1 to include 0 terminater
                            String8 full = str8_concatenate(tm->frame_arena, cat_part, name_part);
                            *category_item = full;
                        }
                    }
                    else{
                        *category_item = str8("", 0);
                    }
                    pm->category_list_count++;
                }
            }
        }


        //########COLUMN2######################################################################

        ImGui::NextColumn();
        ImGui::BeginChild("Child4", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar|ImGuiWindowFlags_AlwaysHorizontalScrollbar);

        //##################
        //###CSV LOADER###
        //##################

        if(ImGui::Button("Load CSV##csv_profiles")){
            ImVec2 button_pos = ImGui::GetItemRectMin();
            ImVec2 button_size = ImGui::GetItemRectSize();
            button_pos.y += (button_size.y + 20);
            ImGui::SetNextWindowPos(button_pos, ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(700, 300), ImGuiCond_Always);

            ImGui::OpenPopup("CSV Loader");
        }
        tooltip(str8_literal("Load CSV using a profile."));

        //ImGui::BeginChild("Child5", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.45), true, 0);
        //ImGui::BeginChild("Child6", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.85), true, 0);
        //ImGui::BeginChild("Child7", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);

        if(ImGui::BeginPopupModal("CSV Loader", 0, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove)){
            f32 width = ImGui::CalcTextSize("Description").x + 15;
            String8 unique_fmt;

            //CSV_Profile* profile;
            if(pm->csv_profile_count == 0){
                if(pm->csv_profile_count < MAX_PROFILE_COUNT){
                    CSV_Profile* profile = (CSV_Profile*)pool_next(pm->csv_profile_pool);
                    dll_push_back_old(pm->csv_profiles, profile);
                    pm->csv_profile_count++;
                    pm->csv_profile_idx = 0;
                }
                else{
                    // todo(rr): STATUSBAR ERROR
                }
            }

            // go to profile at idx
            CSV_Profile* profile = pm->csv_profiles->next;
            for(s32 i=0; i < pm->csv_profile_idx; ++i){
                profile = profile->next;
            }
            pm->csv_profile = profile;

            String8 csv_path = str8_cstring(pm->csv_path);
            test_csv_against_profile(csv_path);

            ImGui::SeparatorText("Profile");
            {
                ImGui::PushFont(my_font12);
                fmt = str8_fmt(tm->frame_arena, "%c##previous_profile", icon_lookup[Icon_LeftArrow]);
                if(ImGui::Button((char*)fmt.str)){
                    pm->csv_profile_idx = wrap_index(pm->csv_profile_idx - 1, pm->csv_profile_count);
                }
                ImGui::PopFont();
                tooltip(str8_literal("Go to previous profile."));

                ImGui::SameLine();
                ImGui::PushFont(my_font12);
                fmt = str8_fmt(tm->frame_arena, "%c##next_profile", icon_lookup[Icon_RightArrow]);
                if(ImGui::Button((char*)fmt.str)){
                    pm->csv_profile_idx = wrap_index(pm->csv_profile_idx + 1, pm->csv_profile_count);
                }
                ImGui::PopFont();
                tooltip(str8_literal("Go to next profile."));

                ImGui::SameLine();
                if(ImGui::Button("+##add_profile")){
                    if(pm->csv_profile_count < MAX_PROFILE_COUNT){
                        profile = (CSV_Profile*)pool_next(pm->csv_profile_pool);
                        dll_push_back_old(pm->csv_profiles, profile);
                        ++pm->csv_profile_count;
                        pm->csv_profile_idx = pm->csv_profile_count - 1;
                    }
                    else{
                        // todo(rr): STATUSBAR ERROR
                    }
                }
                tooltip(str8_literal("Add new profile."));

                ImGui::SameLine();
                if(ImGui::Button("x##delete_profile")){
                    if(pm->csv_profile_count > 0){
                        dll_remove_old(profile);
                        pool_free(pm->csv_profile_pool, profile);

                        --pm->csv_profile_count;
                        if(pm->csv_profile_idx == pm->csv_profile_count){
                            --pm->csv_profile_idx;
                        }
                        if(pm->csv_profile_count == 0){
                            pool_free_all(pm->csv_profile_pool);
                            pm->csv_profiles = (CSV_Profile*)pool_next(pm->csv_profile_pool);
                            dll_clear(pm->csv_profiles);
                        }
                    }
                }
                tooltip(str8_literal("Delete profile."));


                ImGui::SameLine();
                ImGui::Text("Name");
                ImGui::SameLine();
                ImGui::PushItemWidth(100);
                unique_fmt = str8_fmt(tm->frame_arena, "##profile_name%i\n", pm->csv_profile_idx);
                ImGui::InputText((char*)unique_fmt.data, profile->name, PROFILE_NAME_SIZE);
                ImGui::PopItemWidth();

                ImGui::SameLine();
                ImGui::Text("(%i / %i)", pm->csv_profile_idx + 1, pm->csv_profile_count);
            }
            ImGui::SeparatorText("Headers");

            ImGui::Text("Date");
            ImGui::SameLine();
            ImGui::SetCursorPosX(width);
            unique_fmt = str8_fmt(tm->frame_arena, "##profile_date_name%i\n", pm->csv_profile_idx);

            ImGui::InputText((char*)unique_fmt.data, profile->date, PROFILE_DATE_SIZE);

            ImGui::SameLine();
            if(pm->date_header_found){
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(GREEN.r, GREEN.g, GREEN.b, 255));
                ImGui::PushFont(my_font20);
                f32 currentY = ImGui::GetCursorPosY();
                ImGui::SetCursorPosY(currentY - 3.0f);
                ImGui::Text("%c", icon_lookup[Icon_Square]);
                ImGui::SetCursorPosY(currentY);
                ImGui::PopFont();
                ImGui::PopStyleColor(); // Restore default color
                ImGui::SameLine();
                ImGui::Text("Found");
            }
            else{
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
                ImGui::PushFont(my_font20);
                f32 currentY = ImGui::GetCursorPosY();
                ImGui::SetCursorPosY(currentY - 3.0f);
                ImGui::Text("%c", icon_lookup[Icon_Square]);
                ImGui::SetCursorPosY(currentY);
                ImGui::PopFont();
                ImGui::PopStyleColor(); // Restore default color
                ImGui::SameLine();
                ImGui::Text("Not Found");

            }
            if (!pm->date_header_found) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.3f); // Reduce transparency
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // Darker background
            }
            ImGui::BeginDisabled(!pm->date_header_found);
            ImGui::SetCursorPosX(width);
            s32 item_current_idx = 0;
            unique_fmt = str8_fmt(tm->frame_arena, "##date_format%i\n", pm->csv_profile_idx);
            if(ImGui::BeginCombo((char*)unique_fmt.data, profile->date_format, ImGuiComboFlags_HeightLarge)){
                for(s32 format_kind = 0; format_kind < array_count(date_formats); ++format_kind){
                    bool is_selected = (item_current_idx == format_kind);
                    String8 format = date_formats[format_kind];

                    if (ImGui::Selectable((char*)date_formats[format_kind].data, is_selected)){
                        item_current_idx = format_kind;
                        profile->date_format_kind = format_kind;
                        memcpy(profile->date_format, format.str, format.size + 1);
                        pm->new_file_or_format = true;
                    }

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            if(pm->date_format_found){
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(GREEN.r, GREEN.g, GREEN.b, 255));
                ImGui::PushFont(my_font20);
                f32 currentY = ImGui::GetCursorPosY();
                ImGui::SetCursorPosY(currentY - 3.0f);
                ImGui::Text("%c", icon_lookup[Icon_Square]);
                ImGui::SetCursorPosY(currentY);
                ImGui::PopFont();
                ImGui::PopStyleColor(); // Restore default color
                ImGui::SameLine();
                ImGui::Text("Found");
            }
            else{
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
                ImGui::PushFont(my_font20);
                f32 currentY = ImGui::GetCursorPosY();
                ImGui::SetCursorPosY(currentY - 3.0f);
                ImGui::Text("%c", icon_lookup[Icon_Square]);
                ImGui::SetCursorPosY(currentY);
                ImGui::PopFont();
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::Text("Not Found");
            }
            ImGui::EndDisabled();
            if (!pm->date_header_found) {
                ImGui::PopStyleColor(); // Restore original background
                ImGui::PopStyleVar();   // Restore original alpha
            }
            custom_separator();

            ImGui::Text("Amount");
            ImGui::SameLine();
            ImGui::SetCursorPosX(width);
            unique_fmt = str8_fmt(tm->frame_arena, "##profile_amount_name%i\n", pm->csv_profile_idx);
            ImGui::InputText((char*)unique_fmt.data, profile->amount, PROFILE_AMOUNT_SIZE);
            ImGui::SameLine();
            if(pm->amount_header_found){
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(GREEN.r, GREEN.g, GREEN.b, 255));
                ImGui::PushFont(my_font20);
                f32 currentY = ImGui::GetCursorPosY();
                ImGui::SetCursorPosY(currentY - 3.0f);
                ImGui::Text("%c", icon_lookup[Icon_Square]);
                ImGui::SetCursorPosY(currentY);
                ImGui::PopFont();
                ImGui::PopStyleColor(); // Restore default color
                ImGui::SameLine();
                ImGui::Text("Found");
            }
            else{
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
                ImGui::PushFont(my_font20);
                f32 currentY = ImGui::GetCursorPosY();
                ImGui::SetCursorPosY(currentY - 3.0f);
                ImGui::Text("%c", icon_lookup[Icon_Square]);
                ImGui::SetCursorPosY(currentY);
                ImGui::PopFont();
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::Text("Not Found");
            }
            custom_separator();

            ImGui::Text("Description");
            ImGui::SameLine();
            ImGui::SetCursorPosX(width);
            unique_fmt = str8_fmt(tm->frame_arena, "##profile_description_name%i\n", pm->csv_profile_idx);
            ImGui::InputText((char*)unique_fmt.data, profile->description, PROFILE_DESCRIPTION_SIZE);
            ImGui::SameLine();
            if(pm->description_header_found){
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(GREEN.r, GREEN.g, GREEN.b, 255));
                ImGui::PushFont(my_font20);
                f32 currentY = ImGui::GetCursorPosY();
                ImGui::SetCursorPosY(currentY - 3.0f);
                ImGui::Text("%c", icon_lookup[Icon_Square]);
                ImGui::SetCursorPosY(currentY);
                ImGui::PopFont();
                ImGui::PopStyleColor(); // Restore default color
                ImGui::SameLine();
                ImGui::Text("Found");
            }
            else{
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(RED.r, RED.g, RED.b, 255));
                ImGui::PushFont(my_font20);
                f32 currentY = ImGui::GetCursorPosY();
                ImGui::SetCursorPosY(currentY - 3.0f);
                ImGui::Text("%c", icon_lookup[Icon_Square]);
                ImGui::SetCursorPosY(currentY);
                ImGui::PopFont();
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::Text("Not Found");
            }

            //ImGui::SeparatorText("Date Format");

            custom_separator();
            ImGui::Text("File");
            ImGui::SameLine();
            ImGui::SetCursorPosX(width);
            if(ImGui::Button("...##grab_csv_file")){
                char* file = tinyfd_openFileDialog("Open CSV File", (char*)pm->default_path.str, 0, 0, 0, 0);
                if(file){
                    s32 len = char_length(file);
                    memset(pm->csv_path, 0, 4096);
                    memcpy(pm->csv_path, file, len);
                    pm->new_file_or_format = true;
                }
            }
            tooltip(str8_literal("Select CSV file from disk."));
            ImGui::SameLine();
            ImGui::PushItemWidth(417);
            unique_fmt = str8_fmt(tm->frame_arena, "##profile_description_name%i\n", pm->csv_profile_idx);
            ImGui::InputText("##csv_path", pm->csv_path, PROFILE_DESCRIPTION_SIZE, ImGuiInputTextFlags_ReadOnly);
            ImGui::SameLine();
            if(ImGui::Button("x##delete_csv_path")){
                memset(pm->csv_path, 0, 4096);
            }
            tooltip(str8_literal("Delete path."));

            custom_separator();
            if(controller_button_pressed(KeyCode_ESCAPE, true)){
                ImGui::CloseCurrentPopup();
            }
            if(ImGui::Button("Load")){
                deserialize_csv(csv_path);
                ImGui::CloseCurrentPopup();
            }
            tooltip(str8_literal("loading CSV."));

            ImGui::SameLine();
            if(ImGui::Button("Cancel")){
                ImGui::CloseCurrentPopup();
            }
            tooltip(str8_literal("Exit window without loading CSV."));
            ImGui::EndPopup();
        }

        ImGui::SameLine();
        if(ImGui::Button("Merchants List##csv_profiles")){
            const ImGuiViewport* vp = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(vp->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.75f));
            ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_Always);
            ImGui::OpenPopup("Merchants");
        }
        tooltip(str8_literal("View merchants list."));

        if(ImGui::BeginPopupModal("Merchants", 0, ImGuiWindowFlags_NoScrollbar)){
            if(ImGui::BeginTabBar("##Month", ImGuiTabBarFlags_None)){

                ImGui::PushStyleColor(ImGuiCol_TabActive, active_color);
                ImGui::PushStyleColor(ImGuiCol_TabHovered, hover_color);

                if(ImGui::BeginTabItem("All")){
                    show_all_merchants = true;
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Hidden")){
                    show_all_merchants = false;
                    ImGui::EndTabItem();
                }

                ImGui::PopStyleColor(2);
                ImGui::EndTabBar();
            }

            static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;
            ImVec2 outer_size = ImVec2(0.0f, ImGui::GetContentRegionAvail().y - 25);
            if(ImGui::BeginTable("merchant_table", 3, flags, outer_size)){
                ImGui::TableSetupScrollFreeze(0, 1);

                ImGui::TableSetupColumn("Description");
                ImGui::TableSetupColumn("Category");
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_NoHeaderLabel);

                // Note: Populate table header.
                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                for(s32 column = 0; column < 3; ++column){
                    ImGui::TableSetColumnIndex(column);
                    ImGui::TableHeader(ImGui::TableGetColumnName(column));
                    //if(column < 2){
                    //    ImGui::TableHeader(ImGui::TableGetColumnName(column));
                    //}
                    //else{
                    //    // delet all
                    //    {
                    //        if(ImGui::Button("x##delete_all_merchants")){
                    //            for(Merchant* merch = pm->merchants; merch != 0; merch = merch->next){
                    //            }
                    //        }
                    //        tooltip(str8_literal("Delete All Merchants."));
                    //    }
                    //}
                }

                // Note: Sort columns when clicking on headers
                ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs();
                if(sort_specs && sort_specs->SpecsDirty){
                    for(s32 n = 0; n < sort_specs->SpecsCount; n++){
                        const ImGuiTableColumnSortSpecs* spec = &sort_specs->Specs[n];

                        if(spec->ColumnIndex == 0){ // Description.
                            bool ascending = (spec->SortDirection == ImGuiSortDirection_Ascending);
                            dll_bubble_sort_description_merch(&pm->merchants, ascending);
                        }
                        if(spec->ColumnIndex == 1){ // Category.
                            bool ascending = (spec->SortDirection == ImGuiSortDirection_Ascending);
                            dll_bubble_sort_category_merch(&pm->merchants, ascending);
                        }
                    }
                    sort_specs->SpecsDirty = false;
                }

                // Note: Populate rows with transactions.
                for(Merchant* merch = pm->merchants; merch != 0; merch = merch->next){
                    if(show_all_merchants && merch->hidden){
                        continue;
                    }
                    if(!show_all_merchants && !merch->hidden){
                        continue;
                    }

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", merch->description);

                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(category_group_select_column_width);

                    // note: make selection box not transparent
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetColorU32(combo_popup_background_color));

                    // note: color selection red if not found in category_group names
                    ImVec4 frame_bg_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
                    bool found = false;
                    char space[] = " ";
                    if(!char_compare(merch->category, space)){
                        CategoryGroup* category_group = pm->month_category_groups;
                        for(s32 c_idx = 0; c_idx < pm->category_groups_count && !found; ++c_idx){
                            category_group = category_group->next;

                            Category* category = category_group->categories;
                            for(s32 r_idx = 0; r_idx < category_group->category_count && !found; ++r_idx){
                                category = category->next;
                                String8 merch_category = str8_cstring(merch->category);

                                String8 cat_group_part = str8_format(tm->frame_arena, "%s: ", category_group->name);
                                String8 cat_part = str8(category->name, char_length(category->name));
                                String8 cat = str8_concatenate(tm->frame_arena, cat_group_part, cat_part);

                                if(cat.count == merch_category.count){
                                    if(str8_compare(cat, merch_category)){
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

                    // todo(rr): leave comments on this combo box explaing some things.
                    //           I don't remember why I did some of this stuff.
                    // note: populate selection box with options.
                    //bool selected = false;
                    s32 color_idx = 0;
                    fmt = str8_fmt(scratch.arena, "##merchant_group_select%i", merch->id);
                    if(ImGui::BeginCombo((char*)fmt.data, merch->category, ImGuiComboFlags_HeightLarge)){
                        for(int n = 0; n < pm->category_list_count; n++){
                            String8 selection_item = pm->category_list[n];

                            ImDrawList* draw_list = ImGui::GetWindowDrawList();
                            ImVec2 min = ImGui::GetCursorScreenPos();
                            ImVec2 max = ImVec2(min.x + ImGui::GetContentRegionAvail().x, min.y + ImGui::GetTextLineHeightWithSpacing());

                            if(n != 0){
                                if(last_combo_name.size == 0){
                                    last_combo_name = selection_item;
                                    draw_list->AddRectFilled(min, max, combo_popup_alternating_colors[color_idx % 2]);
                                }
                                else{
                                    //s64 idx = str8_index_from_left(last_combo_name, ':');
                                    String8List split_node1 = str8_split(tm->frame_arena, last_combo_name, ':', 0);
                                    String8List split_node2 = str8_split(tm->frame_arena, selection_item, ':', 0);
                                    if(!str8_compare(split_node1.first->string, split_node2.first->string)){
                                        ++color_idx;
                                    }
                                    last_combo_name = selection_item;
                                    draw_list->AddRectFilled(min, max, combo_popup_alternating_colors[color_idx % 2]);
                                }

                                if(selection_item.size == 0){
                                    continue;
                                }
                            }

                            // todo: doint this merch->category multiple times
                            String8 merch_category = str8_cstring(merch->category);
                            const bool is_selected = str8_compare(selection_item, merch_category);
                            if(ImGui::Selectable((char*)selection_item.str, is_selected)){
                                memcpy(merch->category, selection_item.str, selection_item.count);
                                //merch->category.count = selection_item.count;
                                //merch->category.str[merch->category.count] = '\0';
                                apply_new_category = true;
                            }

                            if(is_selected){
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::PopStyleColor(2);
                    ImGui::PopItemWidth();

                    ImGui::TableNextColumn();

                    ImGui::PushFont(my_font12);
                    if(show_all_merchants){
                        fmt = str8_fmt(tm->frame_arena, "%c##show_transaction%i", icon_lookup[Icon_Hide], merch->id);
                    }
                    else{
                        fmt = str8_fmt(tm->frame_arena, "%c##show_transaction%i", icon_lookup[Icon_Show], merch->id);
                    }
                    if(ImGui::Button((char*)fmt.str)){
                        merch->hidden = !merch->hidden;
                        apply_hidden_transactions = true;
                    }
                    ImGui::PopFont();
                    if(show_all_merchants){
                        tooltip(str8_literal("Hide merchant from budget."));
                    }
                    else{
                        tooltip(str8_literal("Show merchant in budget."));
                    }
                }
                ImGui::EndTable();
            }

            ImGui::Spacing();
            if(ImGui::Button("Exit")){
                ImGui::CloseCurrentPopup();
            }
            tooltip(str8_literal("Exit window."));

            ImGui::EndPopup();
        }

        //##################
        //###TRANSACTIONS###
        //##################

        ImGui::SeparatorText("Transactions");
        ImGui::PushFont(my_font12);
        fmt = str8_fmt(tm->frame_arena, "%c##prev_prev_year", icon_lookup[Icon_DoubleLeftArrow]);
        if(ImGui::Button((char*)fmt.str)){
            pm->year_idx -= 5;
            pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
        }
        ImGui::PopFont();
        tooltip(str8_literal("Go To Previous 5 Years"));
        ImGui::SameLine();
        ImGui::PushFont(my_font12);
        fmt = str8_fmt(tm->frame_arena, "%c##prev_year", icon_lookup[Icon_LeftArrow]);
        if(ImGui::Button((char*)fmt.str)){
            --pm->year_idx;
            pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
        }
        ImGui::PopFont();
        tooltip(str8_literal("Go To Previous Year"));
        ImGui::SameLine();
        ImGui::Text("Year %04d", year->number);
        ImGui::SameLine();
        ImGui::PushFont(my_font12);
        fmt = str8_fmt(tm->frame_arena, "%c##next_year", icon_lookup[Icon_RightArrow]);
        if(ImGui::Button((char*)fmt.str)){
            ++pm->year_idx;
            pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
        }
        ImGui::PopFont();
        tooltip(str8_literal("Go To Next Year"));
        ImGui::SameLine();
        ImGui::PushFont(my_font12);
        fmt = str8_fmt(tm->frame_arena, "%c##next_next_year", icon_lookup[Icon_DoubleRightArrow]);
        if(ImGui::Button((char*)fmt.str)){
            pm->year_idx += 5;
            pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
        }
        ImGui::PopFont();
        tooltip(str8_literal("Go To Next 5 Years"));
        ImGui::SameLine();
        ImGui::PushFont(my_font12);
        fmt = str8_fmt(tm->frame_arena, "%c##current_year", icon_lookup[Icon_UpArrow]);
        if(ImGui::Button((char*)fmt.str)){
            pm->year_idx = MAX_YEAR_COUNT/2;
            pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
        }
        ImGui::PopFont();
        tooltip(str8_literal("Go To Current Year"));
        ImGui::SameLine();
        if(ImGui::Button("x##delete_current_year")){
            ImGui::OpenPopup("delete current year transactions");
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }
        if(ImGui::BeginPopupModal("delete current year transactions", 0,
                                  ImGuiWindowFlags_AlwaysAutoResize|
                                  ImGuiWindowFlags_NoMove)){
            ImGui::Text("Are you sure you want to delete all\ntransactions for the selected year?");
            if(ImGui::Button("Confirm##confirm_delete_current_year")){
                for(s32 i=0; i<Month_Count; ++i){
                    MonthInfo* month = year->months + i;
                    Transaction* t = month->transactions;
                    for(s32 t_idx=0; t_idx < month->transaction_count; ++t_idx){
                        t = t->next;
                        dll_remove_old(t);
                        pool_free(pm->transaction_pool, t);
                        t = month->transactions;
                    }
                    dll_clear(month->transactions);
                    month->transaction_count = 0;
                }
                pm->total_transaction_count -= year->transaction_count;
                year->transaction_count = 0;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 58);
            if(ImGui::Button("Cancel##cancel_delete_current_year")){
                ImGui::CloseCurrentPopup();
            }
            if(controller_button_pressed(KeyCode_ESCAPE, true)){
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        tooltip(str8_literal("Delete all transactions for the selected year"));

        ImGui::SameLine();
        if(ImGui::Button("xx##delete_all_years")){
            ImGui::OpenPopup("delete all transactions");
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }
        if(ImGui::BeginPopupModal("delete all transactions", 0,
                                  ImGuiWindowFlags_AlwaysAutoResize|
                                  ImGuiWindowFlags_NoMove)){
            ImGui::Text("Are you sure you want to delete all\ntransactions for all years?");
            if(ImGui::Button("Confirm##confirm_delete_current_year")){
                for(s32 i=0; i<MAX_YEAR_COUNT; ++i){
                    Year* y = pm->years + i;
                    for(s32 i=0; i<Month_Count; ++i){
                        MonthInfo* month = y->months + i;
                        Transaction* t = month->transactions;
                        for(s32 t_idx=0; t_idx < month->transaction_count; ++t_idx){
                            t = t->next;
                            dll_remove_old(t);
                            pool_free(pm->transaction_pool, t);
                            t = month->transactions;
                        }
                        dll_clear(month->transactions);
                        month->transaction_count = 0;
                    }
                    y->transaction_count = 0;
                }
                pm->total_transaction_count = 0;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 58);
            if(ImGui::Button("Cancel##cancel_delete_current_year")){
                ImGui::CloseCurrentPopup();
            }
            if(controller_button_pressed(KeyCode_ESCAPE, true)){
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        tooltip(str8_literal("Delete all transactions for all the year"));

        if(ImGui::BeginTabBar("##Month", ImGuiTabBarFlags_None)){

            ImGui::PushStyleColor(ImGuiCol_TabActive, active_color);
            ImGui::PushStyleColor(ImGuiCol_TabHovered, hover_color);

            if(ImGui::BeginTabItem("January", 0, pm->month_tab_flags[0])){
                pm->month_tab_idx = Month_Jan;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("February", 0, pm->month_tab_flags[1])){
                pm->month_tab_idx = Month_Feb;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("March", 0, pm->month_tab_flags[2])){
                pm->month_tab_idx = Month_Mar;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("April", 0, pm->month_tab_flags[3])){
                pm->month_tab_idx = Month_Apr;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("May", 0, pm->month_tab_flags[4])){
                pm->month_tab_idx = Month_May;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("June", 0, pm->month_tab_flags[5])){
                pm->month_tab_idx = Month_Jun;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("July", 0, pm->month_tab_flags[6])){
                pm->month_tab_idx = Month_Jul;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("August", 0, pm->month_tab_flags[7])){
                pm->month_tab_idx = Month_Aug;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("September", 0, pm->month_tab_flags[8])){
                pm->month_tab_idx = Month_Sep;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("October", 0, pm->month_tab_flags[9])){
                pm->month_tab_idx = Month_Oct;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("November", 0, pm->month_tab_flags[10])){
                pm->month_tab_idx = Month_Nov;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("December", 0, pm->month_tab_flags[11])){
                pm->month_tab_idx = Month_Dec;
                ImGui::EndTabItem();
            }

            ImGui::PopStyleColor(2);
            ImGui::EndTabBar();
        }

        ImGui::Spacing();
        ImGui::Spacing();

        fmt = str8_fmt(scratch.arena, "Count: %i", month->transaction_visible_count);
        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;
        ImVec2 outer_size = ImVec2(0.0f, ImGui::GetContentRegionAvail().y);
        if(ImGui::BeginTable("transactions_table", 6, flags, outer_size)){

            // note: Make top row always visible when scrolling.
            ImGui::TableSetupScrollFreeze(0, 1);

            ImGui::TableSetupColumn((char*)fmt.str);
            ImGui::TableSetupColumn("Date");
            ImGui::TableSetupColumn("Amount");
            ImGui::TableSetupColumn("Description");
            ImGui::TableSetupColumn("Category");
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_NoHeaderLabel);

            // note: Populate table header.
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            for(s32 column = 0; column < 6; ++column){
                ImGui::TableSetColumnIndex(column);
                if(column < 5){
                    ImGui::TableHeader(ImGui::TableGetColumnName(column));
                }
                else{
                    // lock all
                    {
                        ImGui::PushID(100001);
                        if(month->locked){
                            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(RED.r, RED.g, RED.b, 255));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
                        }
                        else{
                            ImGui::PushStyleColor(ImGuiCol_Button, pm->default_button_color);
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pm->default_button_hovered_color);
                        }

                        ImGui::PushFont(my_font12);
                        if(month->locked){
                            fmt = str8_fmt(tm->frame_arena, "%c##lock_all", icon_lookup[Icon_Locked]);
                        }
                        else{
                            fmt = str8_fmt(tm->frame_arena, "%c##lock_all", icon_lookup[Icon_Unlocked]);
                        }
                        if(ImGui::Button((char*)fmt.str, ImVec2(lock_button_width, 0.0f))){
                            month->locked = !month->locked;

                            if(month->locked){
                                Transaction* trans = month->transactions;
                                for(s32 t_idx = 0; t_idx < month->transaction_count; ++t_idx){
                                    trans = trans->next;
                                    trans->locked = true;
                                }
                            }
                            else{
                                apply_new_category = true;

                                Transaction* trans = month->transactions;
                                for(s32 t_idx = 0; t_idx < month->transaction_count; ++t_idx){
                                    trans = trans->next;
                                    trans->locked = false;
                                }
                            }
                        }
                        ImGui::PopStyleColor(2);
                        ImGui::PopID();
                        ImGui::PopFont();
                        tooltip(str8_literal("Lock/Unlock All Transactions."));
                    }
                    {
                        ImGui::SameLine();
                        ImGui::PushFont(my_font12);
                        fmt = str8_fmt(tm->frame_arena, "%c##hide_all_transaction", icon_lookup[Icon_Hide]);
                        if(ImGui::Button((char*)fmt.str)){
                            Transaction* trans = month->transactions;
                            for(s32 t_idx = 0; t_idx < month->transaction_count; ++t_idx){
                                trans = trans->next;

                                String8 trans_description = str8_cstring(trans->description);
                                for(Merchant* m = pm->merchants; m != 0; m = m->next){
                                    String8 m_description = str8_cstring(m->description);
                                    if(str8_compare(m_description, trans_description)){
                                        m->hidden = true;
                                        apply_hidden_transactions = true;
                                        break;
                                    }
                                }
                            }
                        }
                        ImGui::PopFont();
                        tooltip(str8_literal("Hide all merchants in month from budget."));
                    }

                    // mute all
                    {
                        ImGui::SameLine();
                        ImGui::PushID(100000);
                        if(month->muted){
                            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(RED.r, RED.g, RED.b, 255));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.1f, 0.0f, 1.0f));
                        }
                        else{
                            ImGui::PushStyleColor(ImGuiCol_Button, pm->default_button_color);
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pm->default_button_hovered_color);
                        }
                        if(ImGui::Button("m##mute_month123")){
                            month->muted = !month->muted;
                            if(month->muted){
                                Transaction* trans = month->transactions;
                                for(s32 t_idx = 0; t_idx < month->transaction_count; ++t_idx){
                                    trans = trans->next;
                                    trans->muted = true;
                                }
                            }
                            else{
                                Transaction* trans = month->transactions;
                                for(s32 t_idx = 0; t_idx < month->transaction_count; ++t_idx){
                                    trans = trans->next;
                                    trans->muted = false;
                                }
                            }
                        }
                        tooltip(str8_literal("Mute All Transactions."));
                        ImGui::PopStyleColor(2);
                        ImGui::PopID();
                    }

                    // delet all
                    {
                        ImGui::SameLine();
                        if(ImGui::Button("x##delete_all_transactions123")){
                            Transaction* t = month->transactions;
                            for(s32 t_idx=0; t_idx < month->transaction_count; ++t_idx){
                                t = t->next;
                                dll_remove_old(t);
                                pool_free(pm->transaction_pool, t);
                                t = month->transactions;
                            }
                            dll_clear(month->transactions);
                            pm->total_transaction_count -= month->transaction_count;
                            year->transaction_count -= month->transaction_count;
                            month->transaction_count = 0;
                        }
                        tooltip(str8_literal("Delete All Transactions."));
                    }

                    // add transaction
                    {
                        ImGui::SameLine();
                        if(ImGui::Button("+##add_transaction_button")){
                            if(pm->total_transaction_count < MAX_TRANSACTION_COUNT){
                                Transaction* trans = (Transaction*)pool_next(pm->transaction_pool);
                                dll_push_back_old(month->transactions, trans);

                                if(month->transaction_count == 0){
                                    fmt = str8_fmt(scratch.arena, "01/01/%04d", year->number);
                                    memcpy((void*)trans->date, (void*)fmt.str, fmt.size);
                                }
                                else{
                                    Transaction* last = trans->prev;
                                    s32 date_length = (s32)char_length(last->date);
                                    String8 date_str8 = str8(last->date, date_length);
                                    String8List parts = str8_split(scratch.arena, date_str8, '/', 0);
                                    parts.last->string = str8_fmt(scratch.arena, "%04d", year->number);

                                    String8Join join = {0};
                                    join.mid = str8_literal("/");
                                    String8 result = str8_join(scratch.arena, &parts, &join);
                                    memcpy((void*)trans->date, (void*)result.str, result.count);
                                }
                                memcpy((void*)trans->category, (void*)pm->category_list->str, pm->category_list->size);

                                ++month->transaction_count;
                                ++year->transaction_count;
                                ++pm->total_transaction_count;
                            }
                        }
                        tooltip(str8_literal("Add Transaction."));
                    }

                }
            }

            // Note: Sort columns when clicking on headers
            ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs();
            if(sort_specs && sort_specs->SpecsDirty){
                for(s32 n = 0; n < sort_specs->SpecsCount; n++){
                    const ImGuiTableColumnSortSpecs* spec = &sort_specs->Specs[n];

                    if(spec->ColumnIndex == 1){ // Date.
                        bool ascending = (spec->SortDirection == ImGuiSortDirection_Ascending);
                        dll_bubble_sort_date(month->transactions, ascending);
                    }
                    if(spec->ColumnIndex == 2){ // Amount.
                        bool ascending = (spec->SortDirection == ImGuiSortDirection_Ascending);
                        dll_bubble_sort_amount(month->transactions, ascending);
                    }
                    if(spec->ColumnIndex == 3){ // Description.
                        bool ascending = (spec->SortDirection == ImGuiSortDirection_Ascending);
                        dll_bubble_sort_description(month->transactions, ascending);
                    }
                    if(spec->ColumnIndex == 4){ // Category.
                        bool ascending = (spec->SortDirection == ImGuiSortDirection_Ascending);
                        dll_bubble_sort_category(month->transactions, ascending);
                    }
                }
                sort_specs->SpecsDirty = false;
            }

            // Note: Populate rows with transactions.
            s32 idx = 0;
            for(Transaction* trans = month->transactions->next; trans != month->transactions; trans = trans->next){

                // note slow: popluate empty amount's in transactions with 0's for visual appeal
                if(trans->amount[0] == 0){
                    trans->amount[0] = '0';
                    trans->amount[1] = '\0';
                }

                if(trans->hidden){
                    continue;
                }
                idx++;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                //ImGui::PushID(idx);
                fmt = str8_fmt(scratch.arena, "%i", idx);
                ImGui::Text((char*)fmt.data);
                //ImGui::PopID();
                //if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)){
                //    ImGui::SetDragDropPayload("DRAG_CATEGORY", &idx, sizeof(s32));
                //    ImGui::Text("%i", idx);
                //    ImGui::EndDragDropSource();
                //}
                //tooltip(str8_literal("Drag/Swap Transaction."));
                //if(ImGui::BeginDragDropTarget()){
                //    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_CATEGORY")){
                //        s32* payload_data = (s32*)payload->Data;
                //        s32 from_idx = *payload_data;
                //        if(from_idx != idx){
                //            Transaction* t = month->transactions;
                //            for(s32 i=0; i <= from_idx; ++i){
                //                t = t->next;
                //            }
                //            dll_swap(t, trans, Transaction);
                //        }
                //    }
                //    ImGui::EndDragDropTarget();
                //}

                ImGui::TableNextColumn();
                ImGui::Text("%s", trans->date);

                ImGui::TableNextColumn();
                ImGui::Text("%s", trans->amount);

                ImGui::TableNextColumn();
                ImGui::Text("%s", trans->description);

                ImGui::TableNextColumn();
                {
                    ImGui::PushItemWidth(category_group_select_column_width);

                    // note: make selection box not transparent
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetColorU32(combo_popup_background_color));

                    // note: color selection red if not found in category_group names
                    ImVec4 frame_bg_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
                    bool found = false;
                    char space[] = " ";
                    if(!char_compare(trans->category, space)){
                        CategoryGroup* category_group = pm->month_category_groups;
                        for(s32 c_idx = 0; c_idx < pm->category_groups_count && !found; ++c_idx){
                            category_group = category_group->next;

                            Category* category = category_group->categories;
                            for(s32 r_idx = 0; r_idx < category_group->category_count && !found; ++r_idx){
                                category = category->next;
                                String8 trans_selection = str8_cstring(trans->category);

                                String8 cat_group_part = str8_format(tm->frame_arena, "%s: ", category_group->name);
                                String8 cat_part = str8(category->name, char_length(category->name));
                                String8 cat = str8_concatenate(tm->frame_arena, cat_group_part, cat_part);

                                if(cat.count == trans_selection.count){
                                    if(str8_compare(cat, trans_selection)){
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

                    // todo(rr): leave comments on this combo box explaing some things.
                    //           I don't remember why I did some of this stuff.
                    // note: populate selection box with options.
                    //bool selected = false;
                    s32 color_idx = 0;
                    fmt = str8_fmt(scratch.arena, "##category_group_select%i", idx);
                    if(ImGui::BeginCombo((char*)fmt.data, trans->category, ImGuiComboFlags_HeightLarge)){
                        for(int n = 0; n < pm->category_list_count; n++){
                            String8 selection_item = pm->category_list[n];
                            //String8 trans_selection = str8(trans->category, char_length(trans->category));
                            String8 trans_selection = str8_cstring(trans->category);

                            ImDrawList* draw_list = ImGui::GetWindowDrawList();
                            ImVec2 min = ImGui::GetCursorScreenPos();
                            ImVec2 max = ImVec2(min.x + ImGui::GetContentRegionAvail().x, min.y + ImGui::GetTextLineHeightWithSpacing());

                            if(n != 0){
                                if(last_combo_name.size == 0){
                                    last_combo_name = selection_item;
                                    draw_list->AddRectFilled(min, max, combo_popup_alternating_colors[color_idx % 2]);
                                }
                                else{
                                    //s64 idx = str8_index_from_left(last_combo_name, ':');
                                    String8List split_node1 = str8_split(tm->frame_arena, last_combo_name, ':', 0);
                                    String8List split_node2 = str8_split(tm->frame_arena, selection_item, ':', 0);
                                    if(!str8_compare(split_node1.first->string, split_node2.first->string)){
                                        ++color_idx;
                                    }
                                    last_combo_name = selection_item;
                                    draw_list->AddRectFilled(min, max, combo_popup_alternating_colors[color_idx % 2]);
                                }

                                if(selection_item.size == 0){
                                    continue;
                                }
                            }

                            const bool is_selected = str8_compare(selection_item, trans_selection);
                            if(ImGui::Selectable((char*)selection_item.str, is_selected)){
                                String8 a = str8_lit("a");
                                String8 b = str8_lit("b\0");
                                memcpy((void*)trans->category, (void*)selection_item.str, selection_item.size + 1);
                                if(!trans->locked){
                                    //consider: Why even have this, just have the operation here and be done with it.
                                    //selected = true;

                                    // Note: Apply selection to merchnat.
                                    String8 trans_description = str8_cstring(trans->description);
                                    String8 trans_category = str8_cstring(trans->category);
                                    for(Merchant* merch = pm->merchants; merch != 0; merch = merch->next){
                                        String8 m_description = str8_cstring(merch->description);
                                        if(str8_compare(m_description, trans_description)){
                                            memcpy(merch->category, trans_category.str, trans_category.count);
                                            //merch->category.count = trans_category.count;
                                            //merch->category.str[merch->category.count] = '\0';
                                            break;
                                        }
                                    }
                                    apply_new_category = true;
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::PopStyleColor(2);
                    ImGui::PopItemWidth();
                    //if(selected){
                    //}
                }
                {
                    ImGui::TableNextColumn();

                    if(trans->locked){
                        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(RED.r, RED.g, RED.b, 255));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
                    }
                    else{
                        ImGui::PushStyleColor(ImGuiCol_Button, pm->default_button_color);
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pm->default_button_hovered_color);
                    }

                    ImGui::PushFont(my_font12);
                    if(trans->locked){
                        fmt = str8_fmt(tm->frame_arena, "%c##lock_category%i", icon_lookup[Icon_Locked], idx);
                    }
                    else{
                        fmt = str8_fmt(tm->frame_arena, "%c##lock_category%i", icon_lookup[Icon_Unlocked], idx);
                    }
                    if(ImGui::Button((char*)fmt.str, ImVec2(lock_button_width, 0.0f))){
                        trans->locked = !trans->locked;
                        if(!trans->locked){
                            apply_new_category = true;
                        }
                    }
                    ImGui::PopStyleColor(2);
                    ImGui::PopFont();
                    if(trans->locked){
                        tooltip(str8_literal("Unlock category so affects the other transactions when a category is change."));
                    }
                    else{
                        tooltip(str8_literal("Lock category so it doesn't affect the other transactions when a category is change."));
                    }
                    ImGui::SameLine();

                    ImGui::PushFont(my_font12);
                    fmt = str8_fmt(tm->frame_arena, "%c##hide_transaction%i", icon_lookup[Icon_Hide], idx);
                    if(ImGui::Button((char*)fmt.str)){
                        String8 trans_description = str8_cstring(trans->description);
                        for(Merchant* m = pm->merchants; m != 0; m = m->next){
                            String8 m_description = str8_cstring(m->description);
                            if(str8_compare(m_description, trans_description)){
                                m->hidden = true;
                                apply_hidden_transactions = true;
                                break;
                            }
                        }
                    }
                    ImGui::PopFont();
                    tooltip(str8_literal("Hide merchant from budget."));
                    ImGui::SameLine();

                    ImGui::PushID(idx);
                    if(trans->muted){
                        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(RED.r, RED.g, RED.b, 255));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.1f, 0.0f, 1.0f));
                    }
                    else{
                        ImGui::PushStyleColor(ImGuiCol_Button, pm->default_button_color);
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pm->default_button_hovered_color);
                    }
                    if(ImGui::Button("m##mute_transaction")){
                        trans->muted = !trans->muted;
                    }
                    tooltip(str8_literal("Mute Transaction."));
                    ImGui::PopStyleColor(2);
                    ImGui::PopID();
                    ImGui::SameLine();

                    fmt = str8_fmt(scratch.arena, "x##delete_transaction%i", idx);
                    if(ImGui::Button((char*)fmt.data)){
                        --month->transaction_count;
                        --year->transaction_count;
                        --pm->total_transaction_count;

                        dll_remove_old(trans);
                        pool_free(pm->transaction_pool, trans);
                    }
                    tooltip(str8_literal("Delete Transaction."));
                }
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();

        end_scratch(scratch);
        ImGui::End();
}

static void
collect_totals_for_months(void){
    // note: collect totals info for all months
    for(s32 m_idx = 0; m_idx < Month_Count; ++m_idx){
        MonthInfo* month = pm->year->months + m_idx;

        // note: collect category->spent from transactions
        CategoryGroup* category_group = pm->month_category_groups;
        for(s32 c_idx = 0; c_idx < pm->category_groups_count; ++c_idx){
            category_group = category_group->next;

            Category* category = category_group->categories;
            for(s32 r_idx = 0; r_idx < category_group->category_count; ++r_idx){
                category = category->next;
                category->spent = 0;

                if(!month->muted){
                    Transaction* trans = month->transactions;
                    for(s32 t_idx = 0; t_idx < month->transaction_count; ++t_idx){
                        trans = trans->next;
                        if(!trans->muted){
                            u32 t_length = char_length(trans->category);
                            String8 trans_selection = str8(trans->category, t_length);

                            u32 r_length = char_length(category->name);
                            String8 cat_part = str8_format(tm->frame_arena, "%s: ", category_group->name);
                            String8 name_part = str8(category->name, r_length);
                            String8 full = str8_concatenate(tm->frame_arena, cat_part, name_part);
                            if(str8_compare(full, trans_selection)){
                                f32 amount = atof(trans->amount);
                                category->spent += amount;
                                category->spent = round_to_hundredth(category->spent);
                            }
                        }
                    }
                }
            }
        }

        // note: calcluate month budget/totals
        f32 month_total_planned = 0.0f;
        f32 month_total_spent = 0.0f;
        f32 month_total_diff = 0.0f;
        category_group = pm->month_category_groups;
        for(s32 c_idx = 0; c_idx < pm->category_groups_count; ++c_idx){
            category_group = category_group->next;

            category_group->planned = 0;
            category_group->spent = 0;
            category_group->diff = 0;

            Category* category = category_group->categories;
            for(s32 r_idx = 0; r_idx < category_group->category_count; ++r_idx){
                category = category->next;
                if(!category->muted){
                    category->diff = atof(category->planned) - category->spent;
                    category->diff = round_to_hundredth(category->diff);

                    category_group->planned += atof(category->planned);
                    category_group->spent += category->spent;
                    category_group->diff += category->diff;
                }
            }
            category_group->planned = round_to_hundredth(category_group->planned);
            category_group->spent  = round_to_hundredth(category_group->spent);
            category_group->diff    = round_to_hundredth(category_group->diff);
            if(!category_group->muted){
                month_total_planned += category_group->planned;
                month_total_spent   += category_group->spent;
                month_total_diff    += category_group->diff;
            }
        }
        if(!month->muted){
            month->totals.planned = round_to_hundredth(month_total_planned);
            month->totals.spent   = round_to_hundredth(month_total_spent);
            month->totals.diff    = round_to_hundredth(month_total_diff);
            month->totals.saved   = round_to_hundredth(atof(pm->budget) - month->totals.spent);
            month->totals.goal    = round_to_hundredth(atof(pm->budget) - month->totals.planned);
        }
        else{
            month->totals.planned = 0;
            month->totals.spent   = 0;
            month->totals.diff    = 0;
            month->totals.saved   = 0;
            month->totals.goal    = 0;
        }
    }

    // note: calcluate quarterly budget/totals
    s32 month_start = 0;
    s32 month_end = 3;
    for(s32 q_idx = 0; q_idx < array_count(pm->quarter_totals); ++q_idx){
        Totals* totals = pm->quarter_totals + q_idx;

        totals->planned = 0;
        totals->spent   = 0;
        totals->diff    = 0;
        totals->saved   = 0;
        totals->goal    = 0;
        for(s32 m_idx = month_start; m_idx < month_end; ++m_idx){
            MonthInfo* month = pm->year->months + m_idx;

            if(!month->muted){
                totals->planned += month->totals.planned;
                totals->spent   += month->totals.spent;
                totals->diff    += month->totals.diff;
                totals->saved   += atof(pm->budget) - month->totals.spent;
                totals->goal    += atof(pm->budget) - month->totals.planned;
            }
        }
        totals->planned = round_to_hundredth(totals->planned);
        totals->spent   = round_to_hundredth(totals->spent);
        totals->diff    = round_to_hundredth(totals->diff);
        totals->saved   = round_to_hundredth(totals->saved);
        totals->goal    = round_to_hundredth(totals->goal);

        month_start += 3;
        month_end += 3;
    }

    // note: calcluate biannually budget/totals
    month_start = 0;
    month_end = 6;
    for(s32 q_idx = 0; q_idx < array_count(pm->biannual_totals); ++q_idx){
        Totals* totals = pm->biannual_totals + q_idx;

        totals->planned = 0;
        totals->spent   = 0;
        totals->diff    = 0;
        totals->saved   = 0;
        totals->goal    = 0;
        for(s32 m_idx = month_start; m_idx < month_end; ++m_idx){
            MonthInfo* month = pm->year->months + m_idx;

            if(!month->muted){
                totals->planned += month->totals.planned;
                totals->spent   += month->totals.spent;
                totals->diff    += month->totals.diff;
                totals->saved   += atof(pm->budget) - month->totals.spent;
                totals->goal    += atof(pm->budget) - month->totals.planned;
            }
        }
        totals->planned = round_to_hundredth(totals->planned);
        totals->spent   = round_to_hundredth(totals->spent);
        totals->diff    = round_to_hundredth(totals->diff);
        totals->saved   = round_to_hundredth(totals->saved);
        totals->goal    = round_to_hundredth(totals->goal);

        month_start += 6;
        month_end += 6;
    }

    // note: calcluate annual budget/totals
    pm->annual_totals.planned = 0;
    pm->annual_totals.spent   = 0;
    pm->annual_totals.diff    = 0;
    pm->annual_totals.saved   = 0;
    pm->annual_totals.goal    = 0;
    for(s32 m_idx = 0; m_idx < Month_Count; ++m_idx){
        MonthInfo* month = pm->year->months + m_idx;

        if(!month->muted){
            pm->annual_totals.planned += month->totals.planned;
            pm->annual_totals.spent   += month->totals.spent;
            pm->annual_totals.diff    += month->totals.diff;
            pm->annual_totals.saved   += atof(pm->budget) - month->totals.spent;
            pm->annual_totals.goal    += atof(pm->budget) - month->totals.planned;
        }
    }
    pm->annual_totals.planned = round_to_hundredth(pm->annual_totals.planned);
    pm->annual_totals.spent   = round_to_hundredth(pm->annual_totals.spent);
    pm->annual_totals.diff    = round_to_hundredth(pm->annual_totals.diff);
    pm->annual_totals.saved   = round_to_hundredth(pm->annual_totals.saved);
    pm->annual_totals.goal    = round_to_hundredth(pm->annual_totals.goal);

    {
        // note: calculate selected months category->spent
        CategoryGroup* category_group = pm->month_category_groups;
        for(s32 c_idx = 0; c_idx < pm->category_groups_count; ++c_idx){
            category_group = category_group->next;

            Category* category = category_group->categories;
            for(s32 r_idx = 0; r_idx < category_group->category_count; ++r_idx){
                category = category->next;
                category->spent = 0;

                MonthInfo* month = pm->year->months + pm->month_tab_idx;
                if(!month->muted){
                    Transaction* trans = month->transactions;
                    for(s32 t_idx = 0; t_idx < month->transaction_count; ++t_idx){
                        trans = trans->next;
                        if(!trans->muted){
                            u32 t_length = char_length(trans->category);
                            String8 trans_selection = str8(trans->category, t_length);

                            u32 r_length = char_length(category->name);
                            String8 cat_part = str8_format(tm->frame_arena, "%s: ", category_group->name);
                            String8 name_part = str8(category->name, r_length);
                            String8 full = str8_concatenate(tm->frame_arena, cat_part, name_part);
                            if(str8_compare(full, trans_selection)){
                                f32 amount = atof(trans->amount);
                                category->spent += amount;
                                category->spent = round_to_hundredth(category->spent);
                            }
                        }
                    }
                }
            }
        }

        // note: calcluate selected month budget/totals
        f32 month_total_planned = 0.0f;
        f32 month_total_spent = 0.0f;
        f32 month_total_diff = 0.0f;
        category_group = pm->month_category_groups;
        for(s32 c_idx = 0; c_idx < pm->category_groups_count; ++c_idx){
            category_group = category_group->next;

            category_group->planned = 0;
            category_group->spent = 0;
            category_group->diff = 0;

            Category* category = category_group->categories;
            for(s32 r_idx = 0; r_idx < category_group->category_count; ++r_idx){
                category = category->next;
                if(!category->muted){
                    category->diff = atof(category->planned) - category->spent;
                    category->diff = round_to_hundredth(category->diff);

                    category_group->planned += atof(category->planned);
                    category_group->spent += category->spent;
                    category_group->diff += category->diff;
                }
            }
            category_group->planned = round_to_hundredth(category_group->planned);
            category_group->spent  = round_to_hundredth(category_group->spent);
            category_group->diff    = round_to_hundredth(category_group->diff);
            if(!category_group->muted){
                month_total_planned += category_group->planned;
                month_total_spent   += category_group->spent;
                month_total_diff    += category_group->diff;
            }
        }
        MonthInfo* month = pm->year->months + pm->month_tab_idx;
        month->totals.planned = round_to_hundredth(month_total_planned);
        month->totals.spent   = round_to_hundredth(month_total_spent);
        month->totals.diff    = round_to_hundredth(month_total_diff);
        month->totals.saved   = round_to_hundredth(atof(pm->budget) - month->totals.spent);
        month->totals.goal    = round_to_hundredth(atof(pm->budget) - month->totals.planned);
    }

    // note mute/unmute category_group based on categories muted.
    CategoryGroup* category_group = pm->month_category_groups;
    for(s32 c_idx = 0; c_idx < pm->category_groups_count; ++c_idx){
        category_group = category_group->next;

        bool all_muted = true;
        Category* category = category_group->categories;
        for(s32 r_idx = 0; r_idx < category_group->category_count; ++r_idx){
            category = category->next;
            if(!category->muted){
                all_muted = false;
                break;
            }
        }

        category_group->muted = all_muted;
    }

    // note mute/unmute months based on transactions muted.
    for(s32 m_idx=0; m_idx < Month_Count; ++m_idx){
        MonthInfo* month = pm->year->months + m_idx;

        bool all_muted = true;
        bool all_locked = true;
        Transaction* trans = month->transactions;
        for(s32 t_idx = 0; t_idx < month->transaction_count; ++t_idx){
            trans = trans->next;
            if(!trans->muted){
                all_muted = false;
            }

            if(!trans->locked){
                all_locked = false;
            }
        }

        month->muted = all_muted;
        month->locked = all_locked;
    }

}

static void
do_one_frame(void){
    while(!events_empty(&events)){
        Event event = events_next(&events);

        if(event.type == EventType_KEYBOARD){
            if(event.keycode == KeyCode_ESCAPE){
                should_quit = true;
            }
        }
        if(event.type == EventType_QUIT){
            should_quit = true;
        }
        handle_controller_events(event);
    }


    u64 now_ticks = t_clock.get_os_timer();
    f64 frame_time = t_clock.get_seconds_elapsed(now_ticks, last_ticks);
    MSPF = 1000/1000/((f64)t_clock.frequency / (f64)(now_ticks - last_ticks));
    last_ticks = now_ticks;


    f64 second_elapsed = t_clock.get_seconds_elapsed(t_clock.get_os_timer(), frame_tick_start);
    if(second_elapsed > 1){
        FPS = ((f64)frame_count / second_elapsed);
        frame_tick_start = t_clock.get_os_timer();
        frame_count = 0;
    }

    //print("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f - simulations: %i\n", FPS, MSPF, t_clock.dt, accumulator, frame_time, second_elapsed, simulations);
    String8 fps = str8_fmt(tm->frame_arena, "FPS: %.2f", FPS);

    //------------------------------------------------------------------------------------------------------------
    // DRAW ENTIRE UI
    collect_totals_for_months();
    draw_entire_ui();

    if(apply_new_category){
        for(s32 year_idx=0; year_idx < MAX_YEAR_COUNT; ++year_idx){
            Year* year = pm->years + year_idx;
            for(s32 month_idx=0; month_idx < Month_Count; ++month_idx){
                MonthInfo* month = year->months + month_idx;
                for(Transaction* trans = month->transactions->next; trans != month->transactions; trans = trans->next){
                    if(trans->locked){
                        continue;
                    }

                    String8 trans_description = str8_cstring(trans->description);
                    String8 trans_category = str8_cstring(trans->category);
                    for(Merchant* m = pm->merchants; m != 0; m = m->next){
                        String8 m_description = str8_cstring(m->description);
                        if(str8_compare(m_description, trans_description)){
                            memcpy(trans->category, m->category, MERCH_CATEGORY_SIZE);
                            trans->merchant_id = m->id;
                            break;
                        }
                    }
                }
            }
        }
        apply_new_category = false;
    }

    if(apply_hidden_transactions){
        for(s32 year_idx=0; year_idx < MAX_YEAR_COUNT; ++year_idx){
            Year* year = pm->years + year_idx;
            for(s32 month_idx=0; month_idx < Month_Count; ++month_idx){
                MonthInfo* month = year->months + month_idx;
                month->transaction_visible_count = 0;
                for(Transaction* trans = month->transactions->next; trans != month->transactions; trans = trans->next){
                    String8 trans_description = str8_cstring(trans->description);
                    // slow: Hashtable here would help.
                    for(Merchant* m = pm->merchants; m != 0; m = m->next){
                        String8 m_description = str8_cstring(m->description);
                        if(str8_compare(m_description, trans_description)){
                            trans->hidden = m->hidden;
                            if(!m->hidden){
                                month->transaction_visible_count++;
                            }
                            break;
                        }
                    }
                }
            }
        }
        apply_hidden_transactions = false;
    }

#if DEBUG
    // ImGui::ShowDemoWindow(); // Show demo window! :)
#endif
    //

	//s32 count = 0;
	//for(Merchant* merch = pm->merchants; merch != 0; merch = merch->next){
      //print("hidden:[%i]\n", merch->hidden);
      //print("-----------------\n");
      //count++;
	//}
    //print("*******************************************\n");
    //print("------------------ count: %i -----------------------\n", count);


    {
        d3d_context->ClearRenderTargetView(d3d_framebuffer_view, BACKGROUND_COLOR.e);
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        d3d_swapchain->Present(1, 0);
    }
    // todo(rr): maybe remove?
    if(controller_button_pressed(KeyCode_ESCAPE, true)){
        //should_quit = true;

    }
    clear_controller_pressed();

    // note: Reset tab flags every frame.
    for(s32 i=0; i < 12; ++i){
        pm->month_tab_flags[i] = 0;
    }
    for(s32 i=0; i < 4; ++i){
        pm->quarter_tab_flags[i] = 0;
    }
    for(s32 i=0; i < 2; ++i){
        pm->biannual_tab_flags[i] = 0;
    }

    // IMPORTANT NOTE(rr): WE SERIALIZE ALL THE TIME
    serialize_config();
    serialize_budget();
    for(s32 y_idx=0; y_idx < MAX_YEAR_COUNT; ++y_idx){
        Year* year = pm->years + y_idx;
        serialize_year(year);
    }

    arena_free_zero(tm->frame_arena);
    total_frames++;
}

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type){
    begin_profiler();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    init_paths(global_arena);
    random_seed(0, 1);

    memory_init();
    clock_init(&t_clock);

    init_events(&events);

    FPS = 0;
    MSPF = 0;
    total_frames = 0;
    frame_count = 0;
	//simulations = 0;
    time_elapsed = 0;
    accumulator = 0.0;

    t_clock.dt =  1.0/240.0;
    last_ticks = t_clock.get_os_timer();
    frame_tick_start = t_clock.get_os_timer();

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

        // create pools
        pm->category_group_pool = push_pool(&pm->arena, sizeof(CategoryGroup), MAX_CATEGORY_GROUP_COUNT);
        pm->category_pool       = push_pool(&pm->arena, sizeof(Category), MAX_CATEGORY_COUNT);
        pm->transaction_pool    = push_pool(&pm->arena, sizeof(Transaction), MAX_TRANSACTION_COUNT);
        pm->csv_profile_pool    = push_pool(&pm->arena, sizeof(CSV_Profile), MAX_PROFILE_COUNT);
        // todo(rr): maybe I can just use scratch memory? I don't think I need this
        pm->data_arena          = push_arena(&pm->arena, MB(1));

        // setup free list from pools
        pool_free_all(pm->category_group_pool);
        pool_free_all(pm->category_pool);
        pool_free_all(pm->transaction_pool);
        pool_free_all(pm->csv_profile_pool);

        // setup sentinel node for category_groups
        pm->month_category_groups = (CategoryGroup*)pool_next(pm->category_group_pool);
        dll_clear(pm->month_category_groups);
        pm->quarter_category_groups = (CategoryGroup*)pool_next(pm->category_group_pool);
        dll_clear(pm->quarter_category_groups);
        pm->biannual_category_groups = (CategoryGroup*)pool_next(pm->category_group_pool);
        dll_clear(pm->biannual_category_groups);
        pm->annual_category_groups = (CategoryGroup*)pool_next(pm->category_group_pool);
        dll_clear(pm->annual_category_groups);
        pm->csv_profiles = (CSV_Profile*)pool_next(pm->csv_profile_pool);
        dll_clear(pm->csv_profiles);

        pm->yyyy.str = push_array(&pm->arena, u8, 32);
        pm->mm.str = push_array(&pm->arena, u8, 32);
        pm->dd.str = push_array(&pm->arena, u8, 32);

        //setup_date_formats();

        // give selection list memory
        pm->category_list = push_array(tm->options_arena, String8, MAX_CATEGORY_LIST_COUNT);
        for(s32 i=0; i < MAX_CATEGORY_LIST_COUNT; ++i){
            String8* option = pm->category_list + i;
            option->str = push_array(tm->options_arena, u8, CATEGORY_SIZE);
        }
        *pm->category_list = str8(" \0", 2);
        pm->default_path = os_application_path(&pm->arena);

        pm->draw_month_plan = true;
        pm->draw_quarter_plan = true;
        pm->draw_biannual_plan = true;
        pm->draw_annual_plan = true;
        pm->default_button_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
        pm->default_button_hovered_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
        default_active_color = ImGui::GetStyleColorVec4(ImGuiCol_TabActive);
        active_color = ImVec4(0.0f, default_active_color.y * 0.8f, default_active_color.z * 0.4f, default_active_color.w);

        default_hover_color = ImGui::GetStyleColorVec4(ImGuiCol_TabHovered);
        hover_color = ImVec4(0.0f, default_hover_color.y * 0.4f, default_hover_color.z * 0.8f, default_hover_color.w);

        // load config

        // todo: do this once
        for(s32 i=0; i < 12; ++i){
            pm->month_tab_flags[i] = 0;
        }
        for(s32 i=0; i < 4; ++i){
            pm->quarter_tab_flags[i] = 0;
        }
        for(s32 i=0; i < 2; ++i){
            pm->biannual_tab_flags[i] = 0;
        }

        deserialize_config();
        deserialize_budget();
        initialize_years_and_transactions();

        if(!window_width || !window_height){
            window_width = DEFAULT_SCREEN_WIDTH;
            window_height = DEFAULT_SCREEN_HEIGHT;
        }
        if(window_x < 0 || window_y < 0){
            window_x = 0;
            window_y = 0;
        }
        window = win32_window_create(L"Budgeteer", window_x, window_y, window_width, window_height, window_maximized);
        if(!window.handle){
            print("Error: Could not create window\n");
            return(0);
        }
        if(window.maximized){
            WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
            wp.showCmd = SW_SHOW;
            wp.rcNormalPosition.left   = window_restored_rect.left;
            wp.rcNormalPosition.top    = window_restored_rect.top;
            wp.rcNormalPosition.right  = window_restored_rect.right;
            wp.rcNormalPosition.bottom = window_restored_rect.bottom;
            if(SetWindowPlacement(window.handle, &wp)){
                print("SUCCESS\n");
            }
            else{
                print("FAIL\n");
                DWORD error = GetLastError();
                print_last_error(error);
            }
        }

        d3d_init(window.handle, window.width, window.height);
#if DEBUG
        d3d_init_debug_stuff();
#endif

        ImGui_ImplWin32_Init(window.handle);
        ImGui_ImplDX11_Init(d3d_device, d3d_context);

        show_cursor(true);

        pm->quarter_tab_flags[pm->quarter_tab_idx] = ImGuiTabItemFlags_SetSelected;
        pm->biannual_tab_flags[pm->biannual_tab_idx] = ImGuiTabItemFlags_SetSelected;

        // colors
        combo_popup_background_color = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
        combo_popup_background_color.w = 1;
        combo_popup_alternating_colors[0] = ImColor(10, 10, 10, 255);
        combo_popup_alternating_colors[1] = ImColor(20, 20, 20, 255);

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontDefault();
        ImFontConfig font_cfg;
        font_cfg.FontDataOwnedByAtlas = false;
        my_font12 = io.Fonts->AddFontFromMemoryTTF((void*)budgeteer_font, array_count(budgeteer_font), 12.0f, &font_cfg);
        my_font20 = io.Fonts->AddFontFromMemoryTTF((void*)budgeteer_font, array_count(budgeteer_font), 20.0f, &font_cfg);
        io.Fonts->Build();

        GREEN = {
            .r = 112/255.0f,
            .g = 197/255.0f,
            .b = 72/255.0f,
            .a = 255.0f,
        };
        GREEN = linear_from_srgb(GREEN);
        GREEN = RGBA_1_to_255(GREEN);
        RED = {
            .r = 220/255.0f,
            .g = 60/255.0f,
            .b = 60/255.0f,
            .a = 255.0f,
        };
        RED = linear_from_srgb(RED);
        RED = RGBA_1_to_255(RED);

        ImGui::PushFont(my_font12);
        ScratchArena scratch = begin_scratch();
        fmt = str8_fmt(scratch.arena, "%c", icon_lookup[Icon_Unlocked]);
        ImVec2 size = ImGui::CalcTextSize((char*)fmt.str);
        const ImGuiStyle& style = ImGui::GetStyle();
        lock_button_width = size.x + (style.FramePadding.x * 2.0f);
        ImGui::PopFont();
        end_scratch(scratch);

        generate_merchants();
        test_merchants();

        memory.initialized = true;
    }

    should_quit = false;
    while(!should_quit){
        begin_timed_scope("while(!should_quit)");
        MSG message;
        while(PeekMessageW(&message, window.handle, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        do_one_frame();
    }

    //if(should_quit){
    //    if(!os_path_exists(saves_path)){
    //        os_dir_create(saves_path);
    //    }
    //    serialize_config();
    //    serialize_budget();
    //    Year* year;
    //    for(s32 y_idx=0; y_idx < MAX_YEAR_COUNT; ++y_idx){
    //        year = pm->years + y_idx;
    //        serialize_year(year);
    //    }
    //}

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    d3d_release();
    end_profiler();

    return(0);
}
