typedef struct huffman_entry {
    uint32_t start_code;
    uint8_t code_length;
    uint8_t symbol;
    uint16_t padding;
} huffman_entry;

typedef struct decoder_context {
    char pad0[24];
    void *vtable_ptr;
    char pad1[20];
    uint32_t width;
    uint32_t height;
    char pad2[164];
    uint32_t compression_type;
    char pad3[4];
    uint32_t pixel_format;
    char pad4[4];
    uint32_t *buffer_start;
    uint32_t *buffer_end;
    char pad5[24];
    uint32_t *data_start;
    char pad6[152];
} decoder_context;

typedef struct huffman_decoder_context {
    void* data_ptr;                    // 0
    void* current_ptr;                 // 8
    void* memory_block;                // 16
    uint32_t total_size;               // 24
    uint32_t current_offset;           // 28
    uint32_t width;                    // 44
    uint32_t height;                   // 48
    void* huffman_tables[4];           // 192
    void* vtable;                      // 0
    char padding1[152];                // 8
    uint32_t component_count;          // 160
    char padding2[88];                 // 164
    uint16_t restart_interval;         // 216
} huffman_decoder_context;

typedef struct jpeg_component_info {
    uint32_t component_id;             // 0
    uint32_t h_sampling;               // 4
    uint32_t v_sampling;               // 8
    uint32_t quantization_table;       // 12
    char padding[8];                   // 16
} jpeg_component_info;

typedef struct huffman_decode_block {
    void* base_ptr;                    // 0
    void* current_ptr;                 // 8
    void* memory_block;                // 16
    uint32_t total_size;               // 24
    char padding1[496];                // 28
    void* huffman_table_ptrs[32];      // 32
    char padding2[176];                // 288
    uint32_t image_width;              // 536
    uint32_t num_threads;              // 540
    uint32_t block_width;              // 544
    uint32_t block_height;             // 548
    uint32_t total_blocks;             // 552
    uint32_t processed_count;          // 556
    char padding3[12];                 // 560
    uint32_t huffman_dc_flag;          // 560
    void* huffman_dc_table1;           // 568
    void* huffman_dc_table2;           // 576
    uint16_t image_height;             // 592
    uint16_t restart_count;            // 594
    uint32_t component_bits;           // 600
    void* image_buffer;                // 584
    char padding4[680];                // 608
    uint32_t sync_counter1;            // 712
    uint32_t mask_value;               // 716
    uint32_t lookup_offset;            // 720
    uint64_t buffer_start;             // 712
    void* buffer_current;              // 720
    char padding5[208];                // 728
    uint64_t bit_buffer;               // 752
    uint64_t bit_count;                // 768
    char padding6[224];                // 776
    void* component_data;              // 832
    void* component_info_ptr;          // 840
    char padding7[16];                 // 848
    uint32_t bit_shift_count;          // 1176
    uint32_t page_mask;                // 1180
    void* aligned_buffer;              // 1192
    void* decode_buffer;               // 1184
    char padding8[16];                 // 1200
} huffman_decode_block;

