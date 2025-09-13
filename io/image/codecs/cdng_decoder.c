
// cdng_lossless_jpeg_unpack (0x1b2866bc4)
void cdng_lossless_jpeg_unpack(cdng_unpacker_t *unpacker) {
    void *signpost_log = sub_1B27E53F4();
    signpost_log = _objc_retainAutoreleasedReturnValue(signpost_log);
    
    if (unpacker + 1 < (cdng_unpacker_t*)2) {
        _objc_release(signpost_log);
        
        block_params_t completion_block;
        completion_block.function_ptr = (void*)0x1E7FDC048;  // function pointer with PAC
        completion_block.descriptor = (void*)0x1B296A770;    // descriptor data
        completion_block.invoke_ptr = (void*)sub_1B2867090;  // invoke function with PAC
        completion_block.descriptor_ptr = (void*)0x1E90B62D0; // descriptor pointer
        completion_block.unpacker = unpacker;
        
        void *retained_block = _objc_retainBlock(&completion_block);
        
        if (unpacker->is_compressed == 0) {
            // Call virtual function at offset 224 (0xE0)
            void *vtable = unpacker->vtable;
            int (*vfunc)(cdng_unpacker_t*) = (int(*)(cdng_unpacker_t*))((char*)vtable + 224);
            if (vfunc(unpacker) != 2) {
                goto process_tiles;
            }
        }
        
        jpeg_decoder_process(unpacker);
        
        // Execute completion block
        void (*invoke_func)(void*) = (void(*)(void*))((char*)retained_block + 16);
        invoke_func(retained_block);
        _objc_release(retained_block);
        return;
    }
    
    // Signpost logging enabled
    if (_os_signpost_enabled(signpost_log)) {
        uint16_t buffer = 0;
        __os_signpost_emit_with_name_impl((void*)0x1B2773000, signpost_log, 1, unpacker, 
                                          "CDNGLosslessJpegUnpacker", (void*)0x1B29AD08E, 
                                          &buffer, 2);
    }
    
    _objc_release(signpost_log);
    
    block_params_t completion_block;
    completion_block.function_ptr = (void*)0x1E7FDC048;
    completion_block.descriptor = (void*)0x1B296A770;
    completion_block.invoke_ptr = (void*)sub_1B2867090;
    completion_block.descriptor_ptr = (void*)0x1E90B62D0;
    completion_block.unpacker = unpacker;
    
    void *retained_block = _objc_retainBlock(&completion_block);
    
    if (unpacker->is_compressed != 0) {
        jpeg_decoder_process(unpacker);
        void (*invoke_func)(void*) = (void(*)(void*))((char*)retained_block + 16);
        invoke_func(retained_block);
        _objc_release(retained_block);
        return;
    }
    
process_tiles:
    uint32_t width = unpacker->width;
    uint32_t height = unpacker->height;
    uint32_t tile_width = unpacker->tile_width;
    uint32_t tile_height = unpacker->tile_height;
    
    uint64_t tile_offsets_count = (unpacker->tile_offsets_end - unpacker->tile_offsets_start);
    uint64_t tile_byte_counts_count = (unpacker->tile_byte_counts_end - unpacker->tile_byte_counts_start);
    
    if (tile_offsets_count != tile_byte_counts_count) {
        void *exception = __cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error((std::runtime_error*)exception, "RawCameraException");
        __cxa_throw(exception, (void*)0x1E7FD8A98, (void*)0x1E7FD8920);
    }
    
    // Validation checks
    uint32_t limit = 0xFFFE7960;
    uint32_t limit_plus_one = limit + 1;
    
    if ((uint32_t)(width + limit) < limit_plus_one) {
        goto throw_exception;
    }
    if ((uint32_t)(height + limit) < limit_plus_one) {
        goto throw_exception;
    }
    if ((uint32_t)(tile_width + limit) < limit_plus_one) {
        goto throw_exception;
    }
    if ((uint32_t)(tile_height + limit) < limit_plus_one) {
        goto throw_exception;
    }
    
    uint32_t tile_count;
    check_and_extract_lower_32bits(&tile_count, &tile_offsets_count);
    
    if (((tile_count >> 5) & 0x1FFFFFF) >= 0x271) {
        goto throw_exception;
    }
    
    // Calculate expected tile dimensions
    uint32_t tiles_horizontal = (width + tile_width - 1) / tile_width;
    uint32_t tiles_vertical = (height + tile_height - 1) / tile_height;
    if (unpacker->is_compressed != 0) {
        tiles_vertical >>= 1;
    }
    uint32_t expected_tiles = tiles_horizontal * tiles_vertical;
    
    if (expected_tiles != tile_count) {
        void *exception = __cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error((std::runtime_error*)exception, "RawCameraException");
        __cxa_throw(exception, (void*)0x1E7FD8A98, (void*)0x1E7FD8920);
    }
    
    // Call virtual function at offset 40 (0x28)
    void *vtable = unpacker->vtable;
    void* (*vfunc)(cdng_unpacker_t*) = (void*(*)(cdng_unpacker_t*))((char*)vtable + 40);
    void *decoder = vfunc(unpacker);
    
    void *output_buffer = calculate_offset_difference(unpacker);
    
    if (tile_count == 0) {
        void (*invoke_func)(void*) = (void(*)(void*))((char*)retained_block + 16);
        invoke_func(retained_block);
        _objc_release(retained_block);
        return;
    }
    
    // Process tiles
    uint64_t tile_index = 0;
    uint32_t current_row = 0;
    uint32_t current_col = 0;
    uint64_t row_stride = (uint64_t)width * decoder * 2;
    uint64_t tile_data_size = (uint64_t)tile_count * 4;
    
    while (tile_index < tile_data_size) {
        uint32_t tile_offset = unpacker->tile_offsets_start[tile_index / 4];
        uint32_t tile_byte_count = unpacker->tile_byte_counts_start[tile_index / 4];
        
        void *tile_data = allocate_and_process_data(unpacker, tile_offset, tile_byte_count);
        
        if (tile_data != NULL) {
            uint32_t actual_byte_count = unpacker->tile_byte_counts_start[tile_index / 4];
            
            uint64_t src_params[2] = {0, 0};
            initialize_coordinates((char*)src_params + 8, width, height);
            
            uint64_t dst_params[2];
            initialize_coordinates(dst_params, current_row, current_col);
            initialize_coordinates((char*)dst_params + 8, tile_width, tile_height);
            
            void *color_space = initialize_singleton(unpacker);
            
            char is_compressed = 1;
            
            unpack_jpeg_tile_with_context_check(unpacker, row_stride, src_params, decoder, dst_params, tile_data, 
                         (char*)tile_data + actual_byte_count, color_space);
            
            free(tile_data);
        }
        
        tile_index += 4;
        uint32_t next_row = current_row + tile_width;
        if (next_row < width) {
            current_col += tile_height;
            current_row = 0;
        } else {
            current_row = next_row;
        }
    }
    
    void (*invoke_func)(void*) = (void(*)(void*))((char*)retained_block + 16);
    invoke_func(retained_block);
    _objc_release(retained_block);
    return;
    
throw_exception:
    void *exception = __cxa_allocate_exception(0x10);
    std::runtime_error::runtime_error((std::runtime_error*)exception, "RawCameraException");
    __cxa_throw(exception, (void*)0x1E7FD8A98, (void*)0x1E7FD8920);
}

