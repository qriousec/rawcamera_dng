typedef struct camera_info {
    char data[32];
    int flag;
    char padding[4];
    int offset1;
    int offset2;
    char more_data[16];
} camera_info;

typedef struct image_params {
    int param1;
    int channels;
    char data[32];
} image_params;

typedef struct crop_rect {
    int x;
    int y;
    int width;
    int height;
} crop_rect;

typedef struct output_rect {
    int x;
    int y;
    int width;
    int height;
} output_rect;

