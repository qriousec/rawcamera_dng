
// Function at address 0x1b280740c
uint64_t calculate_offset_difference(camera_object* camera_obj) {
    PACIBSP
    SUB             SP, SP, #0x40
    STP             X22, X21, [SP,#0x30+var_20]
    STP             X20, X19, [SP,#0x30+var_10]
    STP             X29, X30, [SP,#0x30+var_s0]
    ADD             X29, SP, #0x30
    MOV             X19, X0
    LDR             X16, [X0]
    MOV             X17, X0
    MOVK            X17, #0xA354,LSL#48
    AUTDA           X16, X17
    LDR             X9, [X16,#0x70]!
    MOV             X10, X16
    MOV             X8, SP
    MOV             X17, X10
    MOVK            X17, #0x88C9,LSL#48
    BLRAA           X9, X17
    
    void* raw_camera_ptr = *(void**)(SP + 0x30 - 0x30);
    validate_raw_camera_pointer(raw_camera_ptr);
    
    uint32_t value1 = get_value_with_extraction((object_with_vtable*)raw_camera_ptr);
    uint64_t offset1 = *(uint64_t*)(raw_camera_ptr + 24);
    
    uint32_t value2 = get_value_with_extraction((object_with_vtable*)camera_obj);
    
    shared_weak_count* weak_count = *(shared_weak_count**)(SP + 0x30 - 0x28);
    if (weak_count != NULL) {
        shared_ptr_release_weak(weak_count);
    }
    
    uint64_t result = (value1 + offset1) - value2;
    
    LDP             X29, X30, [SP,#0x30+var_s0]
    LDP             X20, X19, [SP,#0x30+var_10]
    LDP             X22, X21, [SP,#0x30+var_20]
    ADD             SP, SP, #0x40
    RETAB
    
    return result;
}

int get_camera_parameter(raw_camera *camera) { // 0x1b2778e7c
    short parameter = 0;
    
    validate_raw_camera_parameter(camera, &parameter, 2);
    
    int byte_order = camera->byte_order;
    short value = parameter;
    int reversed = __builtin_bswap32(value) >> 16;
    
    int result;
    if (byte_order == 2) {
        result = reversed;
    } else {
        result = value;
    }
    
    return result & 0xFFFF;
}
