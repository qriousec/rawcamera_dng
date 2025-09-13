
void throw_exception(exception_info* exception_obj) { // 0x1b295f234
    void* exception_type_info;
    void* saved_lr;
    
    exception_type_info = *((void**)0x1E7FD8A98);
    saved_lr = __builtin_return_address(0);
    
    get_function_pointer(exception_obj, exception_type_info, saved_lr);
    
    __cxa_throw();
}

// Function at 0x1b27a7508
void add_with_overflow_check(int* arg1, int* arg2) {
    int value1 = *arg1;
    int value2 = *arg2;
    int result = value1 + value2;
    *arg1 = result;
    
    // Check for overflow using carry flag
    if ((unsigned int)value1 + (unsigned int)value2 < (unsigned int)value1) {
        exception_info* exception = (exception_info*)___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error((char*)exception, "RawCameraException");
        throw_exception(exception);
    }
}

void check_and_copy_value(int* dest, int* src) { // 0x1b2777264
    *dest = 0;
    int value = *src;
    *dest = value;
    
    if (value < 0) {
        exception_info* exception = (exception_info*)___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error((char*)exception, "RawCameraException");
        throw_exception(exception);
    }
}
