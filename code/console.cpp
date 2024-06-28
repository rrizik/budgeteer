#ifndef CONSOLE_C
#define CONSOLE_C

static void
init_console(Arena* arena, u32 font_type){ //note: everything is positioned relative to the output_rect
    console.state = CLOSED;
    console.font_type = font_type;

    // some size constraints
    console.text_left_pad = 10;

    // how much/fast to open
    console.open_dt = 30.0f;
    console.open_t  = 0.0f;
    console.open_t_target = 0.0f;

    console.y_closed   = 0.0f;
    console.y_open     = 0.4f;
    console.y_open_big = 0.7f;

    // some colors
    console.output_background_color = CONSOLE_OUTPUT_BACKGROUND_COLOR;
    console.input_background_color  = CONSOLE_INPUT_BACKGROUND_COLOR;
    console.input_color  = CONSOLE_TEXT_INPUT_COLOR;
    console.output_color = CONSOLE_TEXT_OUTPUT_COLOR;
    console.cursor_color = CONSOLE_CURSOR_COLOR;

    console.output_history_count = 0;
    console.input_history_count = 0;
    console.input_history_index = 0;
    console.input_count = 0;
    console.cursor_index = 0;
}

static bool
console_is_open(){
    return(console.state != CLOSED);
}

static bool
console_is_visible(){
    return(console.open_t > 0);
}

static u8
console_char_at_cursor(){
    u8 result = 0;
    if(console.input_count == console.cursor_index){
        result = 'a';
    }
    else{
        result = console.input[console.cursor_index];
    }
    return(result);
}

static void
input_add_char(u8 c){
    if(console.input_count < INPUT_COUNT_MAX){
        if(console.cursor_index < console.input_count){
            ScratchArena scratch = begin_scratch();

            String8 left = {
                .str = push_array(scratch.arena, u8, (u32)console.cursor_index),
                .size = (u64)console.cursor_index,
            };
            memory_copy(left.str, console.input, left.size);
            String8 right = {
                .str = push_array(scratch.arena, u8, (u32)(console.input_count - console.cursor_index)),
                .size = (u64)(console.input_count - console.cursor_index)
            };
            memory_copy(right.str, console.input + console.cursor_index, right.size);

            u32 index = 0;
            for(u32 i=0; i < left.size; ++i){
                console.input[index++] = left.str[i];
            }
            console.input[index++] = c;
            for(u32 i=0; i < right.size; ++i){
                console.input[index++] = right.str[i];
            }

            console.input_count++;

            end_scratch(scratch);
        }
        else{
            console.input[console.input_count++] = c;
        }
    }
}

static u8
input_remove_char(){
    u8 c = 0;
    if(console.input_count > 0 && console.cursor_index > 0){
        if(console.cursor_index < console.input_count){
            ScratchArena scratch = begin_scratch();

            String8 left = {
                .str = push_array(scratch.arena, u8, (u32)(console.cursor_index-1)),
                .size = (u64)console.cursor_index-1,
            };
            memory_copy(left.str, console.input, left.size);
            String8 right = {
                .str = push_array(scratch.arena, u8, (u32)(console.input_count - console.cursor_index)),
                .size = (u64)console.input_count - (u64)console.cursor_index,
            };
            memory_copy(right.str, console.input + console.cursor_index, right.size);

            console.input_count--;
            c = console.input[console.cursor_index - 1];
            u32 index = 0;
            for(u32 i=0; i < left.size; ++i){
                console.input[index++] = left.str[i];
            }
            for(u32 i=0; i < right.size; ++i){
                console.input[index++] = right.str[i];
            }

            end_scratch(scratch);
        }
        else{
            c = console.input[--console.input_count];
        }
    }
    return(c);
}

static void
console_set_state(ConsoleState state){
    console.state = state;
    switch(state){
        case CLOSED:{
            console.open_t_target = console.y_closed;
            break;
        }
        case OPEN:{
            console.open_t_target = console.y_open;
            break;
        }
        case OPEN_BIG:{
            console.open_t_target = console.y_open_big;
            break;
        }
    }

    console_update();
}

