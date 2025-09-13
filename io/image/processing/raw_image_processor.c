
// Function at 0x1b28697ac
void process_raw_camera_image(void* arg0, camera_info* camera_info_ptr, image_params* params, int arg3, void* output_buffer, int arg5, crop_rect* crop_area, output_rect* output_area, int use_rgb_flag) {
    int stack_canary;
    int temp_var1, temp_var2, temp_var3, temp_var4;
    int channels, processed_channels;
    int rect_coords[4];
    int calculated_width, calculated_height;
    int loop_counters[2];
    int stride_params[2];
    void* lookup_table_ptr;
    void* color_conversion_table;
    
    // Stack canary setup
    stack_canary = *(int*)0x1E7FDC060;
    
    // Validate camera flag
    if (camera_info_ptr->flag != 1) {
        exception_info* ex = (exception_info*)___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(ex, "RawCameraException");
        throw_exception(ex);
        __builtin_trap();
    }
    
    // Extract channel information
    channels = params->channels;
    processed_channels = (channels == 4 || channels == 2) ? channels : 1;
    
    // Calculate crop rectangle coordinates
    rect_coords[0] = fmax(crop_area->x, output_area->x);
    rect_coords[1] = fmax(crop_area->y, output_area->y);
    rect_coords[2] = fmin(crop_area->x + crop_area->width, output_area->x + output_area->width) - rect_coords[0];
    rect_coords[3] = fmin(crop_area->y + crop_area->height, output_area->y + output_area->height) - rect_coords[1];
    
    // Validate camera offsets and parameters
    validate_camera_id(&camera_info_ptr->offset1, &temp_var1);
    validate_camera_id(&camera_info_ptr->offset2, &temp_var2);
    
    multiply_with_overflow_check(&temp_var1, &temp_var2);
    calculated_width = temp_var1;
    multiply_with_overflow_check(&calculated_width, &channels);
    
    validate_camera_id(&output_area->x + 8, &temp_var3);
    validate_camera_id(&output_area->width, &temp_var4);
    
    multiply_with_overflow_check(&temp_var3, &temp_var4);
    calculated_height = temp_var3;
    multiply_with_overflow_check(&calculated_height, &processed_channels);
    
    if (calculated_width != calculated_height) {
        exception_info* ex = (exception_info*)___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(ex, "RawCameraException");
        throw_exception(ex);
        __builtin_trap();
    }
    
    // Validate rectangle coordinates
    if (rect_coords[0] < 0 || rect_coords[1] < 0 || rect_coords[2] < 0 || rect_coords[3] < 0) {
        exception_info* ex = (exception_info*)___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(ex, "RawCameraException");
        throw_exception(ex);
        __builtin_trap();
    }
    
    // Check rectangle overlap
    if (!rectangles_overlap((rectangle*)rect_coords, (rectangle*)output_area)) {
        exception_info* ex = (exception_info*)___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(ex, "RawCameraException");
        throw_exception(ex);
        __builtin_trap();
    }
    
    // Setup processing parameters
    int offset_x = rect_coords[0] - output_area->x;
    int offset_y = rect_coords[1] - output_area->y;
    int width_diff = rect_coords[2] - crop_area->width;
    int height_diff = rect_coords[3] - crop_area->height;
    
    void* src_ptr = (char*)camera_info_ptr + (height_diff * arg3) + (offset_x * processed_channels);
    void* dst_ptr = (char*)output_buffer + (width_diff * arg5) + (offset_y << (use_rgb_flag ? 1 : 0));
    
    stride_params[0] = arg3;
    stride_params[1] = arg3;
    
    if (processed_channels >= 3) {
        int camera_offset = camera_info_ptr->offset1;
        int scaled_offset = camera_offset * processed_channels;
        int row_stride = scaled_offset / output_area->x;
        
        if (row_stride - 3 <= 3) {  // row_stride in range [0, 6]
            exception_info* ex = (exception_info*)___cxa_allocate_exception(0x10);
            std::runtime_error::runtime_error(ex, "RawCameraException");
            throw_exception(ex);
            __builtin_trap();
        }
        
        if (row_stride >= 2) {
            int split_stride = scaled_offset / row_stride;
            stride_params[0] = split_stride;
            stride_params[1] = arg3 - split_stride;
            loop_counters[0] = 1;
        } else {
            loop_counters[0] = 0;
        }
    } else {
        loop_counters[0] = 0;
    }
    
    // Initialize lookup tables
    lookup_table_ptr = (void*)0x1ECB7F630;
    color_conversion_table = (void*)0x1ECB7F638;
    
    // Main processing loop
    for (int y = 0; y < rect_coords[3]; y++) {
        for (int x = 0; x < rect_coords[2]; x++) {
            if (use_rgb_flag) {
                if (channels == 3) {
                    // RGB processing with lookup table
                    for (int c = 0; c < channels; c++) {
                        uint8_t pixel_val = ((uint8_t*)src_ptr)[c];
                        
                        if (*(int64_t*)lookup_table_ptr == -1) {
                            initialize_once();  // Fallback processing
                        }
                        
                        uint16_t color_val = ((uint16_t*)arg0)[pixel_val];
                        uint16_t converted_val = ((uint16_t*)color_conversion_table)[color_val];
                        
                        ((uint16_t*)dst_ptr)[0] = converted_val;
                        ((uint16_t*)dst_ptr)[1] = converted_val;
                        ((uint16_t*)dst_ptr)[2] = converted_val;
                        dst_ptr = (char*)dst_ptr + 6;
                    }
                } else if (channels >= 3) {
                    // Multi-channel RGB processing
                    for (int c = 0; c < channels; c++) {
                        uint8_t pixel_val = ((uint8_t*)src_ptr)[c];
                        
                        if (*(int64_t*)lookup_table_ptr == -1) {
                            initialize_once();  // Fallback processing
                        }
                        
                        uint16_t color_val = ((uint16_t*)arg0)[pixel_val];
                        uint16_t converted_val = ((uint16_t*)color_conversion_table)[color_val];
                        
                        ((uint16_t*)dst_ptr)[0] = converted_val;
                        dst_ptr = (char*)dst_ptr + 2;
                    }
                    
                    if (*(int64_t*)lookup_table_ptr == -1) {
                        initialize_once();  // Fallback processing
                    }
                    
                    // Add default color value
                    uint16_t default_val = ((uint16_t*)color_conversion_table)[0x19FFE];
                    ((uint16_t*)dst_ptr)[0] = default_val;
                    dst_ptr = (char*)dst_ptr + 2;
                }
            } else {
                if (channels == 2 || channels == 4) {
                    // Direct copy for 2 or 4 channels
                    uint8_t pixel_val = ((uint8_t*)src_ptr)[0];
                    uint16_t color_val = ((uint16_t*)arg0)[pixel_val];
                    ((uint16_t*)dst_ptr)[0] = color_val;
                    dst_ptr = (char*)dst_ptr + 2;
                } else if (channels >= 1) {
                    // Single channel processing
                    for (int c = 0; c < channels; c++) {
                        uint8_t pixel_val = ((uint8_t*)src_ptr)[c];
                        uint16_t color_val = ((uint16_t*)arg0)[pixel_val];
                        ((uint16_t*)dst_ptr)[0] = color_val;
                        dst_ptr = (char*)dst_ptr + 2;
                    }
                } else {
                    // Default case
                    if (*(int64_t*)lookup_table_ptr == -1) {
                        initialize_once();  // Fallback processing
                    }
                    
                    uint16_t default_val = ((uint16_t*)color_conversion_table)[0x19FFE];
                    ((uint16_t*)dst_ptr)[0] = default_val;
                    dst_ptr = (char*)dst_ptr + 2;
                }
            }
            
            src_ptr = (char*)src_ptr + 1;
        }
        
        // Update pointers for next row
        dst_ptr = (char*)dst_ptr + arg5;
        int stride_index = y & loop_counters[0];
        src_ptr = (char*)src_ptr + stride_params[stride_index];
    }
    
    // Stack canary check
    if (*(int*)0x1E7FDC060 != stack_canary) {
        ___stack_chk_fail();
    }
}
