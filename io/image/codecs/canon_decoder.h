typedef struct canon_compressed_unpacker {
    void* vtable;
    char padding1[24];
    void* data_source;
    char padding2[16];
    int32_t parameter1;
    int32_t width;
    char padding3[140];
    int32_t huffman_table_index;
    uint8_t bytes_per_pixel;
    uint8_t processing_flag;
    char padding4[62];
} canon_compressed_unpacker;

typedef struct huffman_decoder_table {
    uint16_t entries[512];
} huffman_decoder_table;

typedef struct image_buffer {
    int16_t pixel_data[64];
} image_buffer;

