#include <stdio.h>
#include <stdlib.h>

#define MICROCOMPUTE_IMPLEMENTATION
#include "microcompute.h"

#define SHADER_FILE "array.spv"
#define BUFFER_LEN 5

static void read_file(const char* path, size_t* length, char* contents) {
    FILE* fp = fopen(path, "rb");

    if (!fp) {
        if (length) *length = 0;
        return;
    }

    fseek(fp, 0, SEEK_END);
    size_t length_internal = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (contents) (void)!fread(contents, 1, length_internal, fp);

    if (length) *length = length_internal;
    fclose(fp);
}

static void debug_cb(
    mc_DebugLevel level,
    const char* source,
    const char* msg,
    void* arg
) {
    (void)!arg;
    printf("[%s from %s]: %s\n", mc_debug_level_to_str(level), source, msg);
}

int main(void) {
    mc_State_t* state = mc_state_create(malloc, free, realloc, debug_cb, NULL);

    size_t shaderLen;
    read_file(SHADER_FILE, &shaderLen, NULL);

    char shader[shaderLen];
    read_file(SHADER_FILE, NULL, shader);

    mc_Program_t* program = mc_program_create(
        state,
        shaderLen,
        (uint32_t*)shader,
        2,
        (uint64_t[]){sizeof(float) * BUFFER_LEN, sizeof(float) * BUFFER_LEN}
    );

    for (uint32_t i = 0; i < mc_program_get_buffer_count(program); i++) {
        float data[BUFFER_LEN];
        printf("in[%d] ", i);
        for (uint32_t j = 0; j < BUFFER_LEN; j++) {
            data[j] = j + i * mc_program_get_buffer_count(program);
            printf("%f, ", data[j]);
        }
        printf("\n");

        mc_program_nth_buffer_write(
            program,
            i,
            0,
            sizeof(float) * BUFFER_LEN,
            data
        );
    }

    mc_program_dispatch(program, (mc_uvec3_t){BUFFER_LEN, 1, 1});

    float data[BUFFER_LEN];
    mc_program_nth_buffer_read(program, 1, 0, sizeof(float) * BUFFER_LEN, data);

    printf("out   ");
    for (uint32_t i = 0; i < BUFFER_LEN; i++) printf("%f, ", data[i]);
    printf("\n");

    mc_program_destroy(program);

    mc_state_destroy(state);
}
