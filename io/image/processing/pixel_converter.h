typedef struct camera_params {
    char data[32];
    int field_20;
    char padding1[4];
    int field_28;
    int field_2C;
} camera_params;

typedef struct image_format {
    int format_type;
} image_format;

typedef struct source_region {
    int x;
    int y;
    int width;
    int height;
} source_region;

typedef struct dest_region {
    int x;
    int y;
    int width;
    int height;
} dest_region;

