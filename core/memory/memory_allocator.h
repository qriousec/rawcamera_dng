typedef struct data_processor {
    void* vtable_ptr;
    char padding[16];
    void* handler_ptr;
} data_processor;

