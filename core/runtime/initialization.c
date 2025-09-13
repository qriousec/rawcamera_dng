
void initialize_once(void) { // 0x1b2963328
    _dispatch_once(&qword_1ECB7F630, &unk_1F38233B8);
}

void* initialize_singleton(singleton_object *arg1) { // initialize_singleton (0x1b2865524)
    dispatch_once_block block;
    
    block.padding1 = *(double*)0x1B296A770;
    block.function_ptr = (void (*)(void*))sub_1B28655B4;
    block.context = (void*)0x1E90B62D0;
    block.captured_object = arg1;
    
    if (arg1->once_token != -1) {
        dispatch_once(&arg1->once_token, &block);
    }
    
    return arg1->cached_value;
}
