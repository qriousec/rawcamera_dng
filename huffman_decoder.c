
uint64_t create_huffman_decoder_tables(uint8_t *input_data, uint16_t *output_table, int table_type) { // 0x1b280de48
    uint64_t allocated_size = 0;
    
    if (input_data == 0) {
        goto cleanup;
    }
    
    if (output_table == 0) {
        goto cleanup;
    }
    
    uint8_t *data_ptr = input_data;
    int sum = 0;
    int weighted_sum = 0;
    int bit_position = 16;
    int multiplier = 1;
    int mask = 0xFE00;
    uint8_t *current_ptr = input_data;
    
    // Process input bytes to calculate sums
    do {
        uint8_t byte_val = *current_ptr++;
        int shift_amount = bit_position - 1;
        
        if (byte_val == 0) {
            break;
        }
        
        int shifted_mult = multiplier << shift_amount;
        int adjusted_byte = (byte_val - 1) << shift_amount;
        int combined = shifted_mult | weighted_sum;
        int result = adjusted_byte + weighted_sum;
        combined |= result;
        combined &= 0xFFFF007F;
        
        if (combined != 0) {
            int masked = weighted_sum & result;
            int cleared = mask & ~masked;
            if (cleared != 0) {
                allocated_size = 0;
                goto cleanup;
            }
        }
        
        sum += byte_val;
        int shifted_byte = byte_val << shift_amount;
        weighted_sum = shifted_byte + weighted_sum;
        bit_position--;
    } while (bit_position != 0);
    
    uint64_t type_info = 0x1000C80789AEA99;
    int size_calc = sum - weighted_sum;
    size_calc += 0x10000;
    
    if (weighted_sum < 0x10000) {
        size_calc = sum;
    }
    
    uint64_t alloc_size = ((uint64_t)size_calc << 3) + 8;
    
    if (size_calc == -1) {
        allocated_size = ~0ULL;
    } else {
        allocated_size = alloc_size;
    }
    
    huffman_entry *table = (huffman_entry*)__ZnamSt19__type_descriptor_t(allocated_size, type_info);
    _bzero(table, allocated_size);
    
    int entry_idx = 0;
    int code_sum = 0;
    int code_count = 0;
    uint8_t *end_ptr = input_data + 16;
    int adjusted_type = table_type - 2;
    huffman_entry *current_entry = table + 1;
    int mask_val = 0xF;
    int increment = 1;
    uint32_t *lookup_table = (uint32_t*)(0x1B296C808);
    
    // Build initial entries
    for (int i = 0; i < 16; i++) {
        uint8_t count = input_data[i];
        if (count == 0) {
            continue;
        }
        
        int shift_bits = mask_val - i;
        huffman_entry *entry = current_entry + code_sum;
        uint8_t *symbol_ptr = end_ptr + code_sum;
        code_sum += count;
        int multiplier = increment << shift_bits;
        
        for (int j = 0; j < count; j++) {
            uint8_t symbol = *symbol_ptr++;
            
            if (symbol >= 16) {
                if (adjusted_type > 2) {
                    continue;
                }
                symbol = lookup_table[adjusted_type];
            }
            
            int high_nibble = symbol >> 4;
            if (table_type == 4) {
                high_nibble = 0;
            }
            
            entry->start_code = code_count;
            entry->symbol = symbol - high_nibble;
            entry->code_length = i + 1;
            entry->padding = 0;
            
            code_count += multiplier;
            entry++;
        }
    }
    
    // Fill remaining entries if needed
    if (code_count < 0x10000) {
        huffman_entry *fill_entry = table + code_sum + 1;
        
        while (code_count < 0x10000) {
            fill_entry->start_code = code_count;
            fill_entry->code_length = 16;
            fill_entry->symbol = 0;
            fill_entry->padding = 0;
            
            code_count++;
            fill_entry++;
        }
        
        code_sum = 0x10000;
    }
    
    // Store final count
    table[code_sum].start_code = 0x10000;
    huffman_entry *table_base = table + 2;
    uint8_t first_length = table[1].code_length;
    
    // Process entries based on length
    if (first_length < 10) {
        // SIMD optimized processing for short codes
        // [Complex SIMD code block omitted for brevity - processes entries in vectorized manner]
    }
    
    // Handle longer codes
    if (code_sum < 0x10000) {
        // Process remaining entries with different encoding
        // [Additional processing code omitted for brevity]
    }
    
    // Final SIMD processing block
    // [Complex SIMD processing code omitted for brevity]
    
    // Final cleanup pass
    if (code_sum < 0x10000) {
        for (int i = 0; i < code_sum && code_sum < 0x10000; i++) {
            int table_idx = table_base[i * 2].start_code;
            if (table_idx < 0xFE00) {
                continue;
            }
            
            // Process entry based on table type
            huffman_entry *entry = &table[i * 2];
            uint8_t length = entry[1].code_length;
            uint8_t symbol = entry[1].symbol;
            
            // Calculate final encoding
            int encoding = 0;
            if (table_type == 1) {
                encoding = (length & 0xF0) != 0 ? (length & 0xF0) : 0x3F0;
                if (length == 0xFF) {
                    encoding = 0;
                }
            }
            
            encoding = (encoding | (length << 10)) | (symbol & 0xF);
            
            // Fill table entries
            while (code_sum < table_idx) {
                if (code_sum >= 0xFE00) {
                    int index = 0x200 | (code_sum & 0x1FF);
                    output_table[index] = encoding;
                }
                code_sum++;
            }
        }
    }
    
    __ZdaPvSt19__type_descriptor_t(table, type_info);
    goto cleanup;

cleanup:
    return allocated_size;
}

