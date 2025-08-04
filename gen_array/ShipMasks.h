#include <stdlib.h> 
#pragma once
struct ship_mask_info
{
    int x;
    int y;
    int length;
    bool horizontal;
    unsigned int check_mask[3];
    unsigned int apply_mask[3];
};

constexpr size_t masks_length_2 = 161;
constexpr size_t masks_length_3 = 142;
constexpr size_t masks_length_4 = 123;

extern ship_mask_info masks_2[];
extern ship_mask_info masks_3[];
extern ship_mask_info masks_4[];



bool masks_intersect(unsigned int* mask1, unsigned int* mask2);
void apply_mask(unsigned int* source_mask, unsigned int* target);
