#ifndef WAVE_CPP
#define WAVE_CPP

// todo: samples to bytes - bytes to samples. Maybe
static Wave
load_wave(Arena* arena, String8 filename){
    Wave result = {0};

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_concatenate(scratch.arena, build_path, filename);
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    end_scratch(scratch);
    assert_h(file.handle);

    String8 data = os_file_read(arena, file);
    WaveHeader* header = (WaveHeader*)data.str;

    u8* chunks = data.str + sizeof(WaveHeader);
    u32 chunk_inc = 0;
    bool data_found = false;
    bool fmt_found = false;

    while(!data_found || !fmt_found){
        ChunkInfo* c = (ChunkInfo*)(chunks + chunk_inc);
        if(str8_compare(str8(c->chunk_id, 4), chunk_ids[ChunkId_RIFF])){
        }
        else if(str8_compare(str8(c->chunk_id, 4), chunk_ids[ChunkId_WAVE])){
        }
        else if(str8_compare(str8(c->chunk_id, 3), chunk_ids[ChunkId_FMT])){
            WaveFormat* format = (WaveFormat*)((u8*)c + sizeof(ChunkInfo));
            result.format = *format;
            fmt_found = true;
        }
        else if(str8_compare(str8(c->chunk_id, 4), chunk_ids[ChunkId_DATA])){
            result.sample_count = c->chunk_size / result.format.block_align; // convert size from bytes to samples
            result.base = push_array(arena, u16, c->chunk_size);
            memcpy(result.base, (u16*)((u8*)c + sizeof(ChunkInfo)), c->chunk_size);
            data_found = true;
        }
        else if((u8*)c > (data.str + data.size)){
            // todo: log error - failed to load wafvff file
            print("Error: Failed to load WAVE file\n");
            result = {0};
            os_file_close(file);
            return(result);
        }
        chunk_inc += c->chunk_size + sizeof(ChunkInfo);
    }
    os_file_close(file);

    return(result);
}

static void
u32_buffer_from_u8_buffer(String8* u8_buffer, String8* u32_buffer){
    u32* base_rgba = (u32*)u32_buffer->str;
    u8* base_a = (u8*)u8_buffer->str;

    for(s32 i=0; i < u8_buffer->size; ++i){
        //*base_rgba = (u32)(*base_a << 24 | *base_a << 16 | *base_a << 8  | *base_a << 0);
        *base_rgba = (u32)(*base_a << 24 | 255 << 16 | 255 << 8  | 255 << 0);
        base_rgba++;
        base_a++;
    }
}

#endif
