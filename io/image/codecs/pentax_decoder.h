typedef struct CameraDimensions {
    int width;
    int height;
} CameraDimensions;

typedef struct CameraProcessor {
    void* vtable;
    char padding1[16];
    void* data_buffer;
    char padding2[176];
    void* data_start;
    void* data_end;
    char padding3[32];
    CameraDimensions dimensions;
    char padding4[4];
    int camera_id1;
    int camera_id2;
} CameraProcessor;

typedef struct HuffmanDecodeResult {
    int value1;
    int value2;
} HuffmanDecodeResult;

