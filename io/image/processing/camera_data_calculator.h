typedef struct camera_object {
    void* vtable_ptr;
    char data[136];
    void* weak_ptr;
} camera_object;

typedef struct raw_camera_vtable {
    void *pad0[4];
    int (*get_parameter)(void *this);
} raw_camera_vtable;

typedef struct raw_camera {
    raw_camera_vtable *vtable;
    char pad8[16];
    int byte_order;
} raw_camera;

