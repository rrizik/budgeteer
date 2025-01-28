#include "main.hpp"

static s32
wrap_index(s32 idx, s32 size){
    return(((idx % size) + size) % size);
}

static void
initialize_years_and_transactions(void){
    time_t t = time(0);
    struct tm *tm_info = localtime(&t);
    pm->current_year = tm_info->tm_year + 1900;

    //print("YEAR: %04d-%02d-%02d\n", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday);
    if(!year_config_deserialized){
        pm->year_idx = 0;
    }

    for(s32 idx=0; idx < MAX_YEAR_COUNT/2; ++idx){
        Year* year = pm->years + idx;
        year->number = pm->current_year + idx;
    }
    for(s32 idx=MAX_YEAR_COUNT-1; idx >= MAX_YEAR_COUNT/2; --idx){
        Year* year = pm->years + idx;
        year->number = pm->current_year - (MAX_YEAR_COUNT - idx);
    }


    for(s32 idx=0; idx < MAX_YEAR_COUNT; ++idx){
        Year* year = pm->years + idx;

        // setup sentinel node for month transactions
        for(s32 i=0; i < Month_Count; ++i){
            MonthInfo* month = year->months + i;
            month->transactions = (Transaction*)pool_next(pm->transaction_pool);
            dll_clear(month->transactions);
        }

        deserialize_year(year);

    }

    pm->month_tab_flags[pm->month_tab_idx] = ImGuiTabItemFlags_SetSelected;
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
    sprites_path = str8_path_append(global_arena, build_path, str8_literal("assets/sprites"));
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
            do_one_frame();

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

        ScratchArena scratch = begin_scratch();
        Year* year = pm->year;
        MonthInfo* month = year->months + pm->month_tab_idx;
        s32 month_tab_idx = pm->month_tab_idx;

        ImGui::Begin("Budgeteer", 0, ImGuiWindowFlags_NoResize|
                                     ImGuiWindowFlags_NoCollapse|
                                     ImGuiWindowFlags_NoMove|
                                     ImGuiWindowFlags_NoTitleBar);
        ImGui::Checkbox("Show Tooltips", &show_tooltips);
        ImGui::Columns(2);
        ImGui::BeginChild("Child1", ImVec2(0, 0), true);

        ImGui::Text("Budget:");
        ImGui::SameLine();
        ImGui::PushItemWidth(75);
        //if(pm->budget[0] == 0){
        //    pm->budget[0] = '0';
        //    pm->budget[1] = '\0';
        //}
        ImGui::InputText("##Budget", (char*)pm->budget.str, 128, ImGuiInputTextFlags_CharsDecimal |
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

            if(ImGui::BeginTabItem(month_names[month_tab_idx], 0)){
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("$%.2f", totals->saved);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", totals->saved);
        }

        ImGui::NextColumn();
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("$%.2f", totals->saved);
            ImGui::PopStyleColor();
        }
        else{
            ImGui::Text("$%.2f", totals->saved);
        }
        ImGui::EndChild();
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        //#####MONTH PLAN######
        ImGui::BeginChild("Child3", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar|ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        if(pm->draw_month_plan){
            if(ImGui::Button("V")){
                pm->draw_month_plan = false;
            }
        }
        else{
            if(ImGui::Button(">")){
                pm->draw_month_plan = true;
            }
        }
        tooltip(str8_literal("Collapse/Expand Month Plan."));

        ImGui::SameLine();
        ImGui::SeparatorText("Month Plan");

        if(pm->draw_month_plan){
            ImGui::SetCursorPosX(row_count_column_start + input_padding);
            ImGui::Text("#");
            ImGui::SameLine();
            ImGui::SetCursorPosX(category_column_start);
            ImGui::Text("Category");
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
            if(ImGui::Button("+##add_category_button")){
                Category* category = (Category*)pool_next(pm->category_pool);
                dll_push_back(pm->month_categories, category);
                category->rows = (Row*)pool_next(pm->row_pool);
                dll_clear(category->rows);

                pm->categories_count++;
            }
            tooltip(str8_literal("Add New Category."));

            //note: popluate empty planned with 0's for visual appeal
            Category* category = pm->month_categories;
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

            category = pm->month_categories;
            for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
                custom_separator();
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
                tooltip(str8_literal("Collapse/Expand Or Drag/Swap Category."));
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
                            Category* c = pm->month_categories;
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
                ImGui::InputText((char*)unique_id.data, category->name, CAT_NAME_SIZE, ImGuiInputTextFlags_AutoSelectAll);
                ImGui::PopItemWidth();

                ImGui::SameLine();
                ImGui::SetCursorPosX(planned_column_start + input_padding);
                String8 planned_str = str8_formatted(scratch.arena, "$%.2f", category->planned);
                ImGui::Text((char*)planned_str.data);

                ImGui::SameLine();
                ImGui::SetCursorPosX(spent_column_start + input_padding);
                String8 spent_str = str8_formatted(scratch.arena, "$%.2f", category->spent);
                ImGui::Text((char*)spent_str.data);

                ImGui::SameLine();
                ImGui::SetCursorPosX(diff_column_start);
                category->diff = category->planned - category->spent;
                String8 category_diff = str8_formatted(scratch.arena, "$%.2f", category->diff);
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
                    //pm->total_rows_count++;
                }
                tooltip(str8_literal("Add New Sub-Category."));
                ImGui::PopID();

                ImGui::SameLine();
                ImGui::SetCursorPosX(x_column_start);
                ImGui::PushID(c_idx);
                if(ImGui::Button("x##remove_category")){
                    //pm->total_rows_count -= category->row_count;
                    --pm->categories_count;

                    dll_remove(category);
                    pool_free(pm->category_pool, category);
                }
                tooltip(str8_literal("Delete All Sub-Categories."));
                ImGui::PopID();

                ImGui::SameLine();
                ImGui::SetCursorPosX(m_column_start);
                ImGui::PushID(c_idx);
                if(category->muted){
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.1f, 0.0f, 1.0f));
                }
                else{
                    ImGui::PushStyleColor(ImGuiCol_Button, pm->default_button_color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pm->default_button_hovered_color);
                }
                if(ImGui::Button("m##mute_category")){
                    category->muted = !category->muted;
                    if(category->muted){
                        Row* row = category->rows;
                        for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                            row = row->next;
                            row->muted = true;
                        }
                    }
                    else{
                        Row* row = category->rows;
                        for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                            row = row->next;
                            row->muted = false;
                        }
                    }
                }
                tooltip(str8_literal("Mute All Sub-Categories."));
                ImGui::PopID();
                ImGui::PopStyleColor(2);

                // render rows per category
                Row* row = category->rows;
                for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                    row = row->next;

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
                        tooltip(str8_literal("Drag/Swap Sub-Category."));

                        if(ImGui::BeginDragDropTarget()){
                            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_ROW")){
                                s32* payload_data = (s32*)payload->Data;
                                s32 from_idx = *payload_data;
                                if(from_idx != r_idx){
                                    Row* r = category->rows;
                                    // note(rr): iterate to the correct node, since we cant index with linked lists
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
                        ImGui::InputText((char*)input_id.data, row->name, ROW_NAME_SIZE, ImGuiInputTextFlags_AutoSelectAll);
                        ImGui::PopItemWidth();

                        ImGui::SameLine();
                        ImGui::SetCursorPosX(planned_column_start);
                        ImGui::PushItemWidth(planned_column_width);
                        String8 planned_id = str8_formatted(scratch.arena, "##planned%i%i", r_idx, c_idx);
                        ImGui::InputText((char*)planned_id.data, row->planned, ROW_PLANNED_SIZE, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll);

                        ImGui::PopItemWidth();

                        ImGui::SameLine();
                        ImGui::SetCursorPosX(spent_column_start + input_padding);
                        ImGui::PushItemWidth(spent_column_width);
                        String8 row_spent = str8_formatted(scratch.arena, "$%.2f", row->spent);
                        ImGui::Text((char*)row_spent.data);
                        ImGui::PopItemWidth();

                        ImGui::SameLine();
                        ImGui::SetCursorPosX(diff_column_start);
                        f32 planned = atof(row->planned);
                        if((planned - row->spent) < 0){
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                        }
                        String8 row_diff = str8_formatted(scratch.arena, "$%.2f", row->diff);
                        ImGui::Text((char*)row_diff.data);
                        if((planned - row->spent) < 0){
                            ImGui::PopStyleColor();
                        }

                        ImGui::SameLine();
                        ImGui::SetCursorPosX(x_column_start);
                        ImGui::PushID(uid);
                        if(ImGui::Button("x##remove_row")){
                            //--pm->total_rows_count;
                            --category->row_count;

                            dll_remove(row);
                            pool_free(pm->row_pool, row);
                        }
                        tooltip(str8_literal("Delete Sub-Category."));
                        ImGui::PopID();

                        ImGui::SameLine();
                        ImGui::SetCursorPosX(m_column_start);
                        ImGui::PushID(uid);
                        if(row->muted){
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.1f, 0.0f, 1.0f));
                        }
                        else{
                            ImGui::PushStyleColor(ImGuiCol_Button, pm->default_button_color);
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pm->default_button_hovered_color);
                        }
                        if(ImGui::Button("m##mute_row")){
                            row->muted = !row->muted;
                        }
                        tooltip(str8_literal("Mute Sub-Category."));
                        ImGui::PopID();
                        ImGui::PopStyleColor(2);
                    }
                }
            }
        }
        custom_separator();

        if(pm->draw_quarter_plan){
            if(ImGui::Button("V##draw_quarter")){
                pm->draw_quarter_plan = false;
            }
        }
        else{
            if(ImGui::Button(">##draw_quarter")){
                pm->draw_quarter_plan = true;
            }
        }
        tooltip(str8_literal("Collapse/Expand Quarter Plan."));
        ImGui::SameLine();
        ImGui::SeparatorText("Quarter Plan");

        if(pm->draw_quarter_plan){
            ImGui::SetCursorPosX(row_count_column_start + input_padding);
            ImGui::Text("#");
            ImGui::SameLine();
            ImGui::SetCursorPosX(category_column_start);
            ImGui::Text("Q1 Category");
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
            if(ImGui::Button("+##add_quarter_category_button")){
                Category* category = (Category*)pool_next(pm->category_pool);
                dll_push_back(pm->quarter_categories, category);
                category->rows = (Row*)pool_next(pm->row_pool);
                dll_clear(category->rows);

                pm->quarter_categories_count++;
            }
            tooltip(str8_literal("Add New Category."));
        }
        custom_separator();

        if(pm->draw_biannual_plan){
            if(ImGui::Button("V##draw_biannual")){
                pm->draw_biannual_plan = false;
            }
        }
        else{
            if(ImGui::Button(">##draw_biannual")){
                pm->draw_biannual_plan = true;
            }
        }
        tooltip(str8_literal("Collapse/Expand Bi-Annual Plan."));
        ImGui::SameLine();
        ImGui::SeparatorText("BiAnnual Plan");

        if(pm->draw_biannual_plan){
            ImGui::SetCursorPosX(row_count_column_start + input_padding);
            ImGui::Text("#");
            ImGui::SameLine();
            ImGui::SetCursorPosX(category_column_start);
            ImGui::Text("Q1 Category");
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
            if(ImGui::Button("+##add_biannual_category_button")){
                Category* category = (Category*)pool_next(pm->category_pool);
                dll_push_back(pm->biannual_categories, category);
                category->rows = (Row*)pool_next(pm->row_pool);
                dll_clear(category->rows);

                pm->biannual_categories_count++;
            }
            tooltip(str8_literal("Add New Category."));
        }
        custom_separator();

        if(pm->draw_annual_plan){
            if(ImGui::Button("V##draw_annual")){
                pm->draw_annual_plan = false;
            }
        }
        else{
            if(ImGui::Button(">##draw_annual")){
                pm->draw_annual_plan = true;
            }
        }
        tooltip(str8_literal("Collapse/Expand Annual Plan."));
        ImGui::SameLine();
        ImGui::SeparatorText("Annual Plan");

        if(pm->draw_annual_plan){
            ImGui::SetCursorPosX(row_count_column_start + input_padding);
            ImGui::Text("#");
            ImGui::SameLine();
            ImGui::SetCursorPosX(category_column_start);
            ImGui::Text("Q1 Category");
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
            if(ImGui::Button("+##add_annual_category_button")){
                Category* category = (Category*)pool_next(pm->category_pool);
                dll_push_back(pm->annual_categories, category);
                category->rows = (Row*)pool_next(pm->row_pool);
                dll_clear(category->rows);

                pm->annual_categories_count++;
            }
            tooltip(str8_literal("Add New Category."));

            //Category* category = pm->annual_categories;
            //for(s32 c_idx = 0; c_idx < pm->annual_categories_count; ++c_idx){
            //    category = category->next;

            //    ImGui::SetCursorPosX(collapse_column_start);
            //    ImGui::PushID(c_idx);
            //    if(category->draw_rows){
            //        if(ImGui::Button("V")){
            //            category->draw_rows = false;
            //        }
            //    }
            //    else{
            //        if(ImGui::Button(">")){
            //            if(category->row_count){
            //                category->draw_rows = true;
            //            }
            //        }
            //    }
            //    ImGui::PopID();
            //    tooltip(str8_literal("Collapse/Expand Or Drag/Swap Category."));

            //    if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)){
            //        ImGui::SetDragDropPayload("DRAG_ROW", &c_idx, sizeof(s32));
            //        ImGui::Text("%s", category->name);
            //        ImGui::EndDragDropSource();
            //    }
            //    if(ImGui::BeginDragDropTarget()){
            //        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_ROW")){
            //            s32* payload_data = (s32*)payload->Data;
            //            s32 from_idx = *payload_data;
            //            if(from_idx != c_idx){
            //                Category* c = pm->categories;
            //                for(s32 i=0; i <= from_idx; ++i){
            //                    c = c->next;
            //                }
            //                dll_swap(c, category, Category);
            //            }
            //        }
            //        ImGui::EndDragDropTarget();
            //    }

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(row_count_column_start + input_padding);
            //    if(category->draw_rows){
            //        ImGui::Text("-");
            //    }
            //    else{
            //        ImGui::Text("%i", category->row_count);
            //    }

            //    ImGui::SameLine();


            //    ImGui::SetCursorPosX(category_column_start);
            //    ImGui::PushItemWidth(category_column_width);
            //    String8 unique_id = str8_formatted(scratch.arena, "##category%i", c_idx);
            //    ImGui::InputText((char*)unique_id.data, category->name, CAT_NAME_SIZE, ImGuiInputTextFlags_AutoSelectAll);
            //    ImGui::PopItemWidth();

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(planned_column_start + input_padding);
            //    String8 planned_str = str8_formatted(scratch.arena, "%.2f", category->planned);
            //    ImGui::Text((char*)planned_str.data);

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(spent_column_start + input_padding);
            //    String8 spent_str = str8_formatted(scratch.arena, "%.2f", category->spent);
            //    ImGui::Text((char*)spent_str.data);

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(diff_column_start);
            //    category->diff = category->planned - category->spent;
            //    String8 category_diff = str8_formatted(scratch.arena, "%.2f", category->diff);
            //    if(category->diff < 0){
            //        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            //    }
            //    ImGui::Text((char*)category_diff.data);
            //    if(category->diff < 0){
            //        ImGui::PopStyleColor();
            //    }

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(plus_column_start);
            //    ImGui::PushID(c_idx);
            //    if(ImGui::Button("+##add_row_button")){
            //        Row* r = (Row*)pool_next(pm->row_pool);
            //        dll_push_back(category->rows, r);

            //        category->draw_rows = true;
            //        category->row_count++;
            //        //pm->total_rows_count++;
            //    }
            //    ImGui::PopID();

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(x_column_start);
            //    ImGui::PushID(c_idx);
            //    if(ImGui::Button("x##remove_category")){
            //        //pm->total_rows_count -= category->row_count;
            //        --pm->annual_categories_count;

            //        dll_remove(category);
            //        pool_free(pm->category_pool, category);
            //    }
            //    ImGui::PopID();

            //    ImGui::SameLine();
            //    ImGui::SetCursorPosX(m_column_start);
            //    ImGui::PushID(c_idx);
            //    if(category->muted){
            //        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            //        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.1f, 0.0f, 1.0f));
            //    }
            //    else{
            //        ImGui::PushStyleColor(ImGuiCol_Button, pm->default_button_color);
            //        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pm->default_button_hovered_color);
            //    }
            //    // todo: review this, this stuff might not have rows
            //    if(ImGui::Button("m##mute_category")){
            //        category->muted = !category->muted;
            //        if(category->muted){
            //            Row* row = category->rows;
            //            for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
            //                row = row->next;
            //                row->muted = true;
            //            }
            //        }
            //        else{
            //            Row* row = category->rows;
            //            for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
            //                row = row->next;
            //                row->muted = false;
            //            }
            //        }
            //    }
            //    ImGui::PopID();
            //    ImGui::PopStyleColor(2);
            //    custom_separator();
            //}
        }
        custom_separator();
        // END OF ROWS
        ImGui::EndChild();
        ImGui::EndChild();

        //########COLUMN2######################################################################
        //##################
        //###TRANSACTIONS###
        //##################

        ImGui::NextColumn();
        ImGui::BeginChild("Child4", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar|ImGuiWindowFlags_AlwaysHorizontalScrollbar);

        ImGui::SeparatorText("Transactions");
        if(ImGui::Button("<<##prev_prev_year")){
            pm->year_idx -= 5;
            pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
        }
        tooltip(str8_literal("Go To Previous 5 Years"));
        ImGui::SameLine();
        if(ImGui::Button("<##prev_year")){
            --pm->year_idx;
            pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
        }
        tooltip(str8_literal("Go To Previous Year"));
        ImGui::SameLine();
        ImGui::Text("Year %04d", year->number);
        ImGui::SameLine();
        if(ImGui::Button(">##next_year")){
            ++pm->year_idx;
            pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
        }
        tooltip(str8_literal("Go To Next Year"));
        ImGui::SameLine();
        if(ImGui::Button(">>##next_next_year")){
            pm->year_idx += 5;
            pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
        }
        tooltip(str8_literal("Go To Next 5 Years"));
        ImGui::SameLine();
        if(ImGui::Button("^##current_year")){
            pm->year_idx = 0;
            pm->year = pm->years + wrap_index(pm->year_idx, MAX_YEAR_COUNT);
        }
        tooltip(str8_literal("Go To Current Year"));

        if(ImGui::BeginTabBar("##Month", ImGuiTabBarFlags_None)){

            ImGui::PushStyleColor(ImGuiCol_TabActive, active_color);
            ImGui::PushStyleColor(ImGuiCol_TabHovered, hover_color);

            if(ImGui::BeginTabItem("January", 0, pm->month_tab_flags[0])){
                pm->month_tab_idx = Month_Jan;
                //year->month = year->months + pm->month_tab_idx;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("February", 0, pm->month_tab_flags[1])){
                pm->month_tab_idx = Month_Feb;
                //year->month = year->months + year->month_tab_idx;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("March", 0, pm->month_tab_flags[2])){
                pm->month_tab_idx = Month_Mar;
                //year->month = year->months + year->month_tab_idx;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("April", 0, pm->month_tab_flags[3])){
                pm->month_tab_idx = Month_Apr;
                //year->month = year->months + year->month_tab_idx;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("May", 0, pm->month_tab_flags[4])){
                pm->month_tab_idx = Month_May;
                //year->month = year->months + year->month_tab_idx;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("June", 0, pm->month_tab_flags[5])){
                pm->month_tab_idx = Month_Jun;
                //year->month = year->months + year->month_tab_idx;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("July", 0, pm->month_tab_flags[6])){
                pm->month_tab_idx = Month_Jul;
                //year->month = year->months + year->month_tab_idx;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("August", 0, pm->month_tab_flags[7])){
                pm->month_tab_idx = Month_Aug;
                //year->month = year->months + year->month_tab_idx;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("September", 0, pm->month_tab_flags[8])){
                pm->month_tab_idx = Month_Sep;
                //year->month = year->months + year->month_tab_idx;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("October", 0, pm->month_tab_flags[9])){
                pm->month_tab_idx = Month_Oct;
                //year->month = year->months + year->month_tab_idx;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("November", 0, pm->month_tab_flags[10])){
                pm->month_tab_idx = Month_Nov;
                //year->month = year->months + year->month_tab_idx;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("December", 0, pm->month_tab_flags[11])){
                pm->month_tab_idx = Month_Dec;
                //year->month = year->months + year->month_tab_idx;
                ImGui::EndTabItem();
            }
            ImGui::PopStyleColor(2);
            ImGui::EndTabBar();
        }


        ImGui::Spacing();
        //ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + date_column_start - 30 + input_padding);
        ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + hash_column_start + input_padding);
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
            dll_push_back(month->transactions, trans);

            if(month->transactions_count == 0){
                ScratchArena scratch = begin_scratch();
                String8 date = str8_fmt(scratch.arena, "01/01/%04d", year->number);
                memcpy((void*)trans->date, (void*)date.str, date.size);
                end_scratch(scratch);
            }
            else{
                ScratchArena scratch = begin_scratch();
                Transaction* last = trans->prev;
                s32 date_length = (s32)char_length(last->date);
                String8 date_str8 = str8(last->date, date_length);
                String8Node* parts = str8_split(scratch.arena, date_str8, '/');
                parts->prev->str = str8_fmt(scratch.arena, "%04d", year->number);

                String8Join join = {0};
                join.mid = str8_literal("/");
                String8 result = str8_join(scratch.arena, parts, join);
                memcpy((void*)trans->date, (void*)result.str, result.count);
                end_scratch(scratch);
            }
            memcpy((void*)trans->selection, (void*)pm->selection_list->str, pm->selection_list->size);

            month->transactions_count++;
            year->total_transactions++;
        }
        tooltip(str8_literal("Add Transaction."));

        ImGui::SameLine();
        //ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + plus_expense_column_start);
        if(ImGui::Button("x##x_all_transactions")){
            Transaction* t = month->transactions;
            for(s32 t_idx=0; t_idx < month->transactions_count; ++t_idx){
                t = t->next;
                dll_remove(t);
                pool_free(pm->transaction_pool, t);
                t = month->transactions;
            }
            dll_clear(month->transactions);
            year->total_transactions -= month->transactions_count;
            month->transactions_count = 0;
        }
        tooltip(str8_literal("Delete All Transactions."));

        ImGui::SameLine();
        //ImGui::SetCursorPosX(m_column_start);
        ImGui::PushID(100000);
        if(month->muted){
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.1f, 0.0f, 1.0f));
        }
        else{
            ImGui::PushStyleColor(ImGuiCol_Button, pm->default_button_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pm->default_button_hovered_color);
        }
        if(ImGui::Button("m##mute_month")){
            month->muted = !month->muted;
            if(month->muted){
                Transaction* trans = month->transactions;
                for(s32 t_idx = 0; t_idx < month->transactions_count; ++t_idx){
                    trans = trans->next;
                    trans->muted = true;
                }
            }
            else{
                Transaction* trans = month->transactions;
                for(s32 t_idx = 0; t_idx < month->transactions_count; ++t_idx){
                    trans = trans->next;
                    trans->muted = false;
                }
            }
        }
        tooltip(str8_literal("Mute All Transactions."));
        ImGui::PopStyleColor(2);
        ImGui::PopID();

        ImGui::SameLine();
        if(ImGui::Button("Load CSV##deserialize_csv")){
            char* file = tinyfd_openFileDialog("Open CSV File", (char*)pm->default_path.str, 0, 0, 0, 0);
            if(file){
                String8 file_path = str8(file, char_length(file));

                if(str8_compare_nocase(str8_path_extension(file_path), str8_literal(".csv"))){
                    pm->default_path = str8_path_pop(&pm->arena, file_path, '\\');
                    deserialize_csv(file_path);
                }
            }
        }
        tooltip(str8_literal("Load CSV From File."));

        ImGui::SameLine();
        ImTextureID gear_texture_id = (ImTextureID)gear_texture.view;
        ImVec2 button_size(17, 17);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
        if(ImGui::ImageButton(gear_texture_id, button_size)){
            ImVec2 button_pos = ImGui::GetItemRectMin();
            ImVec2 button_size = ImGui::GetItemRectSize();
            button_pos.x -= 700;
            button_pos.y += (button_size.y + 10);
            ImGui::SetNextWindowPos(button_pos, ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_Always);

            ImGui::OpenPopup("CSV Profile");
        }
        tooltip(str8_literal("CSV Profile."));
        ImGui::PopStyleVar();

        if(ImGui::BeginPopupModal("CSV Profile", 0, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove)){
            if(pm->csv_profile_count == 0){
                CSV_Profile* node = (CSV_Profile*)pool_next(pm->csv_profile_pool);
                dll_push_back(pm->csv_profile, node);
                pm->csv_profile_count++;
            }

            CSV_Profile* profile = pm->csv_profile + pm->csv_profile_idx;
            ImGui::InputText("##name", profile->name, 1024);
            ImGui::InputText("##date", profile->date, 128);
            ImGui::InputText("##amount", profile->amount, 1024);
            ImGui::InputText("##description", profile->description, 128);

            if(controller_button_pressed(KeyCode_ESCAPE, true)){
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        //if(ImGui::BeginPopupModal("CSV Settings", 0, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove)){

        //    ImGui::BeginChild("Child5", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.45), true, 0);

        //    ImGui::SeparatorText("CSV Column Names");

        //    CSVColumnNode* node;
        //    ImGui::Columns(3, 0, false);
        //    ImGui::SetColumnWidth(0, 222.0f);
        //    ImGui::SetColumnWidth(1, 222.0f);
        //    ImGui::SetColumnWidth(2, 222.0f);

        //    ImGui::Text("Date Column:");
        //    ImGui::SameLine();
        //    ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_plus_button_start);
        //    if(ImGui::Button("+##add_csv_date_column")){
        //        CSVColumnNode* node = (CSVColumnNode*)pool_next(pm->csv_pool);
        //        dll_push_back(pm->date_names, node);
        //        pm->date_names_count++;
        //    }
        //    tooltip(str8_literal("Add date column name to search for when loading CSV files."));

        //    ImGui::SameLine();
        //    ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_x_button_start);
        //    if(ImGui::Button("x##remove_all_csv_date_column")){
        //        node = pm->date_names;
        //        for(s32 c_idx=0; c_idx < pm->date_names_count; ++c_idx){
        //            node = node->next;
        //            dll_remove(node);
        //            pool_free(pm->csv_pool, node);
        //            node = pm->date_names;
        //        }
        //        dll_clear(pm->date_names);
        //        pm->date_names_count = 0;
        //    }
        //    tooltip(str8_literal("Remove all date column names."));

        //    f32 minus_padding = 8;
        //    ImGui::BeginChild("date_names_child", ImVec2(0, 0), true, 0);
        //    String8 unique_fmt;
        //    node = pm->date_names;
        //    for(s32 c_idx=0; c_idx < pm->date_names_count; ++c_idx){
        //        node = node->next;
        //        unique_fmt = str8_fmt(tm->frame_arena, "##CSV_Dates%i\n", c_idx);
        //        ImGui::InputText((char*)unique_fmt.data, node->name, 1024);

        //        ImGui::SameLine();
        //        ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_x_button_start - minus_padding);
        //        unique_fmt = str8_fmt(tm->frame_arena, "x##remove_csv_date_column%i\n", c_idx);
        //        if(ImGui::Button((char*)unique_fmt.data)){
        //            dll_remove(node);
        //            pool_free(pm->csv_pool, node);

        //            --pm->date_names_count;
        //        }
        //        tooltip(str8_literal("Remove date column name."));
        //    }
        //    ImGui::EndChild();

        //    ImGui::NextColumn();
        //    ImGui::Text("Amount Column:");
        //    ImGui::SameLine();
        //    ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_plus_button_start);
        //    if(ImGui::Button("+##add_csv_amounts_column")){
        //        CSVColumnNode* node = (CSVColumnNode*)pool_next(pm->csv_pool);
        //        dll_push_back(pm->amount_names, node);
        //        pm->amount_names_count++;
        //    }
        //    tooltip(str8_literal("Add amount column name to search for when loading CSV files."));

        //    ImGui::SameLine();
        //    ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_x_button_start);
        //    if(ImGui::Button("x##remove_all_csv_amount_column")){
        //        node = pm->amount_names;
        //        for(s32 c_idx=0; c_idx < pm->amount_names_count; ++c_idx){
        //            node = node->next;
        //            dll_remove(node);
        //            pool_free(pm->csv_pool, node);
        //            node = pm->amount_names;
        //        }
        //        dll_clear(pm->amount_names);
        //        pm->amount_names_count = 0;
        //    }
        //    tooltip(str8_literal("Remove all amount column names."));

        //    ImGui::BeginChild("amount_names_child", ImVec2(0, 0), true, 0);
        //    node = pm->amount_names;
        //    for(s32 c_idx=0; c_idx < pm->amount_names_count; ++c_idx){
        //        node = node->next;
        //        unique_fmt = str8_fmt(tm->frame_arena, "##CSV_Amounts%i\n", c_idx);
        //        ImGui::InputText((char*)unique_fmt.data, node->name, 1024);

        //        ImGui::SameLine();
        //        ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_x_button_start - minus_padding);
        //        unique_fmt = str8_fmt(tm->frame_arena, "x##remove_csv_amount_column%i\n", c_idx);
        //        if(ImGui::Button((char*)unique_fmt.data)){
        //            dll_remove(node);
        //            pool_free(pm->csv_pool, node);

        //            --pm->amount_names_count;
        //        }
        //        tooltip(str8_literal("Remove amount column name."));
        //    }
        //    ImGui::EndChild();

        //    ImGui::NextColumn();
        //    ImGui::Text("Description Column:");
        //    ImGui::SameLine();
        //    ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_plus_button_start);
        //    if(ImGui::Button("+##add_csv_description_column")){
        //        CSVColumnNode* node = (CSVColumnNode*)pool_next(pm->csv_pool);
        //        dll_push_back(pm->description_names, node);
        //        pm->description_names_count++;
        //    }
        //    tooltip(str8_literal("Add description column name to search for when loading CSV files."));

        //    ImGui::SameLine();
        //    ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_x_button_start);
        //    if(ImGui::Button("x##remove_all_csv_description_column")){
        //        node = pm->description_names;
        //        for(s32 c_idx=0; c_idx < pm->description_names_count; ++c_idx){
        //            node = node->next;
        //            dll_remove(node);
        //            pool_free(pm->csv_pool, node);
        //            node = pm->description_names;
        //        }
        //        dll_clear(pm->description_names);
        //        pm->description_names_count = 0;
        //    }
        //    tooltip(str8_literal("Remove all description column names."));

        //    ImGui::BeginChild("description_names_child", ImVec2(0, 0), true, 0);
        //    node = pm->description_names;
        //    for(s32 c_idx=0; c_idx < pm->description_names_count; ++c_idx){
        //        node = node->next;
        //        unique_fmt = str8_fmt(tm->frame_arena, "##CSV_Description%i\n", c_idx);
        //        ImGui::InputText((char*)unique_fmt.data, node->name, 1024);

        //        ImGui::SameLine();
        //        ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_x_button_start - minus_padding);
        //        unique_fmt = str8_fmt(tm->frame_arena, "x##remove_csv_description_column%i\n", c_idx);
        //        if(ImGui::Button((char*)unique_fmt.data)){
        //            dll_remove(node);
        //            pool_free(pm->csv_pool, node);

        //            --pm->description_names_count;
        //        }
        //        tooltip(str8_literal("Remove description column name."));
        //    }
        //    ImGui::EndChild();
        //    ImGui::EndChild();

        //    ImGui::BeginChild("Child6", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.85), true, 0);
        //    ImGui::SeparatorText("CSV Date Formats");
        //    ImGui::Text("Date Formats:");
        //    ImGui::SameLine();
        //    ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_plus_button_start);
        //    if(ImGui::Button("+##add_csv_date_format")){
        //        //CSVColumnNode* node = (CSVColumnNode*)pool_next(pm->csv_pool);
        //        //dll_push_back(pm->date_formats, node);
        //        //pm->date_formats_count++;
        //    }
        //    tooltip(str8_literal("Add date formats for the CSV loader to cosnider."));

        //    ImGui::SameLine();
        //    ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_x_button_start);
        //    if(ImGui::Button("x##remove_all_date_formats")){
        //        node = pm->date_formats;
        //        for(s32 c_idx=0; c_idx < pm->date_formats_count; ++c_idx){
        //            node = node->next;
        //            dll_remove(node);
        //            pool_free(pm->csv_pool, node);
        //            node = pm->date_formats;
        //        }
        //        dll_clear(pm->date_formats);
        //        pm->date_formats_count = 0;
        //    }
        //    tooltip(str8_literal("Remove all date formats."));

        //    node = pm->date_formats;
        //    for(s32 c_idx=0; c_idx < pm->date_formats_count; ++c_idx){
        //        node = node->next;
        //        unique_fmt = str8_fmt(tm->frame_arena, "##CSV_Description%i\n", c_idx);
        //        ImGui::InputText((char*)unique_fmt.data, node->name, 1024);

        //        ImGui::SameLine();
        //        ImGui::SetCursorPosX(ImGui::GetColumnOffset(-1) + csv_date_x_button_start);
        //        unique_fmt = str8_fmt(tm->frame_arena, "x##remove_date_format%i\n", c_idx);
        //        if(ImGui::Button((char*)unique_fmt.data)){
        //            dll_remove(node);
        //            pool_free(pm->csv_pool, node);

        //            --pm->description_names_count;
        //        }
        //        tooltip(str8_literal("Remove date format."));
        //    }
        //    ImGui::EndChild();

        //    ImGui::BeginChild("Child7", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);
        //    if(ImGui::Button("Okay")){
        //        //serialize_config();
        //        ImGui::CloseCurrentPopup();
        //    }
        //    tooltip(str8_literal("Submit changes."));
        //    ImGui::EndChild();

        //    if(controller_button_pressed(KeyCode_ESCAPE, true)){
        //        //serialize_config();
        //        ImGui::CloseCurrentPopup();
        //    }
        //    ImGui::EndPopup();
        //}

        // note: popluate empty amount's in transactions with 0's for visual appeal
        Transaction* trans = month->transactions;
        for(s32 t_idx = 0; t_idx < month->transactions_count; ++t_idx){
            trans = trans->next;

            if(trans->amount[0] == 0){
                trans->amount[0] = '0';
                trans->amount[1] = '\0';
            }
        }

        {
            // note: collect selection options
            pm->selection_count = 1;
            Category* category = pm->month_categories;
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
        }

        // note: render transactions
        ImGui::BeginChild("transactions_child", ImVec2(0, 0), true, 0);
        trans = month->transactions;
        f32 minus_padding = 8;
        for(s32 t_idx=0; t_idx < month->transactions_count; ++t_idx){
            trans = trans->next;

            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + hash_column_start - minus_padding);
            ImGui::PushID(t_idx);
            String8 num_button = str8_formatted(scratch.arena, "%i", t_idx + 1);
            ImGui::Button((char*)num_button.data);
            ImGui::PopID();
            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)){
                ImGui::SetDragDropPayload("DRAG_ROW", &t_idx, sizeof(s32));
                ImGui::Text("%i", t_idx);
                ImGui::EndDragDropSource();
            }
            tooltip(str8_literal("Drag/Swap Transaction."));
            if(ImGui::BeginDragDropTarget()){
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_ROW")){
                    s32* payload_data = (s32*)payload->Data;
                    s32 from_idx = *payload_data;
                    if(from_idx != t_idx){
                        Transaction* t = month->transactions;
                        for(s32 i=0; i <= from_idx; ++i){
                            t = t->next;
                        }
                        dll_swap(t, trans, Transaction);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + date_column_start - minus_padding);
            ImGui::PushItemWidth(date_column_width);
            String8 date_id = str8_formatted(scratch.arena, "##date%i", t_idx);
            ImGui::InputText((char*)date_id.data, trans->date, TRANS_DATE_SIZE, ImGuiInputTextFlags_CharsDecimal);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + amount_column_start - minus_padding);
            ImGui::PushItemWidth(amount_column_width);
            String8 amount_id = str8_formatted(scratch.arena, "##amount%i", t_idx);
            ImGui::InputText((char*)amount_id.data, trans->amount, TRANS_AMOUNT_SIZE, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + description_column_start - minus_padding);
            ImGui::PushItemWidth(description_column_width);
            String8 desc_id = str8_formatted(scratch.arena, "##description%i", t_idx);
            ImGui::InputText((char*)desc_id.data, trans->description, TRANS_DESC_SIZE);

            ImGui::PopItemWidth();

            // note: selection box
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(category_select_column_width);

                // make selection box not transparent
                ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetColorU32(combo_popup_background_color));

                // color selection red if not found in category names
                ImVec4 frame_bg_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
                bool found = false;
                char space[] = " ";
                if(!char_compare(trans->selection, space)){
                    Category* category = pm->month_categories;
                    for(s32 c_idx = 0; c_idx < pm->categories_count && !found; ++c_idx){
                        category = category->next;

                        Row* row = category->rows;
                        for(s32 r_idx = 0; r_idx < category->row_count && !found; ++r_idx){
                            row = row->next;
                            String8 trans_selection = str8(trans->selection, char_length(trans->selection));

                            String8 cat_part = str8_format(tm->frame_arena, "%s: ", category->name);
                            String8 name_part = str8(row->name, char_length(row->name));
                            String8 cat_row_name = str8_concatenate(tm->frame_arena, cat_part, name_part);

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
                s32 color_idx = 0;
                String8 combo_id = str8_formatted(scratch.arena, "##category_select%i", t_idx);
                if(ImGui::BeginCombo((char*)combo_id.data, trans->selection, ImGuiComboFlags_HeightLarge)){
                    for (int n = 0; n < pm->selection_count; n++){
                        String8 selection_item = pm->selection_list[n];
                        String8 trans_selection = str8(trans->selection, char_length(trans->selection));

                        ImDrawList* draw_list = ImGui::GetWindowDrawList();
                        ImVec2 min = ImGui::GetCursorScreenPos();
                        ImVec2 max = ImVec2(min.x + ImGui::GetContentRegionAvail().x, min.y + ImGui::GetTextLineHeightWithSpacing());

                        if(n != 0){
                            if(last_combo_name.count == 0){
                                last_combo_name = selection_item;
                                draw_list->AddRectFilled(min, max, combo_popup_alternating_colors[color_idx % 2]);
                            }
                            else{
                                s64 idx = byte_index_from_left(last_combo_name, ':');
                                String8Node* split_node1 = str8_split(tm->frame_arena, last_combo_name, ':');
                                String8Node* split_node2 = str8_split(tm->frame_arena, selection_item, ':');
                                if(!str8_compare(split_node1->next->str, split_node2->next->str)){
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
            ImGui::SetCursorPosX(ImGui::GetColumnOffset(1) + x_expense_column_start - minus_padding);
            String8 delete_id = str8_formatted(scratch.arena, "x##remove_transaction%i", t_idx);
            if(ImGui::Button((char*)delete_id.data)){
                --month->transactions_count;
                --year->total_transactions;

                dll_remove(trans);
                pool_free(pm->transaction_pool, trans);
            }
            tooltip(str8_literal("Delete Transaction."));

            ImGui::SameLine();
            ImGui::PushID(t_idx);
            if(trans->muted){
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
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
        }
        ImGui::EndChild();



        ImGui::EndChild();

        end_scratch(scratch);
        ImGui::End();
}

static void
collect_totals_for_months(void){
    // note: collect totals info for all months
    for(s32 m_idx = 0; m_idx < Month_Count; ++m_idx){
        MonthInfo* month = pm->year->months + m_idx;

        // note: collect row->spent from transactions
        Category* category = pm->month_categories;
        for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
            category = category->next;

            Row* row = category->rows;
            for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                row = row->next;
                row->spent = 0;

                if(!month->muted){
                    Transaction* trans = month->transactions;
                    for(s32 t_idx = 0; t_idx < month->transactions_count; ++t_idx){
                        trans = trans->next;
                        if(!trans->muted){
                            u32 t_length = char_length(trans->selection);
                            String8 trans_selection = str8(trans->selection, t_length);

                            u32 r_length = char_length(row->name);
                            String8 cat_part = str8_format(tm->frame_arena, "%s: ", category->name);
                            String8 name_part = str8(row->name, r_length);
                            String8 full = str8_concatenate(tm->frame_arena, cat_part, name_part);
                            if(str8_compare(full, trans_selection)){
                                f32 amount = atof(trans->amount);
                                row->spent += amount;
                                row->spent = round_to_hundredth(row->spent);
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
        category = pm->month_categories;
        for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
            category = category->next;

            category->planned = 0;
            category->spent = 0;
            category->diff = 0;

            Row* row = category->rows;
            for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                row = row->next;
                if(!row->muted){
                    row->diff = atof(row->planned) - row->spent;
                    row->diff = round_to_hundredth(row->diff);

                    category->planned += atof(row->planned);
                    category->spent += row->spent;
                    category->diff += row->diff;
                }
            }
            category->planned = round_to_hundredth(category->planned);
            category->spent  = round_to_hundredth(category->spent);
            category->diff    = round_to_hundredth(category->diff);
            if(!category->muted){
                month_total_planned += category->planned;
                month_total_spent   += category->spent;
                month_total_diff    += category->diff;
            }
        }
        if(!month->muted){
            month->totals.planned = round_to_hundredth(month_total_planned);
            month->totals.spent   = round_to_hundredth(month_total_spent);
            month->totals.diff    = round_to_hundredth(month_total_diff);
            month->totals.saved   = round_to_hundredth(atof((char*)pm->budget.str) - month->totals.spent);
            month->totals.goal    = round_to_hundredth(atof((char*)pm->budget.str) - month->totals.planned);
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
                totals->saved   += atof((char*)pm->budget.str) - month->totals.spent;
                totals->goal    += atof((char*)pm->budget.str) - month->totals.planned;
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
                totals->saved   += atof((char*)pm->budget.str) - month->totals.spent;
                totals->goal    += atof((char*)pm->budget.str) - month->totals.planned;
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
            pm->annual_totals.saved   += atof((char*)pm->budget.str) - month->totals.spent;
            pm->annual_totals.goal    += atof((char*)pm->budget.str) - month->totals.planned;
        }
    }
    pm->annual_totals.planned = round_to_hundredth(pm->annual_totals.planned);
    pm->annual_totals.spent   = round_to_hundredth(pm->annual_totals.spent);
    pm->annual_totals.diff    = round_to_hundredth(pm->annual_totals.diff);
    pm->annual_totals.saved   = round_to_hundredth(pm->annual_totals.saved);
    pm->annual_totals.goal    = round_to_hundredth(pm->annual_totals.goal);

    {
        // note: calculate selected months row->spent
        Category* category = pm->month_categories;
        for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
            category = category->next;

            Row* row = category->rows;
            for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                row = row->next;
                row->spent = 0;

                MonthInfo* month = pm->year->months + pm->month_tab_idx;
                if(!month->muted){
                    Transaction* trans = month->transactions;
                    for(s32 t_idx = 0; t_idx < month->transactions_count; ++t_idx){
                        trans = trans->next;
                        if(!trans->muted){
                            u32 t_length = char_length(trans->selection);
                            String8 trans_selection = str8(trans->selection, t_length);

                            u32 r_length = char_length(row->name);
                            String8 cat_part = str8_format(tm->frame_arena, "%s: ", category->name);
                            String8 name_part = str8(row->name, r_length);
                            String8 full = str8_concatenate(tm->frame_arena, cat_part, name_part);
                            if(str8_compare(full, trans_selection)){
                                f32 amount = atof(trans->amount);
                                row->spent += amount;
                                row->spent = round_to_hundredth(row->spent);
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
        category = pm->month_categories;
        for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
            category = category->next;

            category->planned = 0;
            category->spent = 0;
            category->diff = 0;

            Row* row = category->rows;
            for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
                row = row->next;
                if(!row->muted){
                    row->diff = atof(row->planned) - row->spent;
                    row->diff = round_to_hundredth(row->diff);

                    category->planned += atof(row->planned);
                    category->spent += row->spent;
                    category->diff += row->diff;
                }
            }
            category->planned = round_to_hundredth(category->planned);
            category->spent  = round_to_hundredth(category->spent);
            category->diff    = round_to_hundredth(category->diff);
            if(!category->muted){
                month_total_planned += category->planned;
                month_total_spent  += category->spent;
                month_total_diff    += category->diff;
            }
        }
        MonthInfo* month = pm->year->months + pm->month_tab_idx;
        month->totals.planned = round_to_hundredth(month_total_planned);
        month->totals.spent   = round_to_hundredth(month_total_spent);
        month->totals.diff    = round_to_hundredth(month_total_diff);
        month->totals.saved   = round_to_hundredth(atof((char*)pm->budget.str) - month->totals.spent);
        month->totals.goal    = round_to_hundredth(atof((char*)pm->budget.str) - month->totals.planned);
    }

    // note mute/unmute category based on rows muted.
    Category* category = pm->month_categories;
    for(s32 c_idx = 0; c_idx < pm->categories_count; ++c_idx){
        category = category->next;

        bool all_muted = true;
        Row* row = category->rows;
        for(s32 r_idx = 0; r_idx < category->row_count; ++r_idx){
            row = row->next;
            if(!row->muted){
                all_muted = false;
            }
        }

        category->muted = all_muted;
    }

    // note mute/unmute months based on transactions muted.
    for(s32 m_idx=0; m_idx < Month_Count; ++m_idx){
        MonthInfo* month = pm->year->months + m_idx;

        bool all_muted = true;
        Transaction* trans = month->transactions;
        for(s32 t_idx = 0; t_idx < month->transactions_count; ++t_idx){
            trans = trans->next;
            if(!trans->muted){
                all_muted = false;
            }
        }

        month->muted = all_muted;
    }

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
}

static void
do_one_frame(void){
    while(!events_empty(&events)){
        Event event = events_next(&events);

        if(event.type == EventType_KEYBOARD){
            if(event.keycode == KeyCode_ESCAPE){
                //should_quit = true;
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

    //prs32("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f - simulations: %i\n", FPS, MSPF, t_clock.dt, accumulator, frame_time, second_elapsed, simulations);
    String8 fps = str8_formatted(tm->frame_arena, "FPS: %.2f", FPS);

    //------------------------------------------------------------------------------------------------------------
    // DRAW ENTIRE UI
    draw_entire_ui();
    //ImGui::ShowDemoWindow(); // Show demo window! :)

    collect_totals_for_months();

    {
        d3d_context->ClearRenderTargetView(d3d_framebuffer_view, BACKGROUND_COLOR.e);
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        d3d_swapchain->Present(1, 0);
    }
    // todo(rr): maybe remove?
    //if(controller_button_pressed(KeyCode_ESCAPE, true)){
    //    should_quit = true;

    //}
    clear_controller_pressed();

    // NOTE IMPORTANT(rr): WE SERIALIZE ALL THE TIME
    serialize_config();
    serialize_budget();
    Year* year;
    for(s32 y_idx=0; y_idx < MAX_YEAR_COUNT; ++y_idx){
        year = pm->years + y_idx;
        serialize_year(year);
    }

    arena_free(tm->frame_arena);
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

        show_cursor(true);

        // create pools
        pm->category_pool    = push_pool(&pm->arena, sizeof(Category), 256);
        pm->row_pool         = push_pool(&pm->arena, sizeof(Row), 2048);
        pm->transaction_pool = push_pool(&pm->arena, sizeof(Transaction), 8192);
        pm->csv_pool         = push_pool(&pm->arena, sizeof(CSVColumnNode), 128);
        pm->csv_profile_pool = push_pool(&pm->arena, sizeof(CSV_Profile), 32);
        // todo(rr): maybe I can just use scratch memory? I don't think I need this
        pm->data_arena       = push_arena(&pm->arena, MB(1));

        // setup free list from pools
        pool_free_all(pm->category_pool);
        pool_free_all(pm->row_pool);
        pool_free_all(pm->transaction_pool);
        pool_free_all(pm->csv_pool);
        pool_free_all(pm->csv_profile_pool);

        // setup sentinel node or categories
        pm->month_categories = (Category*)pool_next(pm->category_pool);
        dll_clear(pm->month_categories);
        pm->quarter_categories = (Category*)pool_next(pm->category_pool);
        dll_clear(pm->quarter_categories);
        pm->biannual_categories = (Category*)pool_next(pm->category_pool);
        dll_clear(pm->biannual_categories);
        pm->annual_categories = (Category*)pool_next(pm->category_pool);
        dll_clear(pm->annual_categories);

        // setup sentinel node for CSV Columns
        pm->date_names = (CSVColumnNode*)pool_next(pm->csv_pool);
        dll_clear(pm->date_names);
        pm->amount_names = (CSVColumnNode*)pool_next(pm->csv_pool);
        dll_clear(pm->amount_names);
        pm->description_names = (CSVColumnNode*)pool_next(pm->csv_pool);
        dll_clear(pm->description_names);
        pm->date_formats = (CSVColumnNode*)pool_next(pm->csv_pool);
        dll_clear(pm->date_formats);
        pm->csv_profile = (CSV_Profile*)pool_next(pm->csv_profile_pool);
        dll_clear(pm->csv_profile);

        // give selection list memory
        pm->selection_list = push_array(tm->options_arena, String8, 1024);
        for(s32 i=0; i < SELECTION_LIST_SIZE; ++i){
            String8* option = pm->selection_list + i;
            option->str = push_array(tm->options_arena, u8, 128);
        }
        *pm->selection_list = str8(" \0", 2);
        pm->default_path = os_application_path(&pm->arena);

        pm->budget.data = push_array(global_arena, u8, 128);
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
        deserialize_config();
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

        // load budget
        deserialize_budget();
        pm->quarter_tab_flags[pm->quarter_tab_idx] = ImGuiTabItemFlags_SetSelected;
        pm->biannual_tab_flags[pm->biannual_tab_idx] = ImGuiTabItemFlags_SetSelected;

        // colors
        combo_popup_background_color = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
        combo_popup_background_color.w = 1;
        combo_popup_alternating_colors[0] = ImColor(10, 10, 10, 255);
        combo_popup_alternating_colors[1] = ImColor(20, 20, 20, 255);

        // load assets
        Bitmap bm = stb_load_image(&pm->arena, sprites_path, str8_literal("gear_new.png"));
        d3d_init_texture_resource(&gear_texture.view, &bm);

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

    if(should_quit){
        if(!os_path_exists(saves_path)){
            os_dir_create(saves_path);
        }
        serialize_config();
        serialize_budget();
        Year* year;
        for(s32 y_idx=0; y_idx < MAX_YEAR_COUNT; ++y_idx){
            year = pm->years + y_idx;
            serialize_year(year);
        }
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    d3d_release();
    end_profiler();

    return(0);
}