void cdng_lossless_jpeg_unpacker_unpack_tile(uint32_t tile_id, camera_params* cam_params, image_format* format, uint32_t param3, void* jpeg_data, uint32_t jpeg_size, source_region* src_region, dest_region* dst_region, int flip_flag) // 0x1b2868cd8
{
    void* signpost_log;
    void* autorelease_context;
    jpeg_decode_context* decoder;
    jpeg_image_info image_info;
    buffer_info output_buffer_info;
    decode_buffer decode_buf;
    int precision;
    int format_option;
    void* output_buffer;
    uint16_t signpost_buf;
    
    signpost_log = sub_1B27E53F4();
    signpost_log = _objc_retainAutoreleasedReturnValue(signpost_log);
    
    if (tile_id + 1 >= 2) {
        if (_os_signpost_enabled(signpost_log)) {
            signpost_buf = 0;
            __os_signpost_emit_with_name_impl((void*)0x1B2773000, signpost_log, 1, tile_id, "CDNGLosslessJpegUnpacker::unpackTile", (void*)0x1B29AD08E, &signpost_buf, 2);
        }
    }
    
    _objc_release(signpost_log);
    
    // Create and retain block for autorelease pool
    void* block_data[6];
    block_data[0] = (void*)0x1E7FDC048; // PAC'd function pointer
    block_data[1] = (void*)0x6AE1; // Block descriptor
    block_data[2] = sub_1B286921C;
    block_data[3] = (void*)0x1E90B62D0;
    block_data[4] = (void*)tile_id;
    
    void* block = _objc_retainBlock(block_data);
    autorelease_context = _objc_autoreleasePoolPush();
    
    // Initialize decoder configuration
    char decoder_config[24];
    *(uint64_t*)&decoder_config[0] = 0x1F3823280;
    *(uint64_t*)&decoder_config[8] = 0x1F3823288;
    *(uint64_t*)&decoder_config[16] = 0x1F3823290;
    
    decoder = _applejpeg_decode_create(decoder_config);
    if (!decoder) {
        ___cxa_allocate_exception(0x10);
        __ZNSt13runtime_errorC1EPKc("RawCameraException");
        ___cxa_throw();
    }
    
    // Setup memory buffer for JPEG data
    buffer_info mem_buffer;
    mem_buffer.buffer_ptr = jpeg_data;
    mem_buffer.buffer_size = (char*)src_region - (char*)jpeg_data;
    
    if (_applejpeg_decode_open_mem(decoder, &mem_buffer) != 0) {
        ___cxa_allocate_exception(0x10);
        __ZNSt13runtime_errorC1EPKc("RawCameraException");
        ___cxa_throw();
    }
    
    if (_applejpeg_decode_get_image_info(decoder, &image_info) != 0) {
        ___cxa_allocate_exception(0x10);
        __ZNSt13runtime_errorC1EPKc("RawCameraException");
        ___cxa_throw();
    }
    
    precision = _applejpeg_decode_get_precision(decoder);
    if (precision - 17 <= -10) {
        ___cxa_allocate_exception(0x10);
        __ZNSt13runtime_errorC1EPKc("RawCameraException");
        ___cxa_throw();
    }
    
    // Determine output format based on components and precision
    if (image_info.components <= 2) {
        if (image_info.components == 1) {
            format_option = 0x12;
        } else if (image_info.components == 2) {
            format_option = (precision > 8) ? 0x17 : 0x16;
        } else {
            ___cxa_allocate_exception(0x10);
            __ZNSt13runtime_errorC1EPKc("RawCameraException");
            ___cxa_throw();
        }
    } else {
        if (image_info.components == 3) {
            format_option = 0x14;
        } else if (image_info.components == 4) {
            format_option = 0x15;
        } else {
            ___cxa_allocate_exception(0x10);
            __ZNSt13runtime_errorC1EPKc("RawCameraException");
            ___cxa_throw();
        }
    }
    
    if (_applejpeg_decode_set_option_outformat(decoder, format_option) != 0) {
        ___cxa_allocate_exception(0x10);
        __ZNSt13runtime_errorC1EPKc("RawCameraException");
        ___cxa_throw();
    }
    
    // Get output buffer size
    output_buffer_info.buffer_ptr = NULL;
    output_buffer_info.buffer_size = 0;
    
    if (_applejpeg_decode_get_output_buffer_size(decoder, &output_buffer_info.buffer_size, &output_buffer_info.buffer_ptr, (void*)0x110) != 0) {
        ___cxa_allocate_exception(0x10);
        __ZNSt13runtime_errorC1EPKc("RawCameraException");
        ___cxa_throw();
    }
    
    // Allocate output buffer
    output_buffer = sub_1B29CE080((void*)0x1E7F64438, output_buffer_info.buffer_size);
    output_buffer = _objc_retainAutoreleasedReturnValue(output_buffer);
    output_buffer = _objc_retainAutorelease(output_buffer);
    
    decode_buf.data_ptr = sub_1B29CF120(output_buffer);
    decode_buf.size = output_buffer_info.buffer_size;
    
    // Decode the image
    if (_applejpeg_decode_image_all(decoder, &decode_buf, 1, &signpost_buf) != 0) {
        ___cxa_allocate_exception(0x10);
        __ZNSt13runtime_errorC1EPKc("RawCameraException");
        ___cxa_throw();
    }
    
    // Get flip flag and process decoded data
    int should_flip = (*(int(*)(uint32_t))(*(uint64_t*)(tile_id + 0x58)))(tile_id);
    should_flip = should_flip & (flip_flag ^ 1);
    
    if (precision >= 9) {
        process_raw_camera_data(tile_id, cam_params, format, param3, &signpost_buf, &image_info, src_region, dst_region, should_flip);
    } else {
        process_raw_camera_image(tile_id, cam_params, format, param3, &signpost_buf, &image_info, src_region, dst_region, should_flip);
    }
    
    _objc_release(output_buffer);
    _applejpeg_decode_destroy(decoder);
    _objc_autoreleasePoolPop(autorelease_context);
    
    // Execute block cleanup
    (*(void(*)(void*))(*(uint64_t*)(block + 0x10)))(block);
    _objc_release(block);
}
