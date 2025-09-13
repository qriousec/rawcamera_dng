
void multiply_with_overflow_check(int* arg1, int* arg2) { // 0x1b27d627c
    int multiplier = *arg2;
    int multiplicand = *arg1;
    
    bool is_negative = multiplier < 0;
    if (multiplier < 0) {
        multiplier = -multiplier;
    }
    
    unsigned long long result = (unsigned long long)multiplicand * (unsigned long long)multiplier;
    
    int final_result = (int)result;
    if (is_negative) {
        final_result = -final_result;
    }
    
    *arg1 = final_result;
    
    bool overflow = (result >> 32) != 0;
    bool should_throw = (multiplier != 0) ? is_negative : false;
    
    if (overflow || should_throw) {
        exception_info* exception = (exception_info*)___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(exception, "RawCameraException");
        throw_exception(exception);
    }
}

uint32_t get_value_with_extraction(object_with_vtable* obj) { // 0x1b2808710
    void* vtable_ptr = obj->vtable_ptr;
    void* authenticated_vtable = vtable_ptr; // Authentication with pointer signing
    
    // Call virtual function at offset 144 (0x90) from vtable
    void* func_ptr = *(void**)(authenticated_vtable + 144);
    void* result = ((void*(*)())(func_ptr))();
    
    uint32_t extracted_value;
    uint64_t full_value = (uint64_t)result;
    
    // Extract lower 32 bits
    check_and_extract_lower_32bits(&extracted_value, &full_value);
    
    // Add with overflow check using global variable
    add_with_overflow_check(&extracted_value, (int*)0x1b296c71c);
    
    return extracted_value;
}

// add_with_overflow_check (0x1b27895c8)
void add_with_overflow_check(int* result_ptr, int* addend_ptr) {
    long long extended_result = (long long)(*result_ptr);
    int addend = *addend_ptr;
    long long sum = extended_result + addend;
    long long overflow_check = sum << 31;
    *result_ptr = (int)sum;
    long long sign_extended = (long long)((int)sum);
    
    if (sum != (overflow_check >> 31)) {
        long long masked_sum = sum & 0x1FFFFFFFF;
        long long masked_check = sum & 0x1FFFFFFFF;
        if (sum != masked_sum || sum != masked_check) {
            exception_info* exception = (exception_info*)___cxa_allocate_exception(0x10);
            std::runtime_error::runtime_error((char*)exception + 8, "RawCameraException");
            throw_exception(exception);
        }
    }
}

void check_bounds_and_subtract(bounds_check_context* ctx, int32_t* subtrahend) { // 0x1b27aac68
    int32_t original_value = ctx->value;
    int64_t subtrahend_value = *subtrahend;
    int64_t result = original_value - subtrahend_value;
    int64_t sign_extended = (result << 31) >> 31;
    
    ctx->value = (int32_t)result;
    
    if (sign_extended != result || (result & 0x8000000000000000ULL) != 0) {
        void* exception = __cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error((std::runtime_error*)exception, "RawCameraException");
        throw_exception(exception);
    }
}

void subtract_with_underflow_check(int *minuend_ptr, int *subtrahend_ptr) { // 0x1b27ab684
    int minuend = *minuend_ptr;
    int subtrahend = *subtrahend_ptr;
    int result = minuend - subtrahend;
    *minuend_ptr = result;
    
    if (result < 0) {
        void *exception_obj = ___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error((std::runtime_error*)exception_obj, "RawCameraException");
        throw_exception(exception_obj);
    }
}

void add_with_overflow_check(int* arg1, int* arg2) { // 0x1b277a424
    int value1 = *arg1;
    long value2 = (long)(*arg2);
    long result = value1 + value2;
    long sign_extended = (result << 31) >> 31;
    
    *arg1 = (int)result;
    
    if (sign_extended != result || (result & 0x8000000000000000ULL) != 0) {
        RawCameraException* exception = (RawCameraException*)___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error((std::runtime_error*)exception, "RawCameraException");
        throw_exception(exception);
    }
}
