typedef struct raw_camera_vtable {
    void *pad0[4];
    int (*get_parameter)(void *this);
} raw_camera_vtable;

typedef struct raw_camera {
    raw_camera_vtable *vtable;
} raw_camera;

typedef struct raw_camera_vtable {
    void *pad0[4];
    int (*get_parameter)(void *this);
} raw_camera_vtable;

typedef struct raw_camera {
    raw_camera_vtable *vtable;
} raw_camera;

typedef struct RawCameraException {
    void* vtable;
    char message[8];
} RawCameraException;

typedef struct camera_parameter {
    int32_t value;
    int32_t expected_value;
} camera_parameter;

