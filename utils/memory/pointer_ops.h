typedef struct exception_info {
    void* vtable_ptr;
    char data[8];
} exception_info;

typedef struct unknown_struct {
    char padding[24];
    void *ptr_field;
} unknown_struct;

typedef struct unknown_struct {
    char padding[72];
    void* field_at_offset_72;
} unknown_struct;

typedef struct unknown_struct {
    char padding0[24];
    void* field_at_offset_24;
} unknown_struct;

