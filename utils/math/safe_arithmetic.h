typedef struct exception_info {
    void* vtable_ptr;
    char data[8];
} exception_info;

typedef struct object_with_vtable {
    void* vtable_ptr;
    char data[144];
} object_with_vtable;

typedef struct exception_info {
    void* vtable_ptr;
    char data[8];
} exception_info;

typedef struct bounds_check_context {
    int32_t value;
} bounds_check_context;

typedef struct exception_info {
    char message[16];
} exception_info;

typedef struct RawCameraException {
    char padding[16];
} RawCameraException;