uint64_t jpeg_decoder_process() {
    uint64_t stack_canary = *(uint64_t*)0x1E7FDC060;
    decoder_context *ctx = (decoder_context*)MEMORY[0x1EF4780A8]();

    // Initialize fixed pattern data
    __int128 pattern_data = {0x0000800000008000, 0x0000800000008000}; // xmmword_1B296F2D0
    __int128 neighbor_offsets = {0x0001000000000001, 0x00000000FFFFFFFF}; // xmmword_1B296F2A0

    int32_t height = ctx->height;
    int32_t block_width = ctx->block_width;
    int32_t width = ctx->width;
    int32_t block_height = ctx->block_height;

    uint32_t num_blocks = (ctx->buffer_end - ctx->buffer_start) >> 2;
    uint32_t buffer_size;
    check_and_extract_lower_32bits(&buffer_size, &num_blocks);

    // Validate dimensions
    if (width - 100000 < 0xFFFE7961) {
        std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(exception, "RawCameraException");
        throw_exception(exception);
    }
    if ((unsigned int)(height - 100000) < 0xFFFE7961) {
        std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(exception, "RawCameraException");
        throw_exception(exception);
    }
    if (block_width - 100000 < 0xFFFE7961) {
        std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(exception, "RawCameraException");
        throw_exception(exception);
    }
    if (block_height - 100000 < 0xFFFE7961) {
        std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(exception, "RawCameraException");
        throw_exception(exception);
    }

    if (buffer_size >> 5 >= 0x271) {
        std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(exception, "RawCameraException");
        throw_exception(exception);
    }

    // Validate block count
    int compression_type = ctx->compression_type;
    char has_compression = compression_type != 0;
    char is_mode2 = compression_type == 2;

    int expected_blocks = ((height + block_height - 1) / block_height) >> has_compression;
    expected_blocks *= ((width + block_width - 1) / block_width) >> is_mode2;

    if (expected_blocks != buffer_size) {
        std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(exception, "RawCameraException");
        throw_exception(exception);
    }

    // Set up neighbor offsets for compression modes
    if (compression_type == 1) {
        neighbor_offsets = (__int128){width + 1, 1 - width, width + 1, 1 - width};
    } else if (compression_type == 2) {
        ((int32_t*)&neighbor_offsets)[1] = width;
        ((int32_t*)&neighbor_offsets)[3] = -width;
    }

    // Calculate sum of block sizes
    uint64_t block_start = ctx->block_offsets_start;
    uint64_t block_end = ctx->block_offsets_end;
    uint32_t sum = 0;

    if (block_start != block_end) {
        uint32_t *ptr = (uint32_t*)block_start;
        uint32_t *end = (uint32_t*)block_end;
        do {
            int temp_sum = sum;
            sum = *(uint32_t*)add_with_overflow_check_0x1b27895c8(&temp_sum, ptr);
            ptr++;
        } while (ptr != end);
    }

    // Validate sum against input stream size
    uint32_t stream_size = (*(uint64_t(**)(uint64_t))(**(uint64_t**)(ctx->input_stream) + 16))(ctx->input_stream);
    if ((sum & 0x80000000) == 0 && sum >= stream_size) {
        std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error(exception, "RawCameraException");
        throw_exception(exception);
    }

    // Find maximum block size
    uint32_t *block_offsets = (uint32_t*)ctx->block_offsets_start;
    uint32_t *block_end_ptr = (uint32_t*)ctx->block_offsets_end;
    uint32_t *max_block_ptr = block_offsets;

    if (block_offsets != block_end_ptr) {
        uint32_t *next = block_offsets + 1;
        if (next != block_end_ptr) {
            uint32_t max_val = *block_offsets;
            uint32_t *check_ptr = block_offsets + 1;
            do {
                uint32_t val = *check_ptr;
                if (max_val <= val) {
                    if (max_val < val) {
                        max_block_ptr = next;
                    }
                    max_val = val;
                }
                next = check_ptr + 1;
                check_ptr++;
            } while (check_ptr != block_end_ptr);
        }
    }

    // Allocate decode buffer
    size_t decode_buffer_size = *max_block_ptr + 64;
    uint16_t *decode_buffer = (uint16_t*)operator new[](decode_buffer_size, 0x1000C8077774924);
    bzero(decode_buffer, decode_buffer_size);

    if (buffer_size == 0) {
        operator delete[](decode_buffer, 0x1000C8077774924);
        return 0;
    }

    // Process each JPEG block
    int component_count = 0;
    int bits_per_sample = 0;

    uint16_t *temp_decode_ptr = decode_buffer + 1;
    uint32_t samples_per_block = 8 * (block_width << is_mode2);
    uint32_t double_samples = 2 * (block_width << is_mode2);
    int total_rows = (block_height << has_compression) * width;
    uint64_t rows_to_process = block_height <= 1 ? 1 : block_height;
    uint32_t blocks_per_row = width / block_width;

    for (uint64_t block_idx = 0; block_idx < buffer_size; block_idx++) {
        uint64_t block_offset = *(uint32_t*)(ctx->block_offsets_start + 4 * block_idx);
        uint64_t input_stream = ctx->input_stream;

        int64_t temp_offset;
        check_and_copy_value(&temp_offset, ctx->buffer_start + 4 * block_idx);

        // Read block data from input stream
        (*(void(**)(uint64_t, int64_t*, uint32_t))(**(uint64_t**)input_stream + 40))(input_stream, &temp_offset, 0);
        (*(void(**)(uint64_t, uint16_t*, uint32_t))(**(uint64_t**)input_stream + 32))(
            input_stream, decode_buffer, *(uint32_t*)(ctx->block_offsets_start + 4 * block_idx));

        // Check JPEG SOI marker
        if (bswap32(*decode_buffer) >> 16 != 0xFFD8) {
            std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
            std::runtime_error::runtime_error(exception, "RawCameraException");
            __cxa_throw(exception, MEMORY[0x1E7FD8A98], MEMORY[0x1E7FD8920]);
        }

        uint16_t *block_end = (uint16_t*)((char*)decode_buffer + block_offset);
        void *huffman_tables[4] = {0};

        // Huffman decode tables storage
        __int128 dc_values[4] = {0};
        __int128 decode_tables[513] = {0};
        decode_tables[0] = pattern_data;

        uint16_t *current_ptr = temp_decode_ptr;

        // Parse JPEG segments
        do {
            uint32_t marker = __rev16(*current_ptr);
            if (marker < 0xFF01) {
                std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
                std::runtime_error::runtime_error(exception, "RawCameraException");
                __cxa_throw(exception, MEMORY[0x1E7FD8A98], MEMORY[0x1E7FD8920]);
            }

            uint16_t *segment_ptr = current_ptr;
            uint32_t segment_length = __rev16(current_ptr[1]);
            if (segment_length >= 0x102) {
                std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
                std::runtime_error::runtime_error(exception, "RawCameraException");
                __cxa_throw(exception, MEMORY[0x1E7FD8A98], MEMORY[0x1E7FD8920]);
            }

            uint8_t *huffman_data = (uint8_t*)(current_ptr + 2);
            current_ptr = (uint16_t*)((char*)current_ptr + segment_length + 2);

            if (marker == 0xFFC3) { // SOF3 - Start of Frame (Lossless)
                decode_tables[0] = vdupq_n_s32(1 << (*(uint8_t*)(segment_ptr + 4) - 1));
                component_count = *(uint8_t*)(segment_ptr + 9);
                if ((unsigned int)(component_count - 5) <= 0xFFFFFFFB) {
                    std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
                    std::runtime_error::runtime_error(exception, "RawCameraException");
                    __cxa_throw(exception, MEMORY[0x1E7FD8A98], MEMORY[0x1E7FD8920]);
                }
            } else if (marker == 0xFFC4) { // DHT - Define Huffman Table
                while (huffman_data < (uint8_t*)current_ptr) {
                    uint64_t table_id = *huffman_data;
                    if (table_id > 3) break;

                    huffman_tables[table_id] = &decode_tables[128 * table_id + 1];
                    huffman_data = (uint8_t*)create_huffman_decoder_tables(huffman_data + 1);

                    if (!huffman_data) {
                        std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
                        std::runtime_error::runtime_error(exception, "RawCameraException");
                        __cxa_throw(exception, MEMORY[0x1E7FD8A98], MEMORY[0x1E7FD8920]);
                    }
                }
            }
        } while (marker != 0xFFDA); // SOS - Start of Scan

        // Verify all required huffman tables are present
        if (component_count >= 1) {
            void **table_ptr = huffman_tables;
            int32_t comp_check = component_count;
            while (*table_ptr) {
                table_ptr++;
                if (!--comp_check) {
                    goto decode_start;
                }
            }
            std::runtime_error *exception = (std::runtime_error*)__cxa_allocate_exception(0x10);
            std::runtime_error::runtime_error(exception, "RawCameraException");
            __cxa_throw(exception, MEMORY[0x1E7FD8A98], MEMORY[0x1E7FD8920]);
        }

decode_start:
        // Initialize decoding state
        int64_t prev_dc[4] = {0};
        int64_t singleton = initialize_singleton(ctx);
        uint64_t total_samples = samples_per_block * (uint64_t)component_count;

        // Get decode mode
        uint32_t decode_mode = (*(uint32_t(**)(uint64_t))(**(uint64_t**)ctx + 224))(ctx);

        if (decode_mode == 1 || compression_type == 2) {
            // Lossless JPEG decoding path
            uint64_t row = 0;
            void *temp_buffer = NULL;
            int bit_count = 0;
            int bit_buffer = 32;

            while (1) {
                uint64_t remaining_rows = block_height - row;
                uint64_t buffer_check = (char*)block_end - (char*)current_ptr + ((39 - bit_buffer) >> 3);

                if (!temp_buffer && buffer_check > 2 * (uint64_t)(component_count * samples_per_block * remaining_rows))
                    break;

                if (!temp_buffer && buffer_check < total_samples) {
                    if (block_end == current_ptr) {
                        temp_buffer = NULL;
                    } else {
                        size_t temp_size = total_samples * remaining_rows;
                        void *new_buffer = operator new[](temp_size, 0x1000C8077774924);
                        bzero(new_buffer, temp_size);
                        memmove(new_buffer, current_ptr, (char*)block_end - (char*)current_ptr);
                        temp_buffer = new_buffer;
                        current_ptr = (uint16_t*)new_buffer;
                        ctx = (decoder_context*)MEMORY[0x1EF4780A8]();
                    }
                }

                // Decode samples for this row - Huffman decoding implementation
                uint8_t *output_row = (uint8_t*)calculate_offset_difference(ctx) +
                    2 * total_rows * (block_idx / blocks_per_row) +
                    2 * (block_idx % blocks_per_row) * block_width +
                    2 * ((width * row) << (compression_type != 0));

                if (component_count >= 1) {
                    for (int comp = 0; comp < component_count; comp++) {
                        // Ensure we have enough bits in the buffer
                        if (bit_buffer >= 16) {
                            int byte_val = *(uint8_t*)current_ptr;
                            uint64_t offset = (byte_val == 255) ? 3 : 2;
                            uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                            int next_byte = *((uint8_t*)current_ptr + next_offset);
                            current_ptr = (uint16_t*)((char*)current_ptr + offset);

                            // Handle JPEG byte stuffing (0xFF followed by 0x00)
                            if (next_byte == 255)
                                current_ptr = (uint16_t*)((char*)current_ptr + 1);

                            bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                            bit_buffer -= 16;
                        }

                        // Huffman decode using pre-built tables
                        void *table_ptr = huffman_tables[comp];
                        uint32_t shifted_bits = bit_count << bit_buffer;
                        uint32_t table_lookup = *(uint16_t*)(table_ptr + 2 * (shifted_bits >> 23));
                        bit_buffer += (table_lookup >> 10) & 0x1F;

                        if (table_lookup > 0x8000) {
                            // Direct symbol encoding
                            table_lookup = (int8_t)table_lookup;
                        } else {
                            // Extended table lookup required
                            if (table_lookup & 0x8000) {
                                table_lookup = *(uint16_t*)(table_ptr + 2 * ((shifted_bits >> 16) & 0x3FF));
                                bit_buffer += (table_lookup >> 10) & 0x1F;
                            }

                            // Extract additional bits if needed
                            int extra_bits = table_lookup & 0x1F;
                            if (extra_bits != 0) {
                                if (bit_buffer >= 16) {
                                    int byte_val = *(uint8_t*)current_ptr;
                                    uint64_t offset = (byte_val == 255) ? 3 : 2;
                                    uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                    int next_byte = *((uint8_t*)current_ptr + next_offset);
                                    current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                    if (next_byte == 255)
                                        current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                    bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                    bit_buffer -= 16;
                                }

                                // Extract the additional bits and apply two's complement if needed
                                table_lookup = ((uint32_t)(bit_count << bit_buffer) >> -extra_bits) -
                                             ((((uint32_t)(bit_count << bit_buffer) >> 31) - 1) >> -extra_bits);
                                bit_buffer += extra_bits;
                            } else {
                                table_lookup = (table_lookup << 8) & 0x8000;
                            }
                        }

                        // Update DC predictor and store result
                        uint32_t decoded_value = prev_dc[comp] + table_lookup;
                        prev_dc[comp] = decoded_value;

                        // Write output for even-indexed components only
                        if ((comp & 1) == 0) {
                            *(uint16_t*)output_row = *(uint16_t*)(singleton + 2 * (uint16_t)decoded_value);
                            output_row += neighbor_offsets.data[comp];
                        }
                    }
                }

                // Process remaining samples in the row
                if (component_count == 2 && compression_type == 0) {
                    // Optimized path for 2-component, uncompressed mode
                    if (double_samples >= 2) {
                        uint32_t sample_idx = 2;
                        int comp0_val = prev_dc[0];
                        int comp1_val = prev_dc[1];

                        do {
                            // Decode component 0
                            if (bit_buffer >= 16) {
                                int byte_val = *(uint8_t*)current_ptr;
                                uint64_t offset = (byte_val == 255) ? 3 : 2;
                                uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                int next_byte = *((uint8_t*)current_ptr + next_offset);
                                current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                if (next_byte == 255)
                                    current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                bit_buffer -= 16;
                            }

                            uint32_t table_lookup0 = *(uint16_t*)(huffman_tables[0] + 2 * ((uint32_t)(bit_count << bit_buffer) >> 23));
                            int bit_buffer_temp = ((table_lookup0 >> 10) & 0x1F) + bit_buffer;

                            if (table_lookup0 > 0x8000) {
                                table_lookup0 = (int8_t)table_lookup0;
                            } else {
                                if (table_lookup0 & 0x8000) {
                                    table_lookup0 = *(uint16_t*)(huffman_tables[0] + 2 * (((uint32_t)(bit_count << bit_buffer) >> 16) & 0x3FF));
                                    bit_buffer_temp += (table_lookup0 >> 10) & 0x1F;
                                }

                                int extra_bits0 = table_lookup0 & 0x1F;
                                if (extra_bits0 != 0) {
                                    int bit_buffer_adj = bit_buffer_temp - 16;
                                    if (bit_buffer_temp >= 16) {
                                        int byte_val = *(uint8_t*)current_ptr;
                                        uint64_t offset = (byte_val == 255) ? 3 : 2;
                                        uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                        int next_byte = *((uint8_t*)current_ptr + next_offset);
                                        current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                        if (next_byte == 255)
                                            current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                        bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                        bit_buffer_temp = bit_buffer_adj;
                                    }

                                    table_lookup0 = ((uint32_t)(bit_count << bit_buffer_temp) >> -extra_bits0) -
                                                   ((((uint32_t)(bit_count << bit_buffer_temp) >> 31) - 1) >> -extra_bits0);
                                    bit_buffer_temp += extra_bits0;
                                } else {
                                    table_lookup0 = (table_lookup0 << 8) & 0x8000;
                                }
                            }

                            comp0_val += table_lookup0;
                            prev_dc[0] = comp0_val;
                            *(uint16_t*)output_row = *(uint16_t*)(singleton + 2 * (uint16_t)comp0_val);

                            // Decode component 1
                            int bit_buffer_adj2 = bit_buffer_temp - 16;
                            if (bit_buffer_temp >= 16) {
                                int byte_val = *(uint8_t*)current_ptr;
                                uint64_t offset = (byte_val == 255) ? 3 : 2;
                                uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                int next_byte = *((uint8_t*)current_ptr + next_offset);
                                current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                if (next_byte == 255)
                                    current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                bit_buffer_temp = bit_buffer_adj2;
                            }

                            uint32_t table_lookup1 = *(uint16_t*)(huffman_tables[1] + 2 * ((uint32_t)(bit_count << bit_buffer_temp) >> 23));
                            bit_buffer = ((table_lookup1 >> 10) & 0x1F) + bit_buffer_temp;

                            if (table_lookup1 > 0x8000) {
                                table_lookup1 = (int8_t)table_lookup1;
                            } else {
                                if (table_lookup1 & 0x8000) {
                                    table_lookup1 = *(uint16_t*)(huffman_tables[1] + 2 * (((uint32_t)(bit_count << bit_buffer_temp) >> 16) & 0x3FF));
                                    bit_buffer += (table_lookup1 >> 10) & 0x1F;
                                }

                                int extra_bits1 = table_lookup1 & 0x1F;
                                if (extra_bits1 != 0) {
                                    if (bit_buffer >= 16) {
                                        int byte_val = *(uint8_t*)current_ptr;
                                        uint64_t offset = (byte_val == 255) ? 3 : 2;
                                        uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                        int next_byte = *((uint8_t*)current_ptr + next_offset);
                                        current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                        if (next_byte == 255)
                                            current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                        bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                        bit_buffer -= 16;
                                    }

                                    table_lookup1 = ((uint32_t)(bit_count << bit_buffer) >> -extra_bits1) -
                                                   ((((uint32_t)(bit_count << bit_buffer) >> 31) - 1) >> -extra_bits1);
                                    bit_buffer += extra_bits1;
                                } else {
                                    table_lookup1 = (table_lookup1 << 8) & 0x8000;
                                }
                            }

                            comp1_val += table_lookup1;
                            uint16_t final_val = *(uint16_t*)(singleton + 2 * (uint16_t)comp1_val);
                            prev_dc[1] = comp1_val;
                            output_row[1] = final_val;
                            output_row += 2;
                            sample_idx += 2;
                        } while (sample_idx < double_samples);
                    }
                } else {
                    // General case for multi-component or compressed modes
                    int total_components = component_count;
                    for (uint32_t sample = component_count; sample < double_samples; sample += component_count) {
                        if (total_components >= 1) {
                            for (int comp = 0; comp < total_components; comp++) {
                                if (bit_buffer >= 16) {
                                    int byte_val = *(uint8_t*)current_ptr;
                                    uint64_t offset = (byte_val == 255) ? 3 : 2;
                                    uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                    int next_byte = *((uint8_t*)current_ptr + next_offset);
                                    current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                    if (next_byte == 255)
                                        current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                    bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                    bit_buffer -= 16;
                                }

                                uint32_t shifted_bits = bit_count << bit_buffer;
                                uint32_t table_lookup = *(uint16_t*)(huffman_tables[comp] + 2 * (shifted_bits >> 23));
                                bit_buffer += (table_lookup >> 10) & 0x1F;

                                if (table_lookup > 0x8000) {
                                    table_lookup = (int8_t)table_lookup;
                                } else {
                                    if (table_lookup & 0x8000) {
                                        table_lookup = *(uint16_t*)(huffman_tables[comp] + 2 * ((shifted_bits >> 16) & 0x3FF));
                                        bit_buffer += (table_lookup >> 10) & 0x1F;
                                    }

                                    int extra_bits = table_lookup & 0x1F;
                                    if (extra_bits != 0) {
                                        if (bit_buffer >= 16) {
                                            int byte_val = *(uint8_t*)current_ptr;
                                            uint64_t offset = (byte_val == 255) ? 3 : 2;
                                            uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                            int next_byte = *((uint8_t*)current_ptr + next_offset);
                                            current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                            if (next_byte == 255)
                                                current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                            bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                            bit_buffer -= 16;
                                        }

                                        table_lookup = ((uint32_t)(bit_count << bit_buffer) >> -extra_bits) -
                                                     ((((uint32_t)(bit_count << bit_buffer) >> 31) - 1) >> -extra_bits);
                                        bit_buffer += extra_bits;
                                    } else {
                                        table_lookup = (table_lookup << 8) & 0x8000;
                                    }
                                }

                                uint32_t decoded_value = prev_dc[comp] + table_lookup;
                                prev_dc[comp] = decoded_value;

                                if ((comp & 1) == 0) {
                                    *(uint16_t*)output_row = *(uint16_t*)(singleton + 2 * (uint16_t)decoded_value);
                                    output_row += neighbor_offsets.data[comp];
                                }
                            }
                        }
                    }
                }

                row++;
                if (row == rows_to_process) {
                    if (temp_buffer)
                        operator delete[](temp_buffer, 0x1000C8077774924);
                    break;
                }
            }
        } else if (decode_mode == 2) {
            // Alternative decoding path with different component processing
            uint64_t row = 0;
            void *temp_buffer = NULL;
            int bit_count = 0;
            int bit_buffer = 32;

            while (1) {
                uint64_t remaining_rows = block_height - row;
                uint64_t buffer_check = (char*)block_end - (char*)current_ptr + ((39 - bit_buffer) >> 3);

                if (!temp_buffer && buffer_check > 2 * (uint64_t)(component_count * samples_per_block * remaining_rows))
                    break;

                if (!temp_buffer && buffer_check < total_samples) {
                    if (block_end == current_ptr) {
                        temp_buffer = NULL;
                    } else {
                        size_t temp_size = total_samples * remaining_rows;
                        void *new_buffer = operator new[](temp_size, 0x1000C8077774924);
                        bzero(new_buffer, temp_size);
                        memmove(new_buffer, current_ptr, (char*)block_end - (char*)current_ptr);
                        temp_buffer = new_buffer;
                        current_ptr = (uint16_t*)new_buffer;
                        ctx = (decoder_context*)MEMORY[0x1EF4780A8]();
                    }
                }

                // Decode samples for this row - mode 2 specific logic
                uint8_t *output_row = (uint8_t*)calculate_offset_difference(ctx) +
                    2 * total_rows * (block_idx / blocks_per_row) +
                    2 * (block_idx % blocks_per_row) * block_width +
                    2 * ((width * row) << (compression_type != 0));

                if (component_count >= 1) {
                    for (int comp = 0; comp < component_count; comp++) {
                        if (bit_buffer >= 16) {
                            int byte_val = *(uint8_t*)current_ptr;
                            uint64_t offset = (byte_val == 255) ? 3 : 2;
                            uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                            int next_byte = *((uint8_t*)current_ptr + next_offset);
                            current_ptr = (uint16_t*)((char*)current_ptr + offset);

                            if (next_byte == 255)
                                current_ptr = (uint16_t*)((char*)current_ptr + 1);

                            bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                            bit_buffer -= 16;
                        }

                        void *table_ptr = huffman_tables[comp];
                        uint32_t shifted_bits = bit_count << bit_buffer;
                        uint32_t table_lookup = *(uint16_t*)(table_ptr + 2 * (shifted_bits >> 23));
                        bit_buffer += (table_lookup >> 10) & 0x1F;

                        if (table_lookup > 0x8000) {
                            table_lookup = (int8_t)table_lookup;
                        } else {
                            if (table_lookup & 0x8000) {
                                table_lookup = *(uint16_t*)(table_ptr + 2 * ((shifted_bits >> 16) & 0x3FF));
                                bit_buffer += (table_lookup >> 10) & 0x1F;
                            }

                            int extra_bits = table_lookup & 0x1F;
                            if (extra_bits != 0) {
                                if (bit_buffer >= 16) {
                                    int byte_val = *(uint8_t*)current_ptr;
                                    uint64_t offset = (byte_val == 255) ? 3 : 2;
                                    uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                    int next_byte = *((uint8_t*)current_ptr + next_offset);
                                    current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                    if (next_byte == 255)
                                        current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                    bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                    bit_buffer -= 16;
                                }

                                table_lookup = ((uint32_t)(bit_count << bit_buffer) >> -extra_bits) -
                                             ((((uint32_t)(bit_count << bit_buffer) >> 31) - 1) >> -extra_bits);
                                bit_buffer += extra_bits;
                            } else {
                                table_lookup = (table_lookup << 8) & 0x8000;
                            }
                        }

                        uint32_t decoded_value = prev_dc[comp] + table_lookup;
                        prev_dc[comp] = decoded_value;

                        if ((comp & 1) == 0) {
                            *(uint16_t*)output_row = *(uint16_t*)(singleton + 2 * (uint16_t)decoded_value);
                            output_row += neighbor_offsets.data[comp];
                        }
                    }
                }

                // Process additional samples in row for mode 2
                if (component_count == 2 && compression_type == 0) {
                    if (double_samples >= 2) {
                        uint32_t sample_idx = 2;
                        int comp0_val = prev_dc[0];
                        int comp1_val = prev_dc[1];

                        do {
                            // First component
                            if (bit_buffer >= 16) {
                                int byte_val = *(uint8_t*)current_ptr;
                                uint64_t offset = (byte_val == 255) ? 3 : 2;
                                uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                int next_byte = *((uint8_t*)current_ptr + next_offset);
                                current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                if (next_byte == 255)
                                    current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                bit_buffer -= 16;
                            }

                            uint32_t table_lookup0 = *(uint16_t*)(huffman_tables[0] + 2 * ((uint32_t)(bit_count << bit_buffer) >> 23));
                            int bit_buffer_temp = ((table_lookup0 >> 10) & 0x1F) + bit_buffer;

                            if (table_lookup0 > 0x8000) {
                                table_lookup0 = (int8_t)table_lookup0;
                            } else {
                                if (table_lookup0 & 0x8000) {
                                    table_lookup0 = *(uint16_t*)(huffman_tables[0] + 2 * (((uint32_t)(bit_count << bit_buffer) >> 16) & 0x3FF));
                                    bit_buffer_temp += (table_lookup0 >> 10) & 0x1F;
                                }

                                int extra_bits0 = table_lookup0 & 0x1F;
                                if (extra_bits0 != 0) {
                                    int bit_buffer_adj = bit_buffer_temp - 16;
                                    if (bit_buffer_temp >= 16) {
                                        int byte_val = *(uint8_t*)current_ptr;
                                        uint64_t offset = (byte_val == 255) ? 3 : 2;
                                        uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                        int next_byte = *((uint8_t*)current_ptr + next_offset);
                                        current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                        if (next_byte == 255)
                                            current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                        bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                        bit_buffer_temp = bit_buffer_adj;
                                    }

                                    table_lookup0 = ((uint32_t)(bit_count << bit_buffer_temp) >> -extra_bits0) -
                                                   ((((uint32_t)(bit_count << bit_buffer_temp) >> 31) - 1) >> -extra_bits0);
                                    bit_buffer_temp += extra_bits0;
                                } else {
                                    table_lookup0 = (table_lookup0 << 8) & 0x8000;
                                }
                            }

                            comp0_val += table_lookup0;
                            prev_dc[0] = comp0_val;
                            *(uint16_t*)output_row = *(uint16_t*)(singleton + 2 * (uint16_t)comp0_val);

                            // Second component with similar decoding logic
                            int bit_buffer_adj2 = bit_buffer_temp - 16;
                            if (bit_buffer_temp >= 16) {
                                int byte_val = *(uint8_t*)current_ptr;
                                uint64_t offset = (byte_val == 255) ? 3 : 2;
                                uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                int next_byte = *((uint8_t*)current_ptr + next_offset);
                                current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                if (next_byte == 255)
                                    current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                bit_buffer_temp = bit_buffer_adj2;
                            }

                            uint32_t table_lookup1 = *(uint16_t*)(huffman_tables[1] + 2 * ((uint32_t)(bit_count << bit_buffer_temp) >> 23));
                            bit_buffer = ((table_lookup1 >> 10) & 0x1F) + bit_buffer_temp;

                            if (table_lookup1 > 0x8000) {
                                table_lookup1 = (int8_t)table_lookup1;
                            } else {
                                if (table_lookup1 & 0x8000) {
                                    table_lookup1 = *(uint16_t*)(huffman_tables[1] + 2 * (((uint32_t)(bit_count << bit_buffer_temp) >> 16) & 0x3FF));
                                    bit_buffer += (table_lookup1 >> 10) & 0x1F;
                                }

                                int extra_bits1 = table_lookup1 & 0x1F;
                                if (extra_bits1 != 0) {
                                    if (bit_buffer >= 16) {
                                        int byte_val = *(uint8_t*)current_ptr;
                                        uint64_t offset = (byte_val == 255) ? 3 : 2;
                                        uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                        int next_byte = *((uint8_t*)current_ptr + next_offset);
                                        current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                        if (next_byte == 255)
                                            current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                        bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                        bit_buffer -= 16;
                                    }

                                    table_lookup1 = ((uint32_t)(bit_count << bit_buffer) >> -extra_bits1) -
                                                   ((((uint32_t)(bit_count << bit_buffer) >> 31) - 1) >> -extra_bits1);
                                    bit_buffer += extra_bits1;
                                } else {
                                    table_lookup1 = (table_lookup1 << 8) & 0x8000;
                                }
                            }

                            comp1_val += table_lookup1;
                            uint16_t final_val = *(uint16_t*)(singleton + 2 * (uint16_t)comp1_val);
                            prev_dc[1] = comp1_val;
                            output_row[1] = final_val;
                            output_row += 2;
                            sample_idx += 2;
                        } while (sample_idx < double_samples);
                    }
                } else {
                    // General case for other component counts and compression types
                    int total_components = component_count;
                    for (uint32_t sample = component_count; sample < double_samples; sample += component_count) {
                        if (total_components >= 1) {
                            for (int comp = 0; comp < total_components; comp++) {
                                if (bit_buffer >= 16) {
                                    int byte_val = *(uint8_t*)current_ptr;
                                    uint64_t offset = (byte_val == 255) ? 3 : 2;
                                    uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                    int next_byte = *((uint8_t*)current_ptr + next_offset);
                                    current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                    if (next_byte == 255)
                                        current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                    bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                    bit_buffer -= 16;
                                }

                                uint32_t shifted_bits = bit_count << bit_buffer;
                                uint32_t table_lookup = *(uint16_t*)(huffman_tables[comp] + 2 * (shifted_bits >> 23));
                                bit_buffer += (table_lookup >> 10) & 0x1F;

                                if (table_lookup > 0x8000) {
                                    table_lookup = (int8_t)table_lookup;
                                } else {
                                    if (table_lookup & 0x8000) {
                                        table_lookup = *(uint16_t*)(huffman_tables[comp] + 2 * ((shifted_bits >> 16) & 0x3FF));
                                        bit_buffer += (table_lookup >> 10) & 0x1F;
                                    }

                                    int extra_bits = table_lookup & 0x1F;
                                    if (extra_bits != 0) {
                                        if (bit_buffer >= 16) {
                                            int byte_val = *(uint8_t*)current_ptr;
                                            uint64_t offset = (byte_val == 255) ? 3 : 2;
                                            uint64_t next_offset = (byte_val == 255) ? 2 : 1;
                                            int next_byte = *((uint8_t*)current_ptr + next_offset);
                                            current_ptr = (uint16_t*)((char*)current_ptr + offset);

                                            if (next_byte == 255)
                                                current_ptr = (uint16_t*)((char*)current_ptr + 1);

                                            bit_count = (bit_count << 16) | (byte_val << 8) | next_byte;
                                            bit_buffer -= 16;
                                        }

                                        table_lookup = ((uint32_t)(bit_count << bit_buffer) >> -extra_bits) -
                                                     ((((uint32_t)(bit_count << bit_buffer) >> 31) - 1) >> -extra_bits);
                                        bit_buffer += extra_bits;
                                    } else {
                                        table_lookup = (table_lookup << 8) & 0x8000;
                                    }
                                }

                                uint32_t decoded_value = prev_dc[comp] + table_lookup;
                                prev_dc[comp] = decoded_value;

                                if ((comp & 1) == 0) {
                                    *(uint16_t*)output_row = *(uint16_t*)(singleton + 2 * (uint16_t)decoded_value);
                                    output_row += neighbor_offsets.data[comp];
                                }
                            }
                        }
                    }
                }

                row++;
                if (row == rows_to_process) {
                    if (temp_buffer)
                        operator delete[](temp_buffer, 0x1000C8077774924);
                    break;
                }
            }
        }
    }

    operator delete[](decode_buffer, 0x1000C8077774924);

    if (*(uint64_t*)0x1E7FDC060 != stack_canary) {
        __stack_chk_fail();
    }
    return 0;
}

