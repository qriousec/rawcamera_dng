
// Function at address 0x1b28692bc
void process_raw_camera_data(void* arg0, camera_params* camera_params_ptr, image_format* format_ptr, uint32_t arg3, void* source_data, uint32_t arg5, source_region* src_rect, dest_region* dst_rect, int use_rgb_mode) {
    uint32_t stack_guard;
    int temp_var1, temp_var2, temp_var3, temp_var4;
    int format_type, adjusted_format;
    int src_x, src_y, src_width, src_height;
    int dst_x, dst_y, dst_width, dst_height;
    rectangle src_bounds, dst_bounds;
    uint32_t row_stride, pixel_stride;
    uint16_t* src_ptr, *dst_ptr;
    void* lookup_table_ptr;
    void* color_lookup_ptr;
    int row_counter, col_counter, channel_counter;
    uint16_t pixel_value;
    exception_info exc;
    
    // Stack guard setup
    stack_guard = *((uint32_t*)0x1E7FDC060);
    
    if (camera_params_ptr->field_20 != 1) {
        ___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error("RawCameraException");
        throw_exception(&exc);
    }
    
    format_type = format_ptr->format_type;
    adjusted_format = (format_type == 4 || format_type == 2) ? format_type : format_type + 1;
    
    // Calculate rectangle bounds
    src_bounds.x = max(src_rect->x, dst_rect->x);
    src_bounds.y = max(src_rect->y, dst_rect->y);
    src_bounds.width = min(src_rect->x + src_rect->width, dst_rect->x + dst_rect->width) - src_bounds.x;
    src_bounds.height = min(src_rect->y + src_rect->height, dst_rect->y + dst_rect->height) - src_bounds.y;
    
    // Validate camera IDs
    validate_camera_id(&temp_var1, &camera_params_ptr->field_28);
    validate_camera_id(&temp_var2, &camera_params_ptr->field_2C);
    
    multiply_with_overflow_check(&temp_var1, &temp_var2);
    temp_var3 = temp_var1;
    
    multiply_with_overflow_check(&temp_var3, &format_type);
    multiply_with_overflow_check(&temp_var3, &adjusted_format);
    
    if (temp_var1 != temp_var3) {
        ___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error("RawCameraException");
        throw_exception(&exc);
    }
    
    // Check bounds validity
    if (src_bounds.x < 0 || src_bounds.y < 0 || src_bounds.width < 0 || src_bounds.height < 0) {
        ___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error("RawCameraException");
        throw_exception(&exc);
    }
    
    if (!rectangles_overlap(&src_bounds, dst_rect)) {
        ___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error("RawCameraException");
        throw_exception(&exc);
    }
    
    // Calculate pixel processing parameters
    if (adjusted_format < 3) {
        int divisor = camera_params_ptr->field_28 * adjusted_format;
        int quotient = divisor / dst_rect->x;
        
        if (quotient - 3 > 3) {
            ___cxa_allocate_exception(0x10);
            std::runtime_error::runtime_error("RawCameraException");
            throw_exception(&exc);
        }
        
        if (quotient >= 2) {
            pixel_stride = (divisor * 2) / quotient;
            row_stride = arg3 - pixel_stride;
        } else {
            pixel_stride = arg3;
            row_stride = arg3;
        }
    }
    
    // Main pixel processing loop
    if (src_bounds.height >= 1) {
        src_x = src_bounds.x - dst_rect->x;
        src_y = src_bounds.y - dst_rect->y;
        dst_x = src_bounds.x - src_rect->x;
        dst_y = src_bounds.y - src_rect->y;
        
        src_ptr = (uint16_t*)((char*)camera_params_ptr + dst_y * arg3 + src_x * temp_var3 * 2);
        dst_ptr = (uint16_t*)((char*)source_data + dst_x * arg5 + dst_y * (use_rgb_mode ? 3 : 1) * 2);
        
        lookup_table_ptr = (void*)0x1ECB7F630;
        color_lookup_ptr = (void*)0x1ECB7F638;
        
        for (row_counter = 0; row_counter < src_bounds.height; row_counter++) {
            if (src_bounds.width >= 1) {
                for (col_counter = 0; col_counter < src_bounds.width; col_counter++) {
                    if (use_rgb_mode) {
                        if (format_type >= 3) {
                            for (channel_counter = 0; channel_counter < format_type; channel_counter++) {
                                pixel_value = src_ptr[0];
                                src_ptr++;
                                
                                if (*((uint64_t*)lookup_table_ptr) != -1) {
                                    // Call some function for lookup
                                }
                                
                                pixel_value = *((uint16_t*)arg0 + pixel_value);
                                pixel_value = *((uint16_t*)color_lookup_ptr + pixel_value);
                                
                                *dst_ptr = pixel_value;
                                *(dst_ptr + 1) = pixel_value;
                                *(dst_ptr + 2) = pixel_value;
                                dst_ptr += 3;
                            }
                        } else if (format_type == 1) {
                            // Single channel processing
                        }
                        
                        // Fill remaining channels with default color
                        if (*((uint64_t*)lookup_table_ptr) != -1) {
                            // Call some function
                        }
                        *dst_ptr = *((uint16_t*)color_lookup_ptr + 0x20000);
                        dst_ptr++;
                    } else {
                        if (format_type == 2 || format_type == 4) {
                            pixel_value = src_ptr[0];
                            src_ptr++;
                            pixel_value = *((uint16_t*)arg0 + pixel_value);
                            *dst_ptr = pixel_value;
                            dst_ptr++;
                        } else {
                            for (channel_counter = 0; channel_counter < format_type; channel_counter++) {
                                pixel_value = src_ptr[0];
                                src_ptr++;
                                pixel_value = *((uint16_t*)arg0 + pixel_value);
                                *dst_ptr = pixel_value;
                                dst_ptr++;
                            }
                        }
                    }
                }
            }
            
            dst_ptr = (uint16_t*)((char*)dst_ptr + arg5);
            src_ptr = (uint16_t*)((char*)src_ptr + row_stride);
        }
    }
    
    // Stack guard check
    if (*((uint32_t*)0x1E7FDC060) != stack_guard) {
        ___stack_chk_fail();
    }
}
