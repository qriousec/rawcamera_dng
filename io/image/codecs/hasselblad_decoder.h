typedef struct hasselblad_unpacker_context {
    void* vtable;                      // 0
    char padding1[24];                 // 8
    void* data_buffer;                 // 32
    char padding2[12];                 // 40
    uint32_t width;                    // 44
    uint32_t height;                   // 48
    char padding3[144];                // 52
} hasselblad_unpacker_context;

typedef struct huffman_decode_tables {
    void* dc_table;                    // 0
    void* ac_table;                    // 8
} huffman_decode_tables;

typedef struct jpeg_marker_info {
    uint16_t marker;                   // 0
    uint16_t length;                   // 2
    char data[];                       // 4
} jpeg_marker_info;

