typedef struct cdng_unpacker {
    void *vtable;
    char padding1[40];
    uint32_t width;
    uint32_t height;
    char padding2[164];
    uint32_t is_compressed;
    char padding3[4];
    uint32_t tile_width;
    uint32_t tile_height;
    uint32_t *tile_offsets_start;
    uint32_t *tile_offsets_end;
    char padding4[24];
    uint32_t *tile_byte_counts_start;
    uint32_t *tile_byte_counts_end;
} cdng_unpacker_t;

typedef struct block_params {
    void *function_ptr;
    void *descriptor;
    void *invoke_ptr;
    void *descriptor_ptr;
    cdng_unpacker_t *unpacker;
} block_params_t;

typedef struct jpeg_decode_context {
    char data[152];
} jpeg_decode_context;

typedef struct jpeg_image_info {
    int width;
    int height;
    int components;
    int precision;
    char reserved[32];
} jpeg_image_info;

typedef struct buffer_info {
    void* buffer_ptr;
    size_t buffer_size;
} buffer_info;

typedef struct decode_buffer {
    void* data_ptr;
    size_t size;
} decode_buffer;

