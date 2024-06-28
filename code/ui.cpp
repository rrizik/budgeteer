#ifndef UI_C
#define UI_C

static UI_Layout*
ui_make_layout(Arena* arena, v2 pos, String8 str, UI_LayoutFlags flags){
    UI_Layout* result = push_struct(arena, UI_Layout);
    if(top_layout != 0){
        UI_Layout* top_layout = get_top_layout();
        result->parent = top_layout;
        if(top_layout->first == 0){
            top_layout->first = result;
            top_layout->last = result;
        }
        else{
            top_layout->last->next = result;
            result->prev = top_layout->last;
            top_layout->last = result;
        }
    }

    result->pos = pos;
    result->flags = flags;
    result->str = str;

    return(result);
}

static bool
ui_button(Arena* arena, String8 str){
    UI_Layout* result = push_struct(arena, UI_Layout);
    UI_Layout* top_layout = get_top_layout();
    result->parent = top_layout;
    if(top_layout->first == 0){
        top_layout->first = result;
        top_layout->last = result;
    }
    else{
        top_layout->last->next = result;
        result->prev = top_layout->last;
        top_layout->last = result;
    }

    result->str = str;
    // look at current layout
    // draw based on its position
    // draw based on siblings
    // what was the event
    // act accordingly
    return(true);
}

static void
traverse_ui(UI_Layout* node){
    if(node == 0){
        return;
    }

    if(node->first != 0){
        traverse_ui(node->first);
    }

    print("node: %s\n", node->str.str);

    UI_Layout* node_next = node->next;
    traverse_ui(node_next);
}

static void
traverse_ui_reverse(Arena* arena, UI_Layout* node, u32 font){
    if(node == 0){
        return;
    }

    if(node->last != 0){
        traverse_ui_reverse(arena, node->last, font);
    }

    UI_Layout* node_prev = node->prev;
    traverse_ui_reverse(arena, node_prev, font);
}

static void
push_layout(Arena* arena, UI_Layout* layout){
    LayoutNode* node = push_struct(arena, LayoutNode);
    node->layout = layout;
    node->next = top_layout;
    top_layout = node;
}

static void
pop_layout(){
    top_layout = top_layout->next;
}

static UI_Layout*
get_top_layout(){
    return(top_layout->layout);
}

#endif
