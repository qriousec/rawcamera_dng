
uint64_t* store_and_advance_pointer(uint64_t value, uint64_t* ptr) { // 0x1b27e5504
    *ptr = value;
    ptr++;
    return ptr;
}

// check_and_extract_lower_32bits (0x1b2777324)
void check_and_extract_lower_32bits(uint32_t* output_ptr, uint64_t* input_ptr) {
    uint64_t input_value = *input_ptr;
    *output_ptr = (uint32_t)input_value;
    
    uint64_t upper_32_bits = input_value >> 32;
    if (upper_32_bits != 0) {
        void* exception_memory = __cxa_allocate_exception(0x10);
        exception_info* exception_obj = (exception_info*)exception_memory;
        std::runtime_error::runtime_error(exception_obj, "RawCameraException");
        throw_exception(exception_obj);
    }
}

void set_pointer_at_offset_24(unknown_struct *obj, void *ptr) { // 0x1b2778cc8
    obj->ptr_field = ptr;
}

void* get_field_at_offset_72(unknown_struct* arg1) { // 0x1b2808c00
    return arg1->field_at_offset_72;
}

void* get_field_at_offset_24(unknown_struct* arg0) { // 0x1b2778cc0
    return arg0->field_at_offset_24;
}
