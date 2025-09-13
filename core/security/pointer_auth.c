
void* get_function_pointer() { // 0x1b2777384
    void* function_ptr = global_function_table.function_ptr;
    // Apply pointer authentication
    return function_ptr;
}
