
void initialize_rawcamera_log() { // 0x1b2960cec
    if (___cxa_guard_acquire(&unk_1ECB9FEC0)) {
        uint64_t log_object = _os_log_create("com.apple.rawcamera", "unpack");
        store_and_advance_pointer(log_object, &qword_1ECB9FEB8);
        ___cxa_guard_release(&unk_1ECB9FEC0);
    }
}
