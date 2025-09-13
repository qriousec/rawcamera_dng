
void nikon_compressed_unpacker(long signpost_id) { // 0x1b2811900
    void* stack_guard;
    char large_buffer[135504];
    char huffman_tables[8448];
    void* block_callback;
    nikon_camera_context* context;
    huffman_decoder_context* decoder_ctx;
    int width, height;
    int camera_params[5];
    void* semaphore1, *semaphore2, *thread_semaphore, *completion_semaphore;
    void* dispatch_group, *dispatch_queue;
    void* memory_buffer;
    int memory_size;
    long current_position, buffer_size;
    
    // Stack canary check setup
    stack_guard = *((void**)0x1E7FDC060);
    
    // Get OS signpost logger
    void* logger = sub_1B27E53F4();
    logger = _objc_retainAutoreleasedReturnValue(logger);
    
    context = (nikon_camera_context*)signpost_id;
    
    if (signpost_id + 1 >= 2) {
        // Emit signpost for profiling
        bool signpost_enabled = _os_signpost_enabled(logger);
        if (signpost_enabled) {
            large_buffer[0] = 0;
            __os_signpost_emit_with_name_impl(
                (void*)0x1B2773000,  // dso
                logger,              // log
                1,                   // type
                signpost_id,         // spid
                "CNikonCompressedUnpacker",  // name
                (void*)0x1B29AD08E,  // format
                large_buffer,        // buf
                2                    // size
            );
        }
    }
    
    _objc_release(logger);
    
    // Create processing block
    image_processing_block* proc_block = (image_processing_block*)(&large_buffer[200]);
    proc_block->callback_ptr = (void*)0x1B2812B94;
    proc_block->callback_data = (void*)0x1E90B62D0;
    proc_block->context = context;
    
    block_callback = _objc_retainBlock(proc_block);
    
    // Get camera parameters
    void* camera_data = get_field_at_offset_72(context);
    
    // Get shared pointer to camera
    void* camera_ptr = context->camera_ptr;
    void* control_block = *((void**)((char*)context + 32));
    if (control_block) {
        // Increment reference count
        long* ref_count = (long*)((char*)control_block + 8);
        __sync_fetch_and_add(ref_count, 1);
    }
    
    void* raw_camera = NULL;
    if (camera_ptr) {
        raw_camera = get_field_at_offset_24(camera_ptr);
        set_pointer_at_offset_24(camera_ptr, camera_data);
    }
    
    // Get image dimensions
    width = context->width;
    height = context->height;
    
    // Calculate offset difference
    long offset_diff = calculate_offset_difference(context);
    int original_height = context->camera_parameter_1;
    
    // Get camera parameter through vtable
    raw_camera_vtable* vtable = ((raw_camera*)context)->vtable;
    int param = vtable->get_parameter(context);
    
    long total_pixels = (long)context->camera_parameter_1 + param;
    
    // Validate pixel count doesn't overflow
    if ((total_pixels << 31) >> 31 != total_pixels) {
        throw std::runtime_error("RawCameraException");
    }
    
    int pixels_to_process = total_pixels - original_height;
    if (pixels_to_process <= 0) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Get camera type parameters
    int camera_param = context->camera_parameter_3;
    if (camera_param != (int)camera_param) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Set camera parameter
    raw_camera_vtable* cam_vtable = ((raw_camera*)camera_ptr)->vtable;
    cam_vtable->get_parameter(camera_ptr); // Actually a setter based on context
    
    // Get color filter information
    int color_filter_1 = get_camera_parameter(camera_ptr);
    int color_filter_2 = get_camera_parameter(camera_ptr);
    
    // Handle special color filter cases
    if (color_filter_1 == 0x49 || color_filter_2 == 0x58) {
        // Set special parameter for these filters
        int special_param = 0x83E;
        cam_vtable->get_parameter(camera_ptr); // Set parameter
    }
    
    // Determine decoder type based on color filters
    int decoder_type = 0;
    if (color_filter_1 == 0x46) {
        decoder_type = 2;
    }
    int camera_setting = context->camera_parameter_4;
    if (camera_setting == 0xE) {
        decoder_type += 3;
    } else {
        decoder_type += 0;
    }
    
    // Get bit depth and other parameters
    int bit_depth_1 = get_camera_parameter(camera_ptr);
    int bit_depth_2 = get_camera_parameter(camera_ptr);
    int bit_depth_3 = get_camera_parameter(camera_ptr);
    int bit_depth_4 = get_camera_parameter(camera_ptr);
    int samples_per_pixel = get_camera_parameter(camera_ptr);
    
    int bits_per_sample = context->camera_parameter_4;
    int max_value = 1 << bits_per_sample;
    max_value &= 0x7FFF;
    
    int step_size;
    if (samples_per_pixel >= 2) {
        step_size = (max_value - 1) / (samples_per_pixel - 1);
        if (color_filter_2 == 0x40) {
            step_size >>= 2;
        }
    } else {
        step_size = 0;
        if (color_filter_2 == 0x40) {
            step_size >>= 2;
        }
    }
    
    // Handle special Bayer pattern processing
    if (color_filter_1 == 0x44 && color_filter_2 == 0x40 && step_size > 0 && samples_per_pixel > 0) {
        // Process Bayer pattern data
        int expanded_step = step_size * 2;
        for (int i = 0; i < samples_per_pixel; i++) {
            int sample = get_camera_parameter(camera_ptr);
            ((short*)(&large_buffer[0x88]))[i] = sample;
        }
        
        // Interpolate values for missing samples
        if (bits_per_sample <= 0xE && max_value > 1) {
            int sample_count = (max_value > 1) ? max_value : 1;
            for (int i = 0; i < sample_count; i++) {
                int base_idx = i / step_size;
                int offset = i - (base_idx * step_size);
                int remaining = step_size - offset;
                
                short val1 = ((short*)(&large_buffer[0x88]))[base_idx * step_size];
                short val2 = ((short*)(&large_buffer[0x88]))[base_idx * step_size + step_size];
                
                int interpolated = (remaining * val1 + offset * val2) / step_size;
                ((short*)(&large_buffer[0x88]))[i] = interpolated;
            }
        }
        
        // Set camera parameter for interpolated data
        int param_val = camera_param + 0x232;
        cam_vtable->get_parameter(camera_ptr); // Set parameter
        
        // Get final sample count
        samples_per_pixel = get_camera_parameter(camera_ptr);
        
        if (samples_per_pixel == 0 || height <= samples_per_pixel) {
            throw std::runtime_error("RawCameraException");
        }
        
        // Create Huffman decoder tables
        bool success = create_huffman_decoder_tables(
            (void*)((long)0x1B296C960 + decoder_type * 32),
            &huffman_tables[0],
            4
        );
        if (!success) {
            throw std::runtime_error("RawCameraException");
        }
        
        success = create_huffman_decoder_tables(
            (void*)((long)0x1B296C960 + (decoder_type + 1) * 32),
            &huffman_tables[2048],
            4
        );
        if (!success) {
            throw std::runtime_error("RawCameraException");
        }
    } else if (color_filter_1 == 0x46 || samples_per_pixel >= 0x4002) {
        // Handle other color filter patterns
        if (samples_per_pixel > 0) {
            for (int i = 0; i < samples_per_pixel; i++) {
                int sample = get_camera_parameter(camera_ptr);
                ((short*)(&large_buffer[0x88]))[i] = sample;
            }
        }
        
        // Fill remaining buffer with interpolated values
        if (samples_per_pixel > 1) {
            short last_val = ((short*)(&large_buffer[0x88]))[samples_per_pixel - 1];
            int remaining = 0x10000 - samples_per_pixel;
            
            // Vectorized fill for efficiency
            for (int i = samples_per_pixel; i < 0x10000; i += 32) {
                // Use SIMD instructions to fill 32 values at once
                for (int j = 0; j < 32 && (i + j) < 0x10000; j++) {
                    ((short*)(&large_buffer[0x88]))[i + j] = last_val;
                }
            }
        }
    } else {
        // Generate linear ramp for default case
        for (int i = 0; i < 0x4000; i++) {
            ((short*)(&large_buffer[0x88 + 32]))[i * 4] = i * 8;
            ((short*)(&large_buffer[0x88 + 32]))[i * 4 + 1] = i * 8 + 8;
            ((short*)(&large_buffer[0x88 + 32]))[i * 4 + 2] = i * 8 + 16;
            ((short*)(&large_buffer[0x88 + 32]))[i * 4 + 3] = i * 8 + 24;
        }
    }
    
    // Create main Huffman decoder table
    bool success = create_huffman_decoder_tables(
        (void*)((long)0x1B296C960 + decoder_type * 32),
        &huffman_tables[0],
        2
    );
    if (!success) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Get system page size for memory alignment
    long page_size = _NSPageSize();
    
    // Get extraction value
    long extraction_value = get_value_with_extraction(context);
    
    if (pixels_to_process <= 0) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Get number of CPU cores for threading
    void* process_info = *(void**)0x1E7F720A0;
    void* cpu_info = sub_1B29CF7C0(process_info);
    cpu_info = _objc_retainAutoreleasedReturnValue(cpu_info);
    long core_count = sub_1B29CD800(cpu_info);
    _objc_release(cpu_info);
    
    // Determine thread configuration
    int max_threads = (core_count < 31) ? core_count : 31;
    int decode_threads = (core_count < 29) ? core_count : 29;
    if (core_count == 1) {
        decode_threads = 2;
    } else {
        decode_threads += 2;
    }
    
    // Calculate memory requirements
    page_size = _NSPageSize();
    int doubled_page_size = page_size * 2;
    int block_size = doubled_page_size + width * 2;
    
    int thread_buffer_size = max_threads * 64;
    int decode_buffer_size = decode_threads * (decode_threads + decode_threads * 2) * 64;
    int total_memory_size = thread_buffer_size + decode_buffer_size + 0x500;
    
    // Allocate aligned memory
    memory_size = total_memory_size + page_size + block_size * decode_threads;
    memory_buffer = allocate_memory_with_type(memory_size);
    if (!memory_buffer) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Set up decoder context
    decoder_ctx = (huffman_decoder_context*)((char*)memory_buffer + page_size);
    decoder_ctx->memory_ptr = memory_buffer;
    decoder_ctx->memory_size = memory_size;
    
    // Initialize thread management
    if (core_count >= 0) {
        for (int i = 0; i < max_threads + 1; i++) {
            void** thread_ptr = (void**)((char*)decoder_ctx + 0x20 + i * 8);
            *thread_ptr = (char*)decoder_ctx + 0x4C0 + i * 0x40;
            *((int*)(*thread_ptr)) = i;
            *((void**)(*thread_ptr + 8)) = decoder_ctx;
        }
    }
    
    // Set up decode threads
    if (decode_threads >= 1) {
        for (int i = 0; i < decode_threads; i++) {
            void** decode_ptr = (void**)((char*)decoder_ctx + 0x120 + i * 8);
            // Initialize decode thread context
        }
    }
    
    // Initialize decoder parameters
    decoder_ctx->decoder_param_1 = thread_buffer_size;
    decoder_ctx->decoder_param_2 = decode_buffer_size;
    decoder_ctx->decoder_param_3 = total_memory_size;
    decoder_ctx->width = width;
    decoder_ctx->thread_count = decode_threads;
    decoder_ctx->block_width = width;
    decoder_ctx->block_height = height;
    decoder_ctx->current_line = 0;
    decoder_ctx->atomic_counter = 0;
    decoder_ctx->stop_flag = 0;
    decoder_ctx->error_flag = 0;
    decoder_ctx->single_threaded = (huffman_tables[0] == &huffman_tables[2048]);
    decoder_ctx->huffman_table_1 = &huffman_tables[0];
    decoder_ctx->huffman_table_2 = &huffman_tables[2048];
    decoder_ctx->line_buffer = &large_buffer[0x88];
    decoder_ctx->stride = width;
    decoder_ctx->total_pixels = width * height;
    
    // Set up function pointers based on single/multi-threaded mode
    if (decoder_ctx->single_threaded) {
        decoder_ctx->decode_line_func = (void*)0x1B2812C24;
        decoder_ctx->process_block_func = (void*)0x1B28130AC;
        decoder_ctx->max_blocks_per_line = 0x7FFFFFFF;
    } else {
        decoder_ctx->decode_line_func = (void*)0x1B2812E64;
        decoder_ctx->process_block_func = (void*)0x1B2813598;
        // Calculate blocks per line for multi-threading
        double blocks_per_line = (double)height * width / pixels_to_process;
        int calculated_blocks = (int)(blocks_per_line + 0.5);
        int adjusted_blocks = (calculated_blocks + 1) / 2;
        decoder_ctx->max_blocks_per_line = width / adjusted_blocks + 1;
    }
    
    // Initialize threading primitives
    semaphore1 = _dispatch_semaphore_create(0);
    decoder_ctx->semaphore_1 = semaphore1;
    decoder_ctx->semaphore_counter_1 = 0;
    
    semaphore2 = _dispatch_semaphore_create(0);
    decoder_ctx->semaphore_2 = semaphore2;
    
    thread_semaphore = _dispatch_semaphore_create(0);
    decoder_ctx->thread_semaphore = thread_semaphore;
    
    completion_semaphore = _dispatch_semaphore_create(0);
    decoder_ctx->completion_semaphore = completion_semaphore;
    
    // Set up bit buffer and data processing
    long bit_buffer_size = extraction_value + extraction_value;
    decoder_ctx->bit_buffer = bit_buffer_size;
    decoder_ctx->bits_available = 0x200001;
    
    // Initialize color filter parameters
    decoder_ctx->color_filter_1 = bit_depth_1;
    decoder_ctx->color_filter_2 = bit_depth_2;
    decoder_ctx->color_filter_3 = bit_depth_3;
    decoder_ctx->color_filter_4 = bit_depth_4;
    
    // Create dispatch group and queue for parallel processing
    dispatch_group = _dispatch_group_create();
    dispatch_queue = _dispatch_queue_create("Huffman Decode Block", (void*)0x1E7FDBEC0);
    
    // Launch worker threads
    if (core_count >= 1) {
        void* worker_func = (core_count == 1) ? (void*)0x1B28143C8 : (void*)0x1B281393C;
        
        for (int i = 0; i < max_threads; i++) {
            void* thread_context = (void**)((char*)decoder_ctx + 0x28 + i * 8);
            _dispatch_group_async_f(dispatch_group, dispatch_queue, thread_context, worker_func);
        }
    }
    
    // Main decoding loop
    long current_pos = decoder_ctx->current_row_index;
    long buffer_mask = decoder_ctx->buffer_mask;
    void* data_buffer = decoder_ctx->decoder_buffer_1;
    
    while (current_pos < decoder_ctx->buffer_size) {
        // Wait for available threads and process data chunks
        long chunk_size = (decoder_ctx->buffer_size - current_pos < buffer_mask) ? 
                         decoder_ctx->buffer_size - current_pos : buffer_mask;
        
        // Read compressed data
        raw_camera_vtable* vtable = ((raw_camera*)camera_ptr)->vtable;
        vtable->get_parameter(camera_ptr); // Actually reads data
        
        current_pos += chunk_size;
    }
    
    // Wait for all threads to complete
    _dispatch_group_wait(dispatch_group, DISPATCH_TIME_FOREVER);
    
    // Clean up
    _objc_release(dispatch_queue);
    _objc_release(dispatch_group);
    
    if (decoder_ctx->memory_ptr) {
        _objc_release(decoder_ctx->semaphore_1);
        _objc_release(decoder_ctx->semaphore_2);
        _objc_release(decoder_ctx->thread_semaphore);
        _objc_release(decoder_ctx->completion_semaphore);
        _free(decoder_ctx->memory_ptr);
    }
    
    // Clean up shared pointer
    if (camera_ptr) {
        set_pointer_at_offset_24(camera_ptr, raw_camera);
    }
    shared_ptr_destructor((shared_ptr*)(&camera_ptr));
    
    // Release block callback
    void* callback_destructor = *((void**)((char*)block_callback + 16));
    callback_destructor(block_callback);
    _objc_release(block_callback);
    
    // Stack canary check
    void* current_guard = *((void**)0x1E7FDC060);
    if (current_guard != stack_guard) {
        __stack_chk_fail();
    }
}
