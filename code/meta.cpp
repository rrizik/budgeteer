#include "base.h"


s32 main(){
    ScratchArena scratch = begin_scratch();

    File in = os_file_open(str8_lit("../code/budgeteer.ttf"), GENERIC_READ, OPEN_EXISTING);
    String8 data = os_file_read(scratch.arena, in);

    Arena* buff = make_arena(MB(1));
    buff->at += snprintf((char*)buff->base + buff->at, buff->size - buff->at, "u8 budgeteer_font[] = {\n    ");
    for(int i=0; i < data.size; ++i){
        if(i + 1 == data.size){
            buff->at += snprintf((char*)buff->base + buff->at, buff->size - buff->at, "0x%02X", data.str[i]);
            buff->at += snprintf((char*)buff->base + buff->at, buff->size - buff->at, "\n};\n");
        }
        else{
            buff->at += snprintf((char*)buff->base + buff->at, buff->size - buff->at, "0x%02X, ", data.str[i]);

            if(((i + 1) % 12) == 0){
                buff->at += snprintf((char*)buff->base + buff->at, buff->size - buff->at, "\n    ");
            }
        }

    }

    File out = os_file_open(str8_lit("../code/meta.h"), GENERIC_WRITE, CREATE_ALWAYS);
    os_file_write(out, buff->base, buff->at);

    //FILE* in = fopen("../code/test.ttf", "rb");
    //fseek(in, 0, SEEK_END);
    //int size = ftell(in);
    //fseek(in, 0, SEEK_SET);

    //u8* buffer = (u8*)malloc(size);
    //fread(buffer, 1, size, in);

    //FILE* out = fopen("../code/meta.h", "w");
    //fprintf(out, "u8 my_font[] = {\n    ");

    //for(int i=0; i < size; ++i){
    //    if(i + 1 == size){
    //        fprintf(out, "0x%02X", buffer[i]);
    //    }
    //    else{
    //        fprintf(out, "0x%02X, ", buffer[i]);
    //    }
    //    if(((i + 1)% 12) == 0){
    //        fprintf(out, "\n    ");
    //    }
    //}

    //fprintf(out, "\n}\n");
    return(0);
}
