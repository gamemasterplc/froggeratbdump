#pragma once

#include <stdint.h>

#define TEX_FORMAT_RGBA32 0
#define TEX_FORMAT_ARGB32 1
#define TEX_FORMAT_RGBA16 2
#define TEX_FORMAT_CI8 3
#define TEX_FORMAT_CI4 4

uint32_t *DecodeTexture(uint8_t *data, uint8_t *pal_data, int16_t w, int16_t h, uint8_t format, bool is32Bit);