typedef struct JPEGProcessor {
    void* vtable;
    char padding1[16];
    void* data_reader;
    char padding2[12];
    int height;
    int width;
    char padding3[148];
    void* segments_start;
    void* segments_end;
    char padding4[24];
    void* color_matrix;
    char padding5[20];
    int color_space_mode;
    int demosaic_flag;
    char padding6[4];
} JPEGProcessor;