void setup_huffman_decoder(huffman_decoder_context *context) { // 0x1b280b164
    void* log_handle = sub_1B27E53F4();
    log_handle = _objc_retainAutoreleasedReturnValue(log_handle);
    
    if ((context + 1) >= 2) {
        if (_os_signpost_enabled(log_handle)) {
            uint16_t signpost_buf = 0;
            __os_signpost_emit_with_name_impl("dso", log_handle, 1, context, "CLosslessJpegUnpacker", "format", &signpost_buf, 2);
        }
    }
    
    _objc_release(log_handle);
    
    // Set up stack canary
    uint64_t canary = *(uint64_t*)0x1E7FDC060;
    
    // Create block structure for Huffman decoding
    void* block_ptr = &huffman_decode_block_func;
    void* block_isa = (void*)0x1E90B62D0;
    
    _objc_retainBlock(&block_ptr);
    
    // Clear various data structures
    memset(&local_structures, 0, sizeof(local_structures));
    
    uint32_t width = context->width;
    uint32_t height = context->height;
    
    uint64_t offset_diff = calculate_offset_difference(context);
    uint32_t current_offset = context->current_offset;
    
    // Call virtual method to get some value
    uint64_t vtable_ptr = context->vtable;
    uint32_t method_result = ((uint32_t(*)(huffman_decoder_context*))vtable_ptr->method_0x98)(context);
    
    uint32_t calculated_offset = context->current_offset + method_result;
    if (calculated_offset > current_offset) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Process component information
    void* component_start = (void*)(context + 0xC0);
    void* component_end = (void*)(context + 0xC8);
    
    if (component_start != component_end) {
        uint32_t component_index = 0;
        jpeg_component_info* comp = (jpeg_component_info*)((uint8_t*)component_start + 4);
        
        while (component_index <= 2 && comp < component_end) {
            uint32_t comp_id = comp->component_id;
            // Store component info
            component_index++;
            comp++;
        }
    }
    
    // Set up component sampling parameters
    uint32_t comp1_h = component_info[0].h_sampling;
    uint32_t comp1_v = component_info[0].v_sampling;
    
    if (comp1_h >= 1 && comp1_v >= 1) {
        // Calculate component dimensions and offsets
        uint32_t adjusted_width1 = width - comp1_h;
        uint32_t offset1 = offset_diff + comp1_h * 2;
        
        uint32_t comp2_h = component_info[1].h_sampling;
        if (comp2_h >= 1) {
            uint32_t adjusted_width2 = width - comp2_h;
            uint32_t offset2 = offset1 + comp2_h * 2;
            
            uint32_t comp3_h = component_info[2].h_sampling;
            if (comp3_h >= 1) {
                uint32_t adjusted_width3 = width - comp3_h;
                // Store component 3 info
            }
        }
    } else {
        // Use default parameters
    }
    
    // Read data from input stream
    void* input_stream = context->input_stream;
    ((void(*)(void*, uint64_t, uint32_t))input_stream->vtable->read_method)(input_stream, offset_diff, 0x1000);
    
    // Check for JPEG SOI marker (0xFFD8)
    uint16_t marker = *(uint16_t*)offset_diff;
    marker = __builtin_bswap16(marker);
    if (marker != 0xFFD8) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Parse JPEG segments
    uint8_t* current_pos = (uint8_t*)offset_diff + 2;
    
    while (true) {
        uint16_t segment_marker = *(uint16_t*)current_pos;
        segment_marker = __builtin_bswap16(segment_marker);
        current_pos += 2;
        
        if (segment_marker == 0xFFFF) {
            continue;
        }
        
        if (segment_marker == 0xFFDA) {
            break; // Start of Scan
        }
        
        if (segment_marker >= 0xFF01) {
            uint16_t segment_length = *(uint16_t*)current_pos;
            segment_length = __builtin_bswap16(segment_length) - 2;
            
            if (segment_length > 0x100) {
                throw std::runtime_error("RawCameraException");
            }
            
            uint8_t* segment_data = current_pos + 2;
            uint8_t* segment_end = segment_data + segment_length;
            
            if (segment_marker == 0xFFC4) { // Define Huffman Table
                while (segment_data < segment_end) {
                    uint8_t table_info = *segment_data;
                    if (table_info > 3) break;
                    
                    void* table_ptr = (void*)0x22200 + (table_info << 11);
                    huffman_tables[table_info] = table_ptr;
                    segment_data++;
                    
                    create_huffman_decoder_tables(segment_data, table_ptr, 2);
                }
            } else if (segment_marker == 0xFFC3) { // Start of Frame
                uint8_t precision = segment_data[0];
                uint16_t img_height = __builtin_bswap16(*(uint16_t*)(segment_data + 1));
                uint16_t img_width = __builtin_bswap16(*(uint16_t*)(segment_data + 3));
                uint8_t num_components = segment_data[5];
                
                // Store frame parameters
                frame_precision = precision;
                frame_height = img_height;
                frame_width = img_width;
                frame_components = num_components;
            }
            
            current_pos = segment_end;
        }
    }
    
    // Validate frame parameters
    if (frame_components != 2 && frame_components != 4) {
        throw std::runtime_error("RawCameraException");
    }
    
    if (frame_height < 1 || frame_width < 1) {
        throw std::runtime_error("RawCameraException");
    }
    
    uint64_t total_pixels = (uint64_t)frame_height * frame_width;
    uint64_t max_pixels = (uint64_t)height * width;
    if (total_pixels > max_pixels) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Set up decoder parameters and buffers
    uint64_t decoder_size = (total_pixels * 10000 + 0x2800) / 10000;
    
    huffman_decode_block* decode_block = (huffman_decode_block*)allocate_memory_with_type(decoder_size);
    if (!decode_block) {
        throw std::runtime_error("RawCameraException");
    }
    
    // Initialize decode block structure
    memset(decode_block, 0, sizeof(huffman_decode_block));
    decode_block->memory_block = decode_block;
    decode_block->total_size = decoder_size;
    
    // Set up various decoder parameters
    decode_block->image_width = width;
    decode_block->num_threads = num_threads;
    decode_block->block_width = block_width;
    decode_block->block_height = block_height;
    decode_block->total_blocks = total_blocks;
    
    // Set up Huffman tables and synchronization primitives
    decode_block->sync_counter1 = _dispatch_semaphore_create(0);
    decode_block->bit_buffer = _dispatch_semaphore_create(0);
    
    // Set up dispatch group and queue for parallel processing
    void* decode_group = _dispatch_group_create();
    void* decode_queue = _dispatch_queue_create("Huffman Decode Block", DISPATCH_QUEUE_CONCURRENT);
    
    // Submit decode tasks
    if (num_threads > 1) {
        for (int i = 0; i < num_threads; i++) {
            _dispatch_group_async_f(decode_group, decode_queue, &decode_block->huffman_table_ptrs[i], decode_function);
        }
    }
    
    // Wait for all decode tasks to complete
    _dispatch_group_wait(decode_group, DISPATCH_TIME_FOREVER);
    
    // Clean up
    _objc_release(decode_queue);
    _objc_release(decode_group);
    
    if (decode_block->processed_count < 1) {
        _objc_release(block_ptr);
        throw std::runtime_error("RawCameraException");
    }
    
    // Clean up resources
    if (decode_block->memory_block) {
        _objc_release(decode_block->sync_counter1);
        _objc_release(decode_block->bit_buffer);
        free(decode_block->memory_block);
    }
    
    _objc_release(block_ptr);
    
    // Verify stack canary
    if (*(uint64_t*)0x1E7FDC060 != canary) {
        __stack_chk_fail();
    }
}
