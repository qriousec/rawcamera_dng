
void validate_raw_camera_pointer(void* camera_object) { // 0x1b27e0070
    void* raw_camera_ptr = *((void**)((char*)camera_object + 16));
    
    if (raw_camera_ptr != NULL) {
        return;
    }
    
    // Throw RawCameraException
    void* exception = __cxa_allocate_exception(0x10);
    std::runtime_error::runtime_error((std::runtime_error*)exception, "RawCameraException");
    throw_exception(exception);
}
