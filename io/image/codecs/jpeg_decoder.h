typedef struct jpeg_decoder_context {
    void* vtable;                    // 0 bytes
    char padding1[24];               // 8 bytes  
    void* data_buffer;               // 24 bytes
    char padding2[20];               // 32 bytes
    uint32_t width;                  // 44 bytes
    uint32_t height;                 // 48 bytes
    char padding3[144];              // 52 bytes
} jpeg_decoder_context;

