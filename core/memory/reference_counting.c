
// shared_ptr_release_weak (0x1b277716c)
void shared_ptr_release_weak(shared_weak_count* this_ptr) {
    long* ref_count_ptr = (long*)((char*)this_ptr + 8);
    long decrement_value = -1;
    long previous_count = __atomic_fetch_add(ref_count_ptr, decrement_value, __ATOMIC_ACQ_REL);
    
    if (previous_count != 1) {
        return;
    }
    
    // Authentication and virtual function call logic
    void* vtable = this_ptr->vtable_ptr;
    // Authenticate vtable pointer with discriminator 0x634F
    void** vtable_authenticated = (void**)vtable;
    void (*release_func)(shared_weak_count*) = (void(*)(shared_weak_count*))vtable_authenticated[2];
    // Call virtual function with discriminator 0x4444
    release_func(this_ptr);
    
    // Jump to standard library implementation
    __ZNSt3__119__shared_weak_count14__release_weakEv();
}

// shared_ptr_destructor (0x1b29627ac)
void shared_ptr_destructor(shared_ptr* this_ptr) {
    shared_weak_count* control_block = (shared_weak_count*)this_ptr->control_block;
    
    if (control_block == nullptr) {
        return;
    }
    
    // Atomic decrement of reference count
    long* ref_count_ptr = &control_block->ref_count;
    long old_count = __sync_fetch_and_add(ref_count_ptr, -1);
    
    if (old_count != 1) {
        return;
    }
    
    // Reference count reached zero, call destructor
    void** vtable = (void**)control_block->vtable;
    void (*destructor)(shared_weak_count*) = (void (*)(shared_weak_count*))vtable[2];
    destructor(control_block);
    
    // Release weak reference
    __ZNSt3__119__shared_weak_count14__release_weakEv(control_block);
}
