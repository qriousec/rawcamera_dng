
// Function at address 0x1b286588c
void* allocate_and_process_data(data_processor* processor, int value, int size) {
    int local_value;
    int temp_var;
    void* allocated_memory;
    void* handler;
    
    local_value = value;
    
    // Allocate memory with size + 0x40 bytes
    allocated_memory = _malloc_type_malloc(size + 0x40, 0x100004077774924);
    
    // Get handler from processor structure at offset 24 (0x18)
    handler = processor->handler_ptr;
    
    // Call check_and_copy_value function
    check_and_copy_value(&temp_var, &local_value);
    
    // Get vtable from handler and call function at offset 40 (0x28)
    void* vtable = *(void**)handler;
    // Authenticate pointer with PAC
    void* (*func_ptr)(void*, int*, int) = (void* (*)(void*, int*, int))(*(void**)(vtable + 0x28));
    func_ptr(handler, &temp_var, 0);
    
    // Get handler again and call function at offset 32 (0x20)
    handler = processor->handler_ptr;
    vtable = *(void**)handler;
    void* (*process_func)(void*, void*, int) = (void* (*)(void*, void*, int))(*(void**)(vtable + 0x20));
    process_func(handler, allocated_memory, size);
    
    return allocated_memory;
}

void* allocate_memory_with_type(size_t size) { // 0x1b295caa8
    void* ptr = _malloc_type_malloc(size, 0xB569D1EE);
    if (ptr == NULL) {
        print_number();
    }
    return ptr;
}
