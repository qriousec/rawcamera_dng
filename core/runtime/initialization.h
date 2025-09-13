typedef struct dispatch_once_block {
    double padding1;
    void (*function_ptr)(void*);
    void *context;
    void *captured_object;
} dispatch_once_block;

typedef struct singleton_object {
    char padding[288];
    void *cached_value;
    char padding2[32];
    long once_token;
} singleton_object;

