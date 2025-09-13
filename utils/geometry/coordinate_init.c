
coordinates* initialize_coordinates(coordinates* coord_ptr, int value1, int value2) { // 0x1b2777074
    int temp_result1;
    int temp_result2;
    
    check_and_copy_value(&temp_result1, &value2);
    coord_ptr->x = temp_result1;
    
    check_and_copy_value(&temp_result1, &value1);
    coord_ptr->y = temp_result1;
    
    return coord_ptr;
}
