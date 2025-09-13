
// Function at 0x1b27ab35c
void multiply_with_overflow_check(uint32_t* operand1, uint32_t* operand2) {
    uint32_t val1 = *operand1;
    uint32_t val2 = *operand2;
    
    uint64_t result = (uint64_t)val1 * (uint64_t)val2;
    *operand1 = (uint32_t)result;
    
    if ((result & 0xFFFFFFFF00000000ULL) != 0) {
        exception_info* exception = (exception_info*)___cxa_allocate_exception(0x10);
        std::runtime_error::runtime_error((char*)exception + 8, "RawCameraException");
        throw_exception(exception);
    }
}
