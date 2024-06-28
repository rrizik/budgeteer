#ifndef WAVE_H
#define WAVE_H

typedef enum ChunkId{
    ChunkId_RIFF,
    ChunkId_WAVE,
    ChunkId_FMT,
    ChunkId_DATA,
    ChunkId_COUNT,
} ChunkId;

String8 chunk_ids[ChunkId_COUNT] = {
    str8_literal("RIFF"),
    str8_literal("WAVE"),
    str8_literal("fmt"),
    str8_literal("data"),
};

typedef struct WaveHeader{
    u8 chunk_id[4];
    u32 chunk_size;
    u8 format[4];
} WaveHeader;

typedef struct ChunkInfo{
    u8 chunk_id[4];
    u32 chunk_size;
} ChunkInfo;

typedef struct WaveFormat{
    u16 audio_format;
    u16 channels;
    u32 samples_per_sec;
    u32 bytes_per_sec;
    u16 block_align;
    u16 bits_per_sample;
    u16 ext_size;
    u16 valid_bits_per_sample;
    u32 channel_mask;
    u32 sub_format[4];
} WaveFormat;

typedef struct Wave{
    WaveFormat format;
    u16* base;
    u32 sample_count; // size in samples, not bytes
} Wave;

static Wave load_wave(Arena* arena, String8 filename);
static void u32_buffer_from_u8_buffer(String8* channel_1, String8* channel_4);

#endif
