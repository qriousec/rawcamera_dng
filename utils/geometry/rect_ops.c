
// rectangles_overlap (0x1b28b9500)
int rectangles_overlap(rectangle *rect1, rectangle *rect2) {
    int rect2_x = rect2->x;
    int rect1_x = rect1->x;
    
    if (rect2_x > rect1_x) {
        return 0;
    }
    
    int rect2_width = rect2->width;
    int rect2_right = rect2_width + rect2_x;
    
    if (rect1_x > rect2_right) {
        return 0;
    }
    
    int rect1_width = rect1->width;
    int rect1_right = rect1_width + rect1_x;
    
    if (rect2_x > rect1_right || rect1_right > rect2_right) {
        return 0;
    }
    
    int rect2_y = rect2->y;
    int rect1_y = rect1->y;
    
    if (rect2_y > rect1_y) {
        return 0;
    }
    
    int rect2_height = rect2->height;
    int rect2_bottom = rect2_height + rect2_y;
    
    if (rect1_y > rect2_bottom) {
        return 0;
    }
    
    int rect1_height = rect1->height;
    int rect1_bottom = rect1_height + rect1_y;
    
    return (rect2_y <= rect1_bottom && rect1_bottom <= rect2_bottom) ? 1 : 0;
}
