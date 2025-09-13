
// decode_hasselblad_huffman_data (0x1b2858648)
void decode_hasselblad_huffman_data(hasselblad_unpacker_context* unpacker_ctx) {
    void* autoreleased_obj;
    void* retained_block;
    void* data_buffer;
    uint64_t offset_diff;
    uint32_t width, height;
    void* vtable_func;
    int32_t buffer_size;
    int32_t signed_size;
    void* allocated_buffer = nullptr;
    void* jpeg_data;
    uint32_t total_pixels;
    void* huffman_tables[2] = {nullptr, nullptr};
    uint32_t restart_interval = 0;
    uint32_t component_count = 0;
    uint32_t image_width = 0, image_height = 0;
    uint32_t total_mcu_count = 0;
    bool found_huffman_tables = false;
    void* temp_buffer = nullptr;
    
    // Get autoreleased object and retain it
    autoreleased_obj = sub_1B27E53F4();
    autoreleased_obj = _objc_retainAutoreleasedReturnValue(autoreleased_obj);
    
    // Check if unpacker_ctx parameter is valid (>= 2)
    if (unpacker_ctx + 1 >= (hasselblad_unpacker_context*)2) {
        // Log signpost if enabled
        if (_os_signpost_enabled(autoreleased_obj)) {
            uint16_t buf = 0;
            __os_signpost_emit_with_name_impl(&dword_1B2773000, autoreleased_obj, 1, 
                (uint64_t)unpacker_ctx, "CHasselbladUnpacker", &buf, 2);
        }
    }
    
    _objc_release(autoreleased_obj);
    
    // Create block for callback
    void* block_literal[] = {
        &_NSConcreteStackBlock,
        0x40000000,
        sub_1B2859034,  // block implementation
        &unk_1E90B62D0, // block descriptor
        unpacker_ctx    // captured variable
    };
    retained_block = _objc_retainBlock(block_literal);
    
    // Calculate offset difference
    offset_diff = calculate_offset_difference(unpacker_ctx);
    
    // Get width and height from context
    width = unpacker_ctx->width;
    height = unpacker_ctx->height;
    
    // Call virtual function to get buffer size
    vtable_func = unpacker_ctx->vtable;
    buffer_size = ((int32_t (*)(hasselblad_unpacker_context*))((char*)vtable_func + 0x98))(unpacker_ctx);
    
    // Check and copy value
    check_and_copy_value(&signed_size, &buffer_size);
    
    // Calculate total pixels and buffer requirements
    total_pixels = width * height;
    uint64_t required_size = offset_diff + (total_pixels * 2) + dword_1B296C71C - signed_size;
    uint64_t buffer_end = required_size - 0x80;
    
    // Allocate buffer if needed
    if (buffer_end >= offset_diff) {
        allocated_buffer = operator new[](signed_size + 0x80, 0x1000C8077774924);
        _bzero(allocated_buffer, signed_size + 0x80);
        jpeg_data = allocated_buffer;
    } else {
        jpeg_data = nullptr;
    }
    
    // Read data from data buffer
    void* data_source = unpacker_ctx->data_buffer;
    ((void (*)(void*, void*, size_t))((char*)data_source + 0x20))(data_source, allocated_buffer, signed_size);
    
    // Check for JPEG SOI marker (0xFFD8)
    uint16_t* jpeg_ptr = (uint16_t*)allocated_buffer;
    uint16_t first_marker = __builtin_bswap16(*jpeg_ptr);
    if ((first_marker >> 16) != 0xFFD8) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Parse JPEG segments
    uint8_t* current_pos = (uint8_t*)allocated_buffer + 2;
    uint8_t* end_pos = (uint8_t*)allocated_buffer + signed_size;
    
    while (current_pos < end_pos) {
        // Read marker
        uint16_t marker = __builtin_bswap16(*(uint16_t*)current_pos);
        current_pos += 2;
        
        if (marker == 0xFFFF) {
            continue; // Skip padding bytes
        }
        
        // Check if it's a valid marker
        if (marker < 0xFF01) {
            throw std::runtime_error("RawCameraException");
        }
        
        // Read segment length
        uint16_t length = __builtin_bswap16(*(uint16_t*)current_pos);
        if (length < 2) {
            throw std::runtime_error("RawCameraException");
        }
        
        uint8_t* segment_data = current_pos + 2;
        uint8_t* segment_end = segment_data + length - 2;
        
        if (marker == 0xFFC3) { // SOF3 - Start of Frame (lossless)
            // Parse frame header
            image_height = __builtin_bswap16(*(uint16_t*)(segment_data + 1));
            image_width = __builtin_bswap16(*(uint16_t*)(segment_data + 3));
            component_count = segment_data[5];
            total_mcu_count = image_width * component_count;
        }
        else if (marker == 0xFFDA) { // SOS - Start of Scan
            uint8_t scan_component_count = segment_data[0];
            if (scan_component_count - 1 > 2) {
                throw std::runtime_error("RawCameraException");
            }
            
            uint32_t expected_length = scan_component_count * 2 + 6;
            if (length != expected_length) {
                throw std::runtime_error("RawCameraException");
            }
            
            // Check precision (should be 8 bits)
            if (segment_data[scan_component_count * 2 + 1] != 8) {
                setup_huffman_decoder(unpacker_ctx);
                return;
            }
            
            break; // Found start of scan, exit parsing loop
        }
        else if (marker == 0xFFC4) { // DHT - Define Huffman Table
            uint8_t* table_ptr = segment_data;
            while (table_ptr < segment_end) {
                uint8_t table_info = *table_ptr;
                if (table_info > 1) {
                    throw std::runtime_error("RawCameraException");
                }
                
                // Set huffman table pointer
                huffman_tables[table_info] = &huffman_tables[0] + (table_info << 11);
                table_ptr++;
                
                // Create huffman decoder tables
                table_ptr = create_huffman_decoder_tables(table_ptr, huffman_tables[table_info], 3);
                if (table_ptr == nullptr) {
                    throw std::runtime_error("RawCameraException");
                }
            }
            found_huffman_tables = true;
        }
        
        current_pos = segment_end;
    }
    
    // Check if we found the required data
    if (total_mcu_count * image_height > total_pixels) {
        throw std::runtime_error("RawCameraException");
    }
    
    if (huffman_tables[0] == nullptr || huffman_tables[1] == nullptr) {
        throw std::runtime_error("RawCameraException");
    }
    
    if (huffman_tables[1] == nullptr || huffman_tables[0] == nullptr) {
        throw std::runtime_error("RawCameraException");
    }
    
    if (image_height == 0) {
        setup_huffman_decoder(unpacker_ctx);
        goto cleanup;
    }
    
    // Decode huffman data
    uint32_t row = 0;
    uint32_t bits_available = 0;
    uint32_t bit_buffer = 0;
    uint32_t byte_pos = 0;
    uint8_t* output_ptr = (uint8_t*)offset_diff;
    
    while (row < image_height) {
        // Calculate output buffer requirements
        uint32_t row_offset = (image_height - row) * total_mcu_count;
        uint64_t bit_offset = (current_pos - (uint8_t*)allocated_buffer) + ((0x27 - 0x20) >> 3);
        uint64_t bit_position = bit_offset << 3;
        
        if (bit_position >= row_offset * 2) {
            throw std::runtime_error("RawCameraException");
        }
        
        // Reallocate buffer if needed
        if (huffman_tables[0] == nullptr || huffman_tables[1] == nullptr) {
            if (current_pos - (uint8_t*)allocated_buffer < (int64_t)(total_mcu_count * 4)) {
                if (bit_offset < total_mcu_count * 4) {
                    // Allocate new buffer
                    uint64_t new_size = (total_mcu_count * 4 * row_offset) | 4;
                    void* new_buffer = operator new[](new_size, 0x1000C8077774924);
                    _bzero(new_buffer, row_offset);
                    
                    // Copy existing data
                    if (temp_buffer) {
                        operator delete[](temp_buffer, 0x1000C8077774924);
                    }
                    
                    size_t copy_size = std::min((size_t)row_offset, bit_offset + 4);
                    _memmove(new_buffer, (uint8_t*)allocated_buffer - 4, copy_size);
                    
                    allocated_buffer = (uint8_t*)new_buffer + 4;
                    current_pos = allocated_buffer + bit_offset;
                    temp_buffer = new_buffer;
                }
            }
        }
        
        // Decode MCUs for this row
        if (total_mcu_count > 0) {
            uint32_t mcu_index = 0;
            uint32_t dc_predictor1 = 0x8000;
            uint32_t dc_predictor2 = 0x8000;
            
            while (mcu_index < total_mcu_count) {
                // Ensure we have enough bits
                while (bits_available < 16) {
                    if ((byte_pos & 3) == 0) {
                        bit_buffer = (bit_buffer << 16) | (allocated_buffer[3] << 8) | allocated_buffer[2];
                    } else {
                        bit_buffer = (bit_buffer << 16) | (allocated_buffer[2] << 8) | allocated_buffer[3];
                    }
                    byte_pos += 2;
                    allocated_buffer += 2;
                    bits_available = 16;
                }
                
                // Decode DC coefficient for first component
                uint32_t lookup_bits = (bit_buffer << bits_available) >> 0x17;
                uint16_t dc_entry = ((uint16_t*)huffman_tables[1])[lookup_bits];
                uint32_t dc_bits = (dc_entry >> 0xA) & 0x1F;
                
                uint32_t dc_value = 0;
                uint32_t dc_sign = 0;
                
                if (dc_entry <= (8 << 12)) {
                    bits_available += dc_bits;
                    if (dc_entry & 0x8000) {
                        uint32_t extra_bits = (bit_buffer << 16) >> 0x16;
                        uint16_t extra_entry = ((uint16_t*)huffman_tables[1])[extra_bits];
                        dc_bits += (extra_entry >> 0xA) & 0x1F;
                        bits_available += (extra_entry >> 0xA) & 0x1F;
                    }
                    dc_value = dc_entry & 0x1F;
                    dc_sign = (dc_entry << 8) & 0x8000;
                } else {
                    int8_t symbol = (int8_t)dc_entry;
                    dc_value = (symbol < 0) ? -symbol : symbol;
                    dc_value &= 0x7F;
                    dc_value = dword_1B296ECE4[dc_value];
                    bits_available = dc_bits + bits_available - dc_value;
                }
                
                // Decode DC difference
                if (dc_value > 0) {
                    while (bits_available < 16) {
                        if ((byte_pos & 3) == 0) {
                            bit_buffer = (bit_buffer << 16) | (allocated_buffer[3] << 8) | allocated_buffer[2];
                        } else {
                            bit_buffer = (bit_buffer << 16) | (allocated_buffer[2] << 8) | allocated_buffer[3];
                        }
                        byte_pos += 2;
                        allocated_buffer += 2;
                        bits_available = 16;
                    }
                    
                    uint32_t diff_bits = (bit_buffer << bits_available) >> (32 - dc_value);
                    uint32_t sign_bit = (bit_buffer << bits_available) >> 31;
                    uint32_t dc_diff = diff_bits - ((sign_bit - 1) >> (32 - dc_value));
                    
                    if (dc_value > 15) {
                        dc_diff = 0x8000;
                    }
                    bits_available += dc_value;
                }
                
                dc_predictor1 += dc_diff;
                *(uint16_t*)output_ptr = dc_predictor1;
                
                // Decode AC coefficient (second component)
                while (bits_available < 16) {
                    if ((byte_pos & 3) == 0) {
                        bit_buffer = (bit_buffer << 16) | (allocated_buffer[3] << 8) | allocated_buffer[2];
                    } else {
                        bit_buffer = (bit_buffer << 16) | (allocated_buffer[2] << 8) | allocated_buffer[3];
                    }
                    byte_pos += 2;
                    allocated_buffer += 2;
                    bits_available = 16;
                }
                
                lookup_bits = (bit_buffer << bits_available) >> 0x17;
                uint16_t ac_entry = ((uint16_t*)huffman_tables[0])[lookup_bits];
                uint32_t ac_bits = (ac_entry >> 0xA) & 0x1F;
                
                uint32_t ac_value = 0;
                if (ac_entry <= (8 << 12)) {
                    bits_available += ac_bits;
                    if (ac_entry & 0x8000) {
                        uint32_t extra_bits = (bit_buffer << 16) >> 0x16;
                        uint16_t extra_entry = ((uint16_t*)huffman_tables[0])[extra_bits];
                        ac_bits += (extra_entry >> 0xA) & 0x1F;
                        bits_available += (extra_entry >> 0xA) & 0x1F;
                    }
                    ac_value = ac_entry & 0x1F;
                    if (ac_value == 0) {
                        goto store_second_component;
                    }
                } else {
                    int8_t symbol = (int8_t)ac_entry;
                    ac_value = (symbol < 0) ? -symbol : symbol;
                    ac_value &= 0x7F;
                    ac_value = dword_1B296ECE4[ac_value];
                    bits_available = ac_bits + bits_available - ac_value;
                    if (ac_value == 0) {
                        goto store_second_component;
                    }
                }
                
                // Decode AC difference
                while (bits_available < 16) {
                    if ((byte_pos & 3) == 0) {
                        bit_buffer = (bit_buffer << 16) | (allocated_buffer[3] << 8) | allocated_buffer[2];
                    } else {
                        bit_buffer = (bit_buffer << 16) | (allocated_buffer[2] << 8) | allocated_buffer[3];
                    }
                    byte_pos += 2;
                    allocated_buffer += 2;
                    bits_available = 16;
                }
                
                uint32_t ac_diff_bits = (bit_buffer << bits_available) >> (32 - ac_value);
                uint32_t ac_sign_bit = (bit_buffer << bits_available) >> 31;
                uint32_t ac_diff = ac_diff_bits - ((ac_sign_bit - 1) >> (32 - ac_value));
                
                if (ac_value > 15) {
                    ac_diff = 0x8000;
                }
                bits_available += ac_value;
                
store_second_component:
                dc_predictor2 += ac_diff;
                *(uint16_t*)(output_ptr + 2) = dc_predictor2;
                
                output_ptr += 4;
                mcu_index += 2;
            }
        }
        
        row++;
    }
    
cleanup:
    // Clean up allocated buffers
    if (temp_buffer) {
        operator delete[](temp_buffer, 0x1000C8077774924);
    }
    
    if (huffman_tables[0]) {
        operator delete[](huffman_tables[0], 0x1000C8077774924);
    }
    
    // Execute retained block and release it
    ((void (*)(void*))((char*)retained_block + 0x10))(retained_block);
    _objc_release(retained_block);
}
