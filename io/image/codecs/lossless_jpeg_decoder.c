
void process_lossless_jpeg_raw(JPEGProcessor* processor) { // 0x1B289AE60
    
    // Stack allocation and setup
    void* log_object = get_signpost_log();
    
    if (processor == nullptr || (processor + 1) > 2) {
        if (is_signpost_enabled(log_object)) {
            char buf[2] = {0};
            emit_signpost("CLosslessJpegSRawUnpacker", buf, 2);
        }
        release_object(log_object);
        return;
    }
    
    release_object(log_object);
    
    // Create completion block
    void* completion_block = create_completion_block();
    
    // Calculate image dimensions and offsets
    int64_t offset_diff = calculate_offset_difference(processor);
    int width = processor->width;
    int height = processor->height;
    
    // Get total image size from virtual method
    int total_pixels = processor->vtable->get_total_pixels(processor);
    if (total_pixels == 0) {
        throw RawCameraException("Invalid pixel count");
    }
    
    // Validate image dimensions
    int reader_height = processor->data_reader->vtable->get_height(processor->data_reader);
    if (processor->height < 0 || processor->height >= reader_height) {
        throw RawCameraException("Invalid height range");
    }
    
    // Allocate buffer for JPEG data
    int buffer_size = total_pixels + 128;
    uint8_t* jpeg_buffer = new uint8_t[buffer_size];
    memset(jpeg_buffer, 0, buffer_size);
    
    // Read JPEG data
    processor->data_reader->vtable->read_data(processor->data_reader, jpeg_buffer, total_pixels);
    
    // Validate JPEG SOI marker
    uint16_t marker = (jpeg_buffer[0] << 8) | jpeg_buffer[1];
    if (marker != 0xFFD8) {
        throw RawCameraException("Invalid JPEG marker");
    }
    
    // Parse JPEG segments
    uint8_t* current_pos = jpeg_buffer + 2;
    uint8_t* end_pos = jpeg_buffer + buffer_size;
    
    int huffman_tables[6] = {0};
    int precision = 0;
    int samples_per_line = 0;
    int num_lines = 0;
    int num_components = 0;
    bool is_lossless = false;
    int component_info = 0;
    
    // Parse JPEG markers
    while (current_pos < end_pos) {
        uint16_t segment_marker = (current_pos[0] << 8) | current_pos[1];
        current_pos += 2;
        
        if (segment_marker == 0xFFFF) {
            continue;
        }
        
        if (segment_marker < 0xFF01) {
            throw RawCameraException("Invalid segment marker");
        }
        
        uint16_t segment_length = (current_pos[0] << 8) | current_pos[1];
        segment_length -= 2;
        
        if (segment_length > 256) {
            throw RawCameraException("Segment too large");
        }
        
        uint8_t* segment_data = current_pos + 2;
        uint8_t* segment_end = segment_data + segment_length;
        
        if (segment_marker == 0xFFC3) { // Start of Frame (Lossless)
            precision = segment_data[0];
            num_lines = (segment_data[1] << 8) | segment_data[2];
            samples_per_line = (segment_data[3] << 8) | segment_data[4];
            num_components = segment_data[7];
            is_lossless = (segment_data[9] != 0x11);
            
            int temp = (segment_data[8] >> 4) * segment_data[8] - 1;
            component_info = temp & 3;
            num_components += component_info + segment_data[6];
        } else if (segment_marker == 0xFFC4) { // Huffman Table
            uint8_t* table_data = segment_data;
            while (table_data < segment_end) {
                uint8_t table_id = table_data[0];
                if (table_id > 5) {
                    throw RawCameraException("Invalid Huffman table ID");
                }
                
                void* table_memory = malloc(2048);
                huffman_tables[table_id] = (int64_t)table_memory;
                table_data++;
                
                if (!create_huffman_decoder_tables(table_data, table_memory, 2)) {
                    throw RawCameraException("Failed to create Huffman tables");
                }
            }
        }
        
        current_pos = segment_end;
        
        if (segment_marker == 0xFFDA) { // Start of Scan
            break;
        }
    }
    
    // Validate lossless JPEG parameters
    if (!is_lossless) {
        throw RawCameraException("Not lossless JPEG");
    }
    
    if (num_components < 7) {
        throw RawCameraException("Insufficient components");
    }
    
    if (samples_per_line * num_lines > width * height) {
        throw RawCameraException("Image size mismatch");
    }
    
    // Process image data
    int row_stride = samples_per_line * 4;
    int16_t* image_data = new int16_t[row_stride];
    memset(image_data, 0, row_stride * sizeof(int16_t));
    
    // Validate camera segments
    validate_camera_id(&processor->width);
    
    void* segments_start = processor->segments_start;
    void* segments_end = processor->segments_end;
    
    if (segments_end != segments_start) {
        for (void* segment = segments_start; segment != segments_end; segment += 4) {
            int segment_value = *(int*)segment;
            if (segment_value < 0) {
                throw RawCameraException("Invalid segment value");
            }
            validate_segment(segment_value);
        }
    }
    
    // Calculate total pixel count
    multiply_with_overflow_check(&processor->width, &width);
    
    int total_image_pixels = samples_per_line * num_lines * (num_lines >> 1);
    if (total_image_pixels < 0 || processor->width != total_image_pixels) {
        throw RawCameraException("Pixel count validation failed");
    }
    
    // Decode image data using Huffman tables
    uint8_t* scan_data = current_pos;
    uint8_t* scan_end = end_pos;
    
    uint16_t* output_buffer = new uint16_t[65536];
    memset(output_buffer, 0, 65536 * sizeof(uint16_t));
    
    // Read compressed data
    processor->vtable->read_scan_data(processor, output_buffer, 65536, precision, 0);
    
    // Decode Huffman-coded data
    if (segments_end != segments_start) {
        // Process each segment
        for (void* segment = segments_start; segment != segments_end; segment += 4) {
            int segment_idx = (segment - segments_start) / 4;
            
            // Decode Huffman symbols and reconstruct pixel values
            for (int row = 0; row < num_lines; row++) {
                for (int col = 0; col < samples_per_line; col++) {
                    // Huffman decode process
                    int huffman_value = decode_huffman_symbol(huffman_tables[component_info], scan_data);
                    int pixel_value = reconstruct_pixel_value(huffman_value);
                    
                    // Store decoded pixel
                    int pixel_idx = row * samples_per_line + col;
                    output_buffer[pixel_idx] = pixel_value;
                }
            }
        }
    }
    
    // Apply color space conversion
    int color_mode = processor->color_space_mode;
    void* color_matrix = processor->color_matrix;
    
    for (int pixel = 0; pixel < total_image_pixels; pixel += 6) {
        int16_t r = output_buffer[pixel];
        int16_t g = output_buffer[pixel + 2];  
        int16_t b = output_buffer[pixel + 4];
        
        switch (color_mode) {
            case 0: // RGB to YUV
                {
                    int16_t y = r - 512;
                    int16_t u = g + ((-1802 * g - 3126 * b) >> 12);
                    int16_t v = b + ((29040 * g - 101 * b) >> 12);
                    
                    // Apply color matrix
                    for (int c = 0; c < 3; c++) {
                        int matrix_value = ((int*)color_matrix)[c];
                        int result = (matrix_value * (&y)[c]) >> 10;
                        result = (result < 0xFFFF) ? result : 0xFFFF;
                        result = (result > 0) ? result : 0;
                        output_buffer[pixel + c * 2] = result;
                    }
                }
                break;
                
            case 1:
            case 3: // Alternative color space
                {
                    int16_t adj_r = r;
                    int16_t adj_g = g;  
                    int16_t adj_b = b;
                    
                    // Color correction calculations
                    int y_val = adj_r * 0x4A8C + adj_g * 0x4AF5;
                    int u_val = adj_r * (-0x4A8) + adj_b * (-0x3DE7) + 0x4000;
                    int v_val = adj_g * (-0xA1) + adj_b * 0x7170 + 0x4000;
                    
                    y_val >>= 14;
                    u_val >>= 14;  
                    v_val >>= 14;
                    
                    // Apply color matrix
                    for (int c = 0; c < 3; c++) {
                        int matrix_value = ((int*)color_matrix)[c];
                        int result = (matrix_value * (&y_val)[c]) >> 10;
                        result = (result < 0xFFFF) ? result : 0xFFFF;
                        result = (result > 0) ? result : 0;
                        output_buffer[pixel + c * 2] = result;
                    }
                    
                    if (color_mode == 3) {
                        // Additional green channel processing
                        output_buffer[pixel + 2] <<= 2;
                    }
                }
                break;
                
            case 2: // CFA Bayer processing
                {
                    // Apply color matrix with different coefficients
                    for (int c = 0; c < 3; c++) {
                        int matrix_value = ((int*)color_matrix)[c];
                        int result = (matrix_value * (&r)[c]) >> 10;
                        result = (result < 0xFFFF) ? result : 0xFFFF; 
                        result = (result > 0) ? result : 0;
                        output_buffer[pixel + c * 2] = result;
                    }
                }
                break;
                
            default:
                throw RawCameraException("Unknown color space mode");
        }
    }
    
    // Cleanup
    delete[] image_data;
    delete[] output_buffer;
    delete[] jpeg_buffer;
    
    // Release completion block
    completion_block->release();
    release_object(completion_block);
}
