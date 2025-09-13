
// validate_raw_camera_parameter (0x1b2778dfc)
void validate_raw_camera_parameter(raw_camera *camera, int unused_param, int expected_value) {
    raw_camera_vtable *vtable = camera->vtable;
    
    int actual_value = vtable->get_parameter(camera);
    
    if (expected_value != actual_value) {
        void *exception = __cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error((std::runtime_error*)exception, "RawCameraException");
        throw_exception(exception);
    }
}

int get_camera_parameter(raw_camera *camera) { // 0x1b2778dcc
    char parameter_buffer = 0;
    validate_raw_camera_parameter(camera, &parameter_buffer, 1);
    return (int)parameter_buffer;
}

void validate_and_copy_width(int* dest, int* src) { // 0x1b277a274
    int width_value = *src;
    *dest = width_value;
    
    if ((long long)width_value != (long long)(int)width_value) {
        RawCameraException* exception = (RawCameraException*)__cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error((std::runtime_error*)exception, "RawCameraException");
        throw_exception(exception);
    }
}

// 0x1b27a4e7c
void validate_camera_parameter(camera_parameter* param) {
    int32_t value = param->value;
    if ((int64_t)param != (int64_t)value) {
        void* exception = ___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error((std::runtime_error*)exception, "RawCameraException");
        throw_exception(exception);
    }
}
