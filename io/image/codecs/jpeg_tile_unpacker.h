typedef struct source_region {
    char data[32];
} source_region;

typedef struct dest_region {
    char data[32];
} dest_region;

typedef struct camera_params {
    char data[64];
} camera_params;

typedef struct image_format {
    char data[48];
} image_format;

typedef struct huffman_decoder_state {
    char padding_0[4];
    uint32_t bit_position;
    uint32_t width;
    uint32_t height;
    uint32_t component_count;
    uint32_t predictor_type;
    uint32_t padding_20[3];
    uint32_t restart_interval;
    char padding_36[132];
    uint16_t quantization_tables[4];
    char padding_142[312];
} huffman_decoder_state;

typedef struct jpeg_block_handler {
    void* vtable;
    char data[16];
} jpeg_block_handler;

typedef struct huffman_tables {
    void* tables[4];
} huffman_tables;

typedef struct tile_params {
    uint32_t width;
    uint32_t height;
    uint32_t total_width;
    uint32_t total_height;
} tile_params;

