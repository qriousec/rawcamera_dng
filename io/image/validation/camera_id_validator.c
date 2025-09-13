
void validate_camera_id(int* camera_id_ptr, int* expected_id_ptr) { // 0x1b277a2d4
    *camera_id_ptr = 0;
    int expected_id = *expected_id_ptr;
    *camera_id_ptr = expected_id;
    
    if ((long)expected_id != (long)(int)expected_id) {
        exception_info* exception = (exception_info*)___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(exception, "RawCameraException");
        throw_exception(exception);
    }
}
