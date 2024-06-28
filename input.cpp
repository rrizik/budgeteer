#ifndef INPUT_C
#define INPUT_C

// consider: maybe I don't need this. array_count() can be computed in place
static void
events_init(Events* events){
    events->size = array_count(events->e);
}

static u32
events_count(Events* events){
    u32 result = events->write - events->read;
    return(result);
}

static bool
events_full(Events* events){
    bool result = (events_count(events) == events->size);
    return(result);
}

static bool
events_empty(Events* events){
    bool result = (events->write == events->read);
    return(result);
}

static u32
mask(Events* events, u32 idx){
    u32 result = idx & (events->size - 1);
    return(result);
}

static void
events_add(Events* events, Event event){
    assert(!events_full(events));

    u32 masked_idx = mask(events, events->write++);
    events->e[masked_idx] = event;
}

static Event
events_next(Events* events){
    assert(!events_empty(events));

    u32 masked_idx = mask(events, events->read++);
    Event event = events->e[masked_idx];
    return(event);
}

static void
clear_controller_pressed(){
    for(s32 i=0; i < KeyCode_Count; ++i){
        controller.button[i].pressed = false;
    }
}

#endif
