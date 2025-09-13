typedef struct shared_weak_count {
    void *vtable_ptr;
    char padding[8];
    void (*release_weak_func)(void);
} shared_weak_count;

typedef struct shared_ptr {
    void* ptr;
    void* control_block;
} shared_ptr;

typedef struct shared_weak_count {
    void* vtable;
    long ref_count;
    long weak_count;
} shared_weak_count;

