#include "texdecode.h"
#include "froggeratbdump.h"
#include <string.h>

const uint8_t color_3_to_8[8] =
{
	0x00,0x24,0x49,0x6d, 0x92,0xb6,0xdb,0xff
};

const uint8_t color_4_to_8[16] =
{
	0x00,0x11,0x22,0x33, 0x44,0x55,0x66,0x77, 0x88,0x99,0xaa,0xbb, 0xcc,0xdd,0xee,0xff
};

const uint8_t color_5_to_8[32] =
{
	0x00,0x08,0x10,0x19, 0x21,0x29,0x31,0x3a, 0x42,0x4a,0x52,0x5a, 0x63,0x6b,0x73,0x7b,
	0x84,0x8c,0x94,0x9c, 0xa5,0xad,0xb5,0xbd, 0xc5,0xce,0xd6,0xde, 0xe6,0xef,0xf7,0xff
};

const uint8_t color_6_to_8[64] =
{
	0x00,0x04,0x08,0x0c, 0x10,0x14,0x18,0x1c, 0x20,0x24,0x28,0x2d, 0x31,0x35,0x39,0x3d,
	0x41,0x45,0x49,0x4d, 0x51,0x55,0x59,0x5d, 0x61,0x65,0x69,0x6d, 0x71,0x75,0x79,0x7d,
	0x82,0x86,0x8a,0x8e, 0x92,0x96,0x9a,0x9e, 0xa2,0xa6,0xaa,0xae, 0xb2,0xb6,0xba,0xbe,
	0xc2,0xc6,0xca,0xce, 0xd2,0xd7,0xdb,0xdf, 0xe3,0xe7,0xeb,0xef, 0xf3,0xf7,0xfb,0xff
};

static uint32_t ConvertRGBA8Data(uint8_t* data)
{
	return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

static uint32_t ConvertABGR32Data(uint8_t* data)
{
	return data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
}


static uint32_t *DecodeRGBA32(uint8_t *data, int16_t w, int16_t h)
{
	uint32_t* dst = new uint32_t[w * h];
	for (int16_t i = 0; i < h; i++) {
		for (int16_t j = 0; j < w; j++) {
			size_t pixel = (i * w) + j;
			dst[pixel] = ConvertRGBA8Data(&data[pixel*4]);
		}
	}
	return dst;
}

static uint32_t ConvertARGB32Data(uint8_t* data)
{
	return data[3] | (data[0] << 8) | (data[1] << 16) | (data[2] << 24);
}

static uint32_t ConvertRGBA16Data(uint8_t* data)
{
	uint16_t value = (data[1] << 8) | data[0];
	uint8_t r, g, b, a;
	if (value & 0x8000) {
		r = color_5_to_8[(value >> 10) & 0x1F];
		g = color_5_to_8[(value >> 5) & 0x1F];
		b = color_5_to_8[value & 0x1F];
		a = 255;
	}
	else {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	}
	return b | (g << 8) | (r << 16) | (a << 24);
}

static uint32_t* DecodeARGB32(uint8_t* data, int16_t w, int16_t h)
{
	uint32_t* dst = new uint32_t[w * h];
	for (int16_t i = 0; i < h; i++) {
		for (int16_t j = 0; j < w; j++) {
			size_t pixel = (i * w) + j;
			dst[pixel] = ConvertARGB32Data(&data[pixel * 4]);
		}
	}
	return dst;
}

static uint32_t* DecodeRGBA16(uint8_t* data, int16_t w, int16_t h)
{
	uint32_t* dst = new uint32_t[w * h];
	for (int16_t i = 0; i < h; i++) {
		for (int16_t j = 0; j < w; j++) {
			size_t pixel = (i * w) + j;
			dst[pixel] = ConvertRGBA16Data(&data[pixel * 2]);
		}
	}
	return dst;
}

static uint32_t ReadPalette(uint8_t* palData, uint8_t index, bool is32Bit)
{
	if (is32Bit) {
		return ConvertABGR32Data(&palData[index * 4]);
	}
	else {
		return ConvertRGBA16Data(&palData[index * 2]);
	}
}

static uint32_t *DecodeCI8(uint8_t *data, uint8_t *pal_data, int16_t w, int16_t h, bool is32Bit)
{
	uint32_t *dst = new uint32_t[w * h];
	for (int16_t i = 0; i < h; i++) {
		for (int16_t j = 0; j < w; j++) {
			size_t pixel = (i * w) + j;
			uint8_t palIdx = data[pixel];
			dst[pixel] = ReadPalette(pal_data, palIdx, is32Bit);
		}
	}
	return dst;
}

static uint32_t *DecodeCI4(uint8_t *data, uint8_t *pal_data, int16_t w, int16_t h, bool is32Bit)
{
	uint32_t* dst = new uint32_t[w * h];
	uint16_t srcPitch = (w+1) / 2;
	for (int16_t i = 0; i < h; i++) {
		for (int16_t j = 0; j < w; j++) {
			size_t pixel = (i * w) + j;
			
			size_t srcIdx = (i * srcPitch) + (j / 2);
			uint8_t palIdx;
			if (i < (h / 2)) {
				if (j % 2) {
					palIdx = (data[pixel / 2] & 0xF0) >> 4;
				}
				else {
					palIdx = data[pixel / 2] & 0xF;
				}
			}
			else {
				if (j % 2) {
					palIdx = data[pixel / 2] & 0xF;
				}
				else {
					palIdx = (data[pixel / 2] & 0xF0) >> 4;
				}
			}
			
			dst[pixel] = ReadPalette(pal_data, palIdx, is32Bit);
		}
	}
	return dst;
}

uint32_t *DecodeTexture(uint8_t *data, uint8_t *pal_data, int16_t w, int16_t h, uint8_t format, bool is32Bit)
{
	switch (format) {
		case TEX_FORMAT_RGBA32:
			return DecodeRGBA32(data, w, h);

		case TEX_FORMAT_ARGB32:
			return DecodeARGB32(data, w, h);

		case TEX_FORMAT_RGBA16:
			return DecodeRGBA16(data, w, h);

		case TEX_FORMAT_CI8:
			return DecodeCI8(data, pal_data, w, h, is32Bit);

		case TEX_FORMAT_CI4:
			return DecodeCI4(data, pal_data, w, h, is32Bit);

		default:
			PrintError("Texture format %d unimplemented.", format);
			return nullptr;
	}
}
