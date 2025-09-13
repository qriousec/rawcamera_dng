
void unpack_jpeg_tile_with_context_check(uint32_t arg0, camera_params* arg1, image_format* arg2, uint32_t arg3, void* arg4, uint32_t arg5, source_region* arg6, dest_region* arg7, int arg8) { // 0x1b28684ec
    void* retained_obj = _objc_retainAutoreleasedReturnValue(sub_1B291E434());
    
    if (sub_1B29CDC20(retained_obj, (void*)0x1F384A7C8) != 0) {
        cdng_lossless_jpeg_unpacker_unpack_tile(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    } else {
        unpackLosslessJPEGTile(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }
    
    _objc_release(retained_obj);
}

// 0x1b2865980
void unpackLosslessJPEGTile(void* logger, huffman_decoder_state* decoder_state, tile_params* params, uint32_t tile_x, uint32_t tile_y, uint8_t* jpeg_data, uint64_t data_size, uint64_t marker_offset, void* output_buffer, bool apply_gamma) {
    PACIBSP();
    
    // Stack canary setup
    uint64_t stack_canary = *((uint64_t*)0x1E7FDC060);
    
    // Get signpost ID and check for profiling
    void* signpost_id = sub_1B27E53F4();
    signpost_id = _objc_retainAutoreleasedReturnValue(signpost_id);
    
    uint64_t* spid_ptr = (uint64_t*)((char*)signpost_id + 0x98);
    uint64_t spid = *spid_ptr + 1;
    if (spid >= 2) {
        // Emit signpost for profiling
        if (_os_signpost_enabled(signpost_id)) {
            char buf[2] = {0};
            __os_signpost_emit_with_name_impl((void*)0x1B2773000, signpost_id, 1, spid, "CDNGLosslessJpegUnpacker::unpackTile", (void*)0x1B29AD08E, buf, 2);
        }
        _objc_release(signpost_id);
        return;
    }
    
    _objc_release(signpost_id);
    
    // Create block for processing
    void* block_impl = sub_1B2866B34;
    uint64_t block[4];
    block[0] = (uint64_t)block_impl;
    block[1] = (uint64_t)0x1E90B62D0;
    block[2] = spid;
    void* retained_block = _objc_retainBlock(block);
    
    // Initialize huffman tables array
    huffman_tables huff_tables = {0};
    
    // Validate JPEG marker
    uint8_t* current_pos = jpeg_data + 2;
    if (current_pos > jpeg_data + data_size) {
        throw std::runtime_error("RawCameraException");
    }
    
    uint16_t marker = __builtin_bswap16(*(uint16_t*)jpeg_data);
    if ((marker >> 16) != 0xFFD8) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Store parameters
    huffman_decoder_state* state = decoder_state;
    tile_params* tile_info = params;
    uint8_t gamma_flag = apply_gamma;
    
    // Parse JPEG segments
    while (true) {
        uint8_t* segment_start = current_pos;
        if (current_pos + 4 > jpeg_data + data_size) {
            throw std::runtime_error("RawCameraException");
        }
        
        uint16_t segment_marker = __builtin_bswap16(*(uint16_t*)current_pos);
        uint16_t segment_length = __builtin_bswap16(*(uint16_t*)(current_pos + 2));
        
        if (segment_marker < 0xFF01) {
            throw std::runtime_error("RawCameraException");
        }
        
        if (segment_length < 2 || segment_length > 0x102) {
            throw std::runtime_error("RawCameraException");
        }
        
        uint8_t* segment_end = current_pos + segment_length;
        if (segment_end - 2 > jpeg_data + data_size) {
            throw std::runtime_error("RawCameraException");
        }
        
        current_pos = segment_end - 2;
        
        // Process different segment types
        if (segment_marker <= 0xFFC3) {
            if (segment_marker == 0xFFC0) {
                // SOF0 - Start of Frame
                uint8_t precision = segment_start[11];
                uint32_t precision_bits = (precision >> 4) * precision + 0xFFFFFFFF;
                decoder_state->predictor_type = precision_bits & 3;
            } else if (segment_marker == 0xFFC3) {
                // Huffman table
                uint8_t* table_ptr = segment_start + 4;
                while (table_ptr < segment_end - 2) {
                    uint8_t table_id = *table_ptr;
                    if (table_id > 3) {
                        throw std::runtime_error("RawCameraException");
                    }
                    
                    void* table_data = (void*)((char*)huff_tables.tables + (table_id << 11));
                    huff_tables.tables[table_id] = table_data;
                    
                    uint16_t restart_marker = decoder_state->quantization_tables[0];
                    uint32_t version = (restart_marker == 0x101) ? 2 : 3;
                    
                    table_ptr++;
                    int result = create_huffman_decoder_tables(table_ptr, table_data, version);
                    if (result == 0) {
                        continue;
                    }
                }
                throw std::runtime_error("RawCameraException");
            }
        } else if (segment_marker == 0xFFDA) {
            // SOS - Start of Scan
            uint8_t components = segment_start[0];
            uint8_t* scan_data = segment_start + 1 + components * 2;
            
            if (scan_data + 2 > jpeg_data + data_size) {
                throw std::runtime_error("RawCameraException");
            }
            
            uint8_t predictor = *scan_data;
            uint8_t point_transform = *(scan_data + 3) & 0xF;
            decoder_state->restart_interval = decoder_state->restart_interval - point_transform;
            decoder_state->component_count = predictor;
        }
        
        if (segment_marker == 0xFFDA) {
            break;
        }
    }
    
    // Decode image data
    uint32_t image_height = decoder_state->height;
    if (image_height < 5 || image_height > 0x7FFF) {
        throw std::runtime_error("RawCameraException");
    }
    
    uint32_t image_width = decoder_state->width;
    uint32_t total_pixels = image_width * image_height;
    
    // Allocate decode buffer
    uint64_t buffer_size = (uint64_t)total_pixels << 2;
    int32_t* decode_buffer = (int32_t*)operator new[](buffer_size, (void*)0x1000C8077774924);
    decoder_state->padding_20[1] = (uint64_t)decode_buffer;
    _bzero(decode_buffer, buffer_size);
    
    decoder_state->padding_20[2] = 0x7FFFFFFF;
    
    // Validate dimensions and component parameters
    uint32_t tile_width = tile_info->width;
    uint32_t tile_height = tile_info->height;
    uint32_t total_width = tile_info->total_width;
    uint32_t total_height = tile_info->total_height;
    
    if (tile_width > 0x7CFF || tile_height >= 0x7D00 || 
        image_width >= 0x7D00 || total_width >= 0x7D00) {
        throw std::runtime_error("RawCameraException");
    }
    
    uint32_t pixel_count_check = tile_height * tile_width;
    uint32_t expected_count = total_width * image_width;
    if (pixel_count_check != expected_count) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Validate component count
    uint32_t components = image_height - 1;
    if (components >= 3) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Process scan data
    uint64_t scan_buffer_size = buffer_size << 1;
    uint16_t* scan_buffer = (uint16_t*)operator new[](scan_buffer_size, (void*)0x1000C8077774924);
    _bzero(scan_buffer, scan_buffer_size);
    
    // Get tile position and dimensions
    uint32_t tile_left = tile_info->width;
    uint32_t tile_top = tile_info->height;
    
    // Process image data with JPEG decoder
    uint32_t bit_count = 0;
    uint32_t bit_buffer = 0;
    
    for (uint32_t y = 0; y < total_width; y++) {
        if (current_pos >= jpeg_data + data_size) {
            throw std::runtime_error("RawCameraException");
        }
        
        uint64_t available_bytes = (jpeg_data + data_size) - current_pos;
        uint64_t copy_size = (scan_buffer_size < available_bytes) ? scan_buffer_size : available_bytes;
        
        _memmove(scan_buffer, current_pos, copy_size);
        
        // Initialize huffman decoding state
        void* huffman_state[3] = {0};
        int32_t* output_row = decode_buffer;
        
        // Decode row using huffman tables
        for (uint32_t component = 0; component < 3; component++) {
            uint32_t table_index = (bit_count + component) & 1;
            huffman_state[component] = (char*)decode_buffer + (table_index ? buffer_size : 0) * 2;
        }
        
        // Process each component
        for (uint32_t comp = 0; comp < image_width; comp++) {
            for (uint32_t x = 0; x < image_height * 2; x += 2) {
                // Huffman decode
                while (bit_count < 16) {
                    uint8_t byte = *current_pos;
                    bit_buffer = (bit_count << 16) | (bit_buffer & 0xFFFF);
                    
                    if (byte == 0xFF) {
                        current_pos += 2;
                    } else {
                        current_pos++;
                    }
                    
                    uint8_t next_byte = *current_pos;
                    if (next_byte == 0xFF) {
                        current_pos++;
                    }
                    
                    bit_buffer |= (byte << 8) | next_byte;
                    bit_count += 16;
                }
                
                // Lookup in huffman table
                void* table = huffman_state[0];
                uint32_t table_index = (bit_buffer << bit_count) >> 23;
                uint16_t huffman_code = ((uint16_t*)table)[table_index];
                uint32_t code_length = (huffman_code >> 10) & 0x1F;
                bit_count += code_length;
                
                int16_t decoded_value = 0;
                if (huffman_code > (8 << 12)) {
                    decoded_value = (int8_t)huffman_code;
                } else {
                    if (huffman_code & 0x8000) {
                        uint32_t extended_index = (bit_buffer << 16) >> 22;
                        uint16_t extended_code = ((uint16_t*)table)[extended_index];
                        uint32_t extended_length = (extended_code >> 10) & 0x1F;
                        bit_count += extended_length;
                    }
                    
                    uint32_t value_bits = huffman_code & 0x1F;
                    if (value_bits == 0) {
                        decoded_value = (huffman_code << 8) & 0x8000;
                    } else {
                        while (bit_count < 16) {
                            // Refill bit buffer
                            uint8_t byte = *current_pos;
                            bit_buffer = (bit_count << 16) | (bit_buffer & 0xFFFF);
                            
                            if (byte == 0xFF) {
                                current_pos += 2;
                            } else {
                                current_pos++;
                            }
                            
                            uint8_t next_byte = *current_pos;
                            if (next_byte == 0xFF) {
                                current_pos++;
                            }
                            
                            bit_buffer |= (byte << 8) | next_byte;
                            bit_count += 16;
                        }
                        
                        uint32_t raw_value = (bit_buffer << bit_count) >> (32 - value_bits);
                        uint32_t sign_extend = raw_value >> 31;
                        decoded_value = raw_value - ((sign_extend - 1) >> (32 - value_bits));
                        bit_count += value_bits;
                    }
                }
                
                // Apply prediction
                if (comp != 0) {
                    uint16_t prev_value = *(uint16_t*)((char*)huffman_state[1] + x);
                    
                    if (bit_count != 0) {
                        uint32_t predictor = decoder_state->component_count;
                        
                        if (predictor <= 3) {
                            switch (predictor) {
                                case 1:
                                    break;
                                case 2:
                                    decoded_value = prev_value;
                                    break;
                                case 3:
                                    decoded_value = *(uint16_t*)((char*)huffman_state[2] + x);
                                    break;
                            }
                        } else if (predictor <= 7) {
                            switch (predictor) {
                                case 4: {
                                    uint16_t left = prev_value;
                                    uint16_t above = *(uint16_t*)((char*)huffman_state[2] + x);
                                    decoded_value = left + decoded_value - above;
                                    break;
                                }
                                case 5: {
                                    uint16_t left = prev_value;
                                    uint16_t above = *(uint16_t*)((char*)huffman_state[2] + x);
                                    decoded_value = decoded_value + (left - above) / 2;
                                    break;
                                }
                                case 6: {
                                    uint16_t left = prev_value;
                                    uint16_t above = *(uint16_t*)((char*)huffman_state[2] + x);
                                    decoded_value = (left + decoded_value) >> 1;
                                    break;
                                }
                                case 7: {
                                    uint16_t left = prev_value;
                                    uint16_t above = *(uint16_t*)((char*)huffman_state[2] + x);
                                    decoded_value = left + (decoded_value - above) / 2;
                                    break;
                                }
                                default:
                                    decoded_value = 0;
                                    break;
                            }
                        }
                    } else {
                        uint32_t* base_value = (uint32_t*)((char*)huffman_state[2] + (x / 2) * 4);
                        int32_t adjusted = *base_value + decoded_value;
                        *base_value = adjusted;
                    }
                }
                
                // Store decoded value
                uint16_t final_value = prev_value + decoded_value;
                *(uint16_t*)((char*)huffman_state[1] + x) = final_value;
            }
        }
        
        // Update pointers for next row
        current_pos += copy_size;
    }
    
    // Apply gamma correction if requested
    if (apply_gamma) {
        // Initialize gamma tables if needed
        if (*(int64_t*)0x1ECB7F630 == -1) {
            initialize_once();
        }
        
        // Apply gamma correction to decoded data
        for (uint32_t row = 0; row < total_width; row++) {
            for (uint32_t col = 0; col < tile_width; col++) {
                uint16_t pixel_value = ((uint16_t*)huffman_state[0])[row * tile_width + col];
                uint16_t gamma_corrected = ((uint16_t*)0x1ECB7F638)[pixel_value];
                ((uint16_t*)output_buffer)[row * params->total_width + col] = gamma_corrected;
            }
        }
    }
    
    // Cleanup
    operator delete[](scan_buffer, (void*)0x1000C8077774924);
    operator delete[](decode_buffer, (void*)0x1000C8077774924);
    
    // Release block
    void* block_release_func = *(void**)((char*)retained_block + 0x10);
    ((void(*)(void*))block_release_func)(retained_block);
    _objc_release(retained_block);
    
    // Verify stack canary
    if (*(uint64_t*)0x1E7FDC060 != stack_canary) {
        __stack_chk_fail();
    }
}