static bool
handle_console_events(Event event){
    if(event.type == TEXT_INPUT){
        if(event.keycode != '`' && event.keycode != '~'){
            u8 c = (u8)event.keycode;
            input_add_char(c);
            console.cursor_index++;
            return(true);
        }
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == KeyCode_HOME){
                console.cursor_index = 0;
            }
            if(event.keycode == KeyCode_END){
                for(s32 i=console.cursor_index; i < console.input_count; ++i){
                    u8 c = console.input[i];
                    console.cursor_index++;
                }
            }
            if(event.keycode == KeyCode_RIGHT){
                if(console.cursor_index < console.input_count){
                    u8 c = console.input[console.cursor_index];
                    console.cursor_index++;
                }
            }
            if(event.keycode == KeyCode_LEFT){
                if(console.cursor_index > 0){
                    console.cursor_index--;
                    u8 c = console.input[console.cursor_index];
                }
            }
            if(event.keycode == KeyCode_BACKSPACE){
                if(console.cursor_index > 0){
                    u8 c = input_remove_char();

                    console.cursor_index--;
                }
                return(true);
            }
            if(event.keycode == KeyCode_UP){
                if(console.input_history_index < console.input_history_count){
                    // clear input
                    console.cursor_index = 0;
                    console.input_count = 0;
                    console.input_history_index++;
                    String8 command = console.input_history[console.input_history_count - console.input_history_index];
                    for(u32 i=0; i < command.size; ++i){
                        u8 c = command.str[i];
                        input_add_char(c);
                        console.cursor_index++;
                    }
                    console.input_count = (s32)command.size;
                }
            }
            if(event.keycode == KeyCode_DOWN){
                if(console.input_history_index > 0){
                    // clear input
                    console.cursor_index = 0;
                    console.input_count = 0;
                    console.input_history_index--;
                    String8 command = console.input_history[console.input_history_count - console.input_history_index];
                    for(u32 i=0; i < command.size; ++i){
                        u8 c = command.str[i];
                        input_add_char(c);
                        console.cursor_index++;
                    }
                    console.input_count = (s32)command.size;
                }
            }
            if(event.keycode == KeyCode_ENTER){
                u8* line_u8 = (u8*)push_array(global_arena, u8, console.input_count + 1);
                memory_copy(line_u8, console.input, (size_t)console.input_count);

                String8 line_str8 = {line_u8, (u64)console.input_count};
                line_str8 = str8_eat_whitespace(line_str8);

                u64 args_count = parse_line_args(line_str8);
                if(!args_count){ return(false); }

                console.input_history[console.input_history_count++] = line_str8;
                run_command(line_str8);

                // clear_input
                console.cursor_index = 0;
                console.input_count = 0;
                console.input_history_index = 0;

                return(true);
            }
        }
    }
    return(false);
}

static void
console_update(){
    f32 open_d = console.open_dt * (f32)clock.dt;
    if(console.open_t < console.open_t_target){
        console.open_t = lerp(console.open_t, console.open_t_target, smoothstep(open_d));
        //console.open_t += open_d;
        if(console.open_t > console.open_t_target){
            console.open_t = console.open_t_target;
        }
    }
    else if(console.open_t > console.open_t_target){
        console.open_t = lerp(console.open_t, console.open_t_target, smoothstep(open_d));
        //console.open_t -= open_d;
        if(console.open_t < 0){
            console.open_t = 0;
        }
    }
}

static void
console_draw(){
    begin_timed_function();
    if(console_is_visible()){
        Font* font = &tm->assets.fonts[console.font_type];

        // rect setup
        f32 y = (f32)(-font->vertical_offset);
        //f32 y = -((f32)font->vertical_offset * font->scale);
        v2 output_p0 = make_v2(0                , y);
        v2 output_p1 = make_v2((f32)window.width, y);
        v2 output_p2 = make_v2((f32)window.width, (console.open_t * (f32)window.height) + y);
        v2 output_p3 = make_v2(0                , (console.open_t * (f32)window.height) + y);

        v2 input_p0 = make_v2(0                , output_p2.y);
        v2 input_p1 = make_v2((f32)window.width, output_p2.y);
        v2 input_p2 = make_v2((f32)window.width, output_p2.y + (f32)font->vertical_offset);
        v2 input_p3 = make_v2(0                , output_p2.y + (f32)font->vertical_offset);

        String8 str = str8(console.input, (u64)console.cursor_index);

        v2 cursor_p0 = make_v2(console.text_left_pad + font_char_width(console.font_type, '>') + font_string_width(console.font_type, str), input_p0.y);
        v2 cursor_p1 = make_v2(cursor_p0.x + font_char_width(console.font_type, console_char_at_cursor()), input_p0.y);
        v2 cursor_p2 = make_v2(cursor_p0.x + font_char_width(console.font_type, console_char_at_cursor()), input_p2.y);
        v2 cursor_p3 = make_v2(console.text_left_pad + font_char_width(console.font_type, '>') + font_string_width(console.font_type, str), input_p2.y);

        // draw regions
        draw_quad(tm->render_command_arena, output_p0, output_p1, output_p2, output_p3, console.output_background_color);
        draw_quad(tm->render_command_arena, input_p0, input_p1, input_p2, input_p3, console.input_background_color);
        draw_quad(tm->render_command_arena, cursor_p0, cursor_p1, cursor_p2, cursor_p3, console.cursor_color);

        // draw input
        f32 input_pos_y = input_p2.y + ((f32)font->descent * font->scale);
        draw_text(tm->render_command_arena, console.font_type, str8_literal(">"), make_v2(console.text_left_pad, input_pos_y), console.input_color);
        if(console.input_count > 0){
            String8 input_str = str8(console.input, (u64)console.input_count);
            draw_text(tm->render_command_arena, console.font_type, input_str, make_v2(console.text_left_pad + font_char_width(console.font_type, '>'), input_pos_y), console.input_color);
        }

        // draw history (in reverse order and only if its on screen)
        if(console.output_history_count > 0){
            f32 output_pos_y = output_p2.y + ((f32)font->descent * font->scale);
            for(s32 i=console.output_history_count-1; i >= 0; --i){
                if(output_pos_y < (f32)window.height){
                    String8 next_string = console.output_history[i];
                    draw_text(tm->render_command_arena, console.font_type, next_string, make_v2(console.text_left_pad, output_pos_y), console.output_color);
                    output_pos_y -= (f32)font->vertical_offset;
                }
            }
        }
    }
};

#endif
