
void process_lossless_jpeg_data(jpeg_decoder_context* context) { // 0x1b2869fdc
    uint8_t stack_buffer[4096];
    uint64_t stack_canary;
    void* block_ptr;
    void* huffman_tables[2];
    uint32_t component_count;
    uint32_t restart_interval;
    uint32_t precision;
    uint32_t max_h_factor;
    uint32_t max_v_factor;
    uint32_t total_samples;
    uint32_t calculated_offset;
    uint32_t components_size;
    int32_t offset_difference;
    void* allocated_buffer = nullptr;
    uint16_t* output_ptr;
    uint8_t* data_ptr;
    uint32_t bits_available = 32;
    uint32_t bit_buffer = 0;
    uint32_t component_index = 0;
    
    // Get stack canary
    stack_canary = __stack_chk_guard;
    
    // Get logging context and check signpost
    void* log_context = sub_1B27E53F4();
    log_context = _objc_retainAutoreleasedReturnValue(log_context);
    
    if (context + 1 < 2) {
        if (_os_signpost_enabled(log_context)) {
            uint8_t signpost_buf[2] = {0};
            __os_signpost_emit_with_name_impl(
                &dword_1B2773000,
                log_context,
                1, // type
                context, // spid
                "CLosslessJpegRestartUnpacker",
                "%{public}s",
                signpost_buf,
                2
            );
        }
        
        _objc_release(log_context);
        
        // Create completion block
        block_ptr = ^{
            // Block implementation at 0x1b286ac68
        };
        block_ptr = _objc_retainBlock(block_ptr);
        
        offset_difference = calculate_offset_difference(context);
        
        uint32_t width = context->width;
        uint32_t height = context->height;
        
        // Get component count from vtable call
        uint32_t (*get_component_count)(jpeg_decoder_context*) = 
            (uint32_t (*)(jpeg_decoder_context*))((char*)context->vtable + 152);
        component_count = get_component_count(context);
        
        check_and_copy_value(&calculated_offset, &component_count);
        
        total_samples = height * width;
        data_ptr = (uint8_t*)(offset_difference + total_samples * 2);
        data_ptr = data_ptr - calculated_offset + dword_1B296C71C;
        uint8_t* scan_start = data_ptr - 128;
        
        // Read data from buffer
        void* data_buffer = context->data_buffer;
        void (*read_data)(void*, uint8_t*, uint32_t) = 
            (void (*)(void*, uint8_t*, uint32_t))((char*)data_buffer->vtable + 32);
        read_data(data_buffer, scan_start, calculated_offset);
        
        // Check for SOI marker (0xFFD8)
        uint16_t marker = __builtin_bswap16(*(uint16_t*)(data_ptr - 128));
        if ((marker >> 16) != 0xFFD8) {
            throw std::runtime_error("RawCameraException");
        }
        
        components_size = total_samples;
        restart_interval = 0;
        precision = 0;
        max_h_factor = 0;
        max_v_factor = 0;
        
        uint8_t* current_ptr = data_ptr - 126;
        huffman_tables[0] = nullptr;
        huffman_tables[1] = nullptr;
        
        uint16_t current_marker;
        
        // Parse JPEG segments
        while (true) {
            current_marker = __builtin_bswap16(*(uint16_t*)current_ptr);
            current_ptr += 2;
            
            if (current_marker == 0xFFFF) {
                continue;
            }
            
            if (current_marker < 0xFF01) {
                throw std::runtime_error("RawCameraException");
            }
            
            uint16_t segment_length = __builtin_bswap16(*(uint16_t*)current_ptr);
            if (segment_length < 258) {
                throw std::runtime_error("RawCameraException");
            }
            
            uint8_t* segment_data = current_ptr + 4;
            uint8_t* segment_end = segment_data + segment_length - 2;
            
            if (current_marker <= 0xFFD9) {
                if (current_marker == 0xFFC3 || current_marker == 0xFFC4) {
                    // DHT (Define Huffman Table)
                    while (segment_data < segment_end) {
                        uint8_t table_id = *segment_data;
                        if (table_id > 1) {
                            throw std::runtime_error("RawCameraException");
                        }
                        
                        huffman_tables[table_id] = &stack_buffer[table_id * 2048];
                        segment_data++;
                        
                        if (create_huffman_decoder_tables(segment_data, 2, huffman_tables[table_id]) != nullptr) {
                            continue;
                        } else {
                            throw std::runtime_error("RawCameraException");
                        }
                    }
                } else if (current_marker == 0xFFDA) {
                    // SOS (Start of Scan)
                    uint8_t component_count_in_scan = *segment_data;
                    uint32_t expected_length = component_count_in_scan * 2 + 6;
                    
                    if (component_count_in_scan < 1 || component_count_in_scan > 3 || 
                        segment_length != expected_length) {
                        throw std::runtime_error("RawCameraException");
                    }
                    
                    if (*(segment_data + component_count_in_scan * 2 + 1) != 1) {
                        throw std::runtime_error("RawCameraException");
                    }
                    
                    max_v_factor = *(segment_data + component_count_in_scan * 2 + 3) & 0xF;
                    break;
                }
            } else if (current_marker == 0xFFDD) {
                // DRI (Define Restart Interval)
                restart_interval = __builtin_bswap16(*(uint16_t*)(current_ptr + 4));
            }
            
            current_ptr = segment_end;
        }
        
        // Check if we have required data
        uint32_t scan_data_size = precision * restart_interval;
        if (scan_data_size > components_size) {
            throw std::runtime_error("RawCameraException");
        }
        
        if (restart_interval >= 1 && precision >= 1) {
            max_h_factor = precision / max_v_factor;
            restart_interval = restart_interval / max_h_factor;
        }
        
        uint32_t height_to_process = (restart_interval >= 1) ? restart_interval : precision;
        
        if (!huffman_tables[0] || !huffman_tables[1]) {
            throw std::runtime_error("RawCameraException");
        }
        
        if (precision <= 0) {
            goto cleanup;
        }
        
        // Decode the scan data
        uint8_t* scan_data_end = scan_start + calculated_offset;
        uint8_t* bit_stream_ptr = current_ptr;
        
        uint32_t dc_predictor = 0;
        uint32_t shift_amount = 8 - max_v_factor;
        uint32_t max_coefficient_bits = 8 + max_v_factor;
        uint32_t dc_table_offset = 1 << shift_amount;
        
        for (uint32_t row = 0; row < height_to_process; row++) {
            uint32_t samples_per_row = precision;
            uint32_t row_offset = (height_to_process - row) * samples_per_row;
            uint8_t* row_end = scan_data_end - calculated_offset;
            
            uint32_t bits_needed = (bits_available - 32) >> 3;
            uint64_t total_bits = row_offset << 3;
            
            if ((total_bits << 1) < (bits_needed << 4)) {
                throw std::runtime_error("RawCameraException");
            }
            
            // Allocate temporary buffer if needed
            if (!allocated_buffer) {
                int32_t remaining_data = calculated_offset - offset_difference;
                if (samples_per_row > remaining_data || bits_needed >= total_bits) {
                    throw std::runtime_error("RawCameraException");
                }
            }
            
            if (scan_data_end == calculated_offset) {
                throw std::runtime_error("RawCameraException");
            }
            
            uint64_t buffer_size = row_offset * total_bits | 6;
            allocated_buffer = operator new[](buffer_size);
            _bzero(allocated_buffer, max_coefficient_bits);
            
            _memmove(allocated_buffer, calculated_offset - 6, bits_needed + 6);
            calculated_offset = (uint8_t*)allocated_buffer + 6;
            scan_data_end = calculated_offset + buffer_size;
            
            // Check for restart marker
            if (row == height_to_process) {
                uint8_t* marker_search = calculated_offset - 8;
                for (int i = 12; i > 0; i--) {
                    if (*marker_search == 0xFF && (*(marker_search + 1) & 0xF8) == 0xD0) {
                        dc_predictor = 0;
                        height_to_process += component_count;
                        calculated_offset = marker_search + 2;
                        bits_available = 32;
                        dc_table_offset = dc_table_offset;
                        break;
                    }
                    marker_search++;
                }
                
                if (i == 0) {
                    throw std::runtime_error("RawCameraException");
                }
            }
            
            output_ptr = (uint16_t*)offset_difference;
            
            // Decode samples in the row
            for (uint32_t col = 0; col < samples_per_row; col++) {
                // Refill bit buffer if needed
                if (bits_available < 16) {
                    uint8_t byte1 = *calculated_offset;
                    bit_buffer <<= 16;
                    
                    if (byte1 == 0xFF) {
                        calculated_offset += 2;
                        calculated_offset += 1;
                    } else {
                        calculated_offset += 1;
                    }
                    
                    uint8_t byte2 = *calculated_offset;
                    if (byte2 == 0xFF) {
                        calculated_offset += 1;
                    }
                    
                    bit_buffer |= (byte1 << 8) | byte2;
                    bits_available += 16;
                }
                
                // Decode DC coefficient using Huffman table
                uint32_t code_bits = bit_buffer << bits_available;
                uint32_t table_index = code_bits >> 23;
                uint16_t huffman_entry = ((uint16_t*)huffman_tables[0])[table_index];
                uint32_t code_length = (huffman_entry >> 10) & 0x1F;
                bits_available += code_length;
                
                int32_t dc_value;
                if (huffman_entry > (8 << 12)) {
                    dc_value = (int8_t)huffman_entry;
                } else {
                    if (huffman_entry & 0x8000) {
                        uint32_t extended_index = (code_bits >> 16) & 0x3FF;
                        huffman_entry = ((uint16_t*)huffman_tables[0])[extended_index];
                        code_length = (huffman_entry >> 10) & 0x1F;
                        bits_available += code_length;
                    }
                    
                    uint32_t magnitude_bits = huffman_entry & 0x1F;
                    if (magnitude_bits == 0) {
                        dc_value = (huffman_entry << 8) & 0x8000;
                    } else {
                        if (bits_available < 16) {
                            // Refill buffer again
                            uint8_t byte1 = *calculated_offset;
                            bit_buffer <<= 16;
                            
                            if (byte1 == 0xFF) {
                                calculated_offset += 2;
                                calculated_offset += 1;
                            } else {
                                calculated_offset += 1;
                            }
                            
                            uint8_t byte2 = *calculated_offset;
                            if (byte2 == 0xFF) {
                                calculated_offset += 1;
                            }
                            
                            bit_buffer |= (byte1 << 8) | byte2;
                            bits_available += 16;
                        }
                        
                        uint32_t magnitude_code = bit_buffer << bits_available;
                        uint32_t magnitude_mask = ~((uint32_t)0) << (32 - magnitude_bits);
                        uint32_t magnitude_value = magnitude_code >> (32 - magnitude_bits);
                        uint32_t sign_bit = magnitude_code >> 31;
                        magnitude_value -= (sign_bit - 1) >> (32 - magnitude_bits);
                        dc_value = magnitude_value;
                        bits_available += magnitude_bits;
                    }
                }
                
                // Decode AC coefficient
                if (bits_available < 16) {
                    // Refill buffer
                    uint8_t byte1 = *calculated_offset;
                    bit_buffer <<= 16;
                    
                    if (byte1 == 0xFF) {
                        calculated_offset += 2;
                        calculated_offset += 1;
                    } else {
                        calculated_offset += 1;
                    }
                    
                    uint8_t byte2 = *calculated_offset;
                    if (byte2 == 0xFF) {
                        calculated_offset += 1;
                    }
                    
                    bit_buffer |= (byte1 << 8) | byte2;
                    bits_available += 16;
                }
                
                code_bits = bit_buffer << bits_available;
                table_index = code_bits >> 23;
                huffman_entry = ((uint16_t*)huffman_tables[1])[table_index];
                code_length = (huffman_entry >> 10) & 0x1F;
                bits_available += code_length;
                
                int32_t ac_value;
                if (huffman_entry > (8 << 12)) {
                    ac_value = (int8_t)huffman_entry;
                } else {
                    if (huffman_entry & 0x8000) {
                        uint32_t extended_index = (code_bits >> 16) & 0x3FF;
                        huffman_entry = ((uint16_t*)huffman_tables[1])[extended_index];
                        code_length = (huffman_entry >> 10) & 0x1F;
                        bits_available += code_length;
                    }
                    
                    uint32_t magnitude_bits = huffman_entry & 0x1F;
                    if (magnitude_bits == 0) {
                        ac_value = (huffman_entry << 8) & 0x8000;
                    } else {
                        if (bits_available < 16) {
                            // Refill buffer again
                            uint8_t byte1 = *calculated_offset;
                            bit_buffer <<= 16;
                            
                            if (byte1 == 0xFF) {
                                calculated_offset += 2;
                                calculated_offset += 1;
                            } else {
                                calculated_offset += 1;
                            }
                            
                            uint8_t byte2 = *calculated_buffer;
                            if (byte2 == 0xFF) {
                                calculated_offset += 1;
                            }
                            
                            bit_buffer |= (byte1 << 8) | byte2;
                            bits_available += 16;
                        }
                        
                        uint32_t magnitude_code = bit_buffer << bits_available;
                        uint32_t magnitude_mask = ~((uint32_t)0) << (32 - magnitude_bits);
                        uint32_t magnitude_value = magnitude_code >> (32 - magnitude_bits);
                        uint32_t sign_bit = magnitude_code >> 31;
                        magnitude_value -= (sign_bit - 1) >> (32 - magnitude_bits);
                        ac_value = magnitude_value;
                        bits_available += magnitude_bits;
                    }
                }
                
                dc_predictor += dc_value;
                dc_table_offset += ac_value;
                
                uint16_t dc_shifted = dc_predictor << max_v_factor;
                uint16_t ac_shifted = dc_table_offset << max_v_factor;
                
                if (samples_per_row >= 3) {
                    // Process multiple components
                    for (int comp = 2; comp < samples_per_row; comp += 2) {
                        if (calculated_offset > scan_data_end) {
                            throw std::runtime_error("RawCameraException");
                        }
                        
                        // Similar decoding process for additional components
                        // ... (omitted for brevity but follows same pattern)
                        
                        *output_ptr = dc_shifted;
                        *(output_ptr + 1) = ac_shifted;
                        output_ptr += 4;
                    }
                }
                
                *output_ptr = dc_shifted;
                *(output_ptr + 1) = ac_shifted;
                output_ptr += 4;
            }
        }
        
cleanup:
        if (allocated_buffer) {
            operator delete[](allocated_buffer);
        }
        
        // Call block completion handler
        void (*block_invoke)(void*) = *(void (**)(void*))((char*)block_ptr + 16);
        block_invoke(block_ptr);
        
        _objc_release(block_ptr);
    }
    
    _objc_release(log_context);
    
    // Check stack canary
    if (__stack_chk_guard != stack_canary) {
        __stack_chk_fail();
    }
}
