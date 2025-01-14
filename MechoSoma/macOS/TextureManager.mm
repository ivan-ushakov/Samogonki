#include "TextureManager.h"

#import <Accelerate/Accelerate.h>

#include <algorithm>
#include <array>
#include <cassert>

using namespace graphics;

struct TTextureFormat
{
	DWORD dwFormatID;
	BOOL bSupported;

	DWORD dwRGBAlphaBitMask;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;

	// This is used for matching 
	DWORD dwRGBBitCount;
	BOOL bPalette8;
	DWORD dwRBitCount;
	DWORD dwGBitCount;
	DWORD dwBBitCount;
	DWORD dwAlphaBitCount;

	// Calculated shifts for RGBA 
	DWORD dwRBitShift;
	DWORD dwGBitShift;
	DWORD dwBBitShift;
	DWORD dwAlphaBitShift;
};

static std::array<TTextureFormat, 6> texture_formats
{
	TTextureFormat
	{ 
		D3DTEXFMT_RGB565,
		TRUE,
		0,
		0b0000000000011111,
		0b0000011111100000,
		0b1111100000000000,
		16,
		FALSE,
		5,
		6,
		5,
		0,
		0,
		0,
		0,
		0
	},
	TTextureFormat
	{
		D3DTEXFMT_RGB555,
		FALSE,
		0,
		0,
		0,
		0,
		16,
		FALSE,
		5,
		5,
		5,
		0,
		0,
		0,
		0,
		0
	},
	TTextureFormat
	{
		D3DTEXFMT_ARGB4444,
		TRUE,
		0b1111000000000000,
		0b0000000000001111,
		0b0000000011110000,
		0b0000111100000000,
		16,
		FALSE,
		4,
		4,
		4,
		4,
		0,
		0,
		0,
		0
	},
	TTextureFormat
	{
		D3DTEXFMT_ARGB1555,
		TRUE,
		0b1000000000000000,
		0b0000000000011111,
		0b0000001111100000,
		0b0111110000000000,
		16,
		FALSE,
		5,
		5,
		5,
		1,
		0,
		0,
		0,
		0
	},
	TTextureFormat
	{
		D3DTEXFMT_RGBA8888,
		TRUE,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		32,
		FALSE,
		8,
		8,
		8,
		8,
		0,
		0,
		0,
		0
	},
	TTextureFormat
	{
		D3DTEXFMT_PAL8,
		TRUE,
		0,
		0,
		0,
		0,
		8,
		TRUE,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	}
};

constexpr DWORD GetMaskBitShift(DWORD dwBits)
{
	if (0 == dwBits)
	{
		return 0;
	}

	DWORD dwShift = 0;
	for (; (dwBits & 1) == 0; dwBits >>= 1)
	{
		dwShift++;
	}

	return dwShift;
}

constexpr DWORD GetMaskBitCount(DWORD dwBits)
{
	DWORD dwBitCount = 0;
	for (; dwBits; dwBits >>= 1)
	{
		dwBitCount += (dwBits & 0x1);
	}

	return dwBitCount;
}

TextureManager::TextureManager(id<MTLDevice> device) : _device(device)
{
	for (auto& format : texture_formats)
	{
		format.dwRBitShift = GetMaskBitShift(format.dwRBitMask);
		format.dwGBitShift = GetMaskBitShift(format.dwGBitMask);
		format.dwBBitShift = GetMaskBitShift(format.dwBBitMask);
		format.dwAlphaBitShift = GetMaskBitShift(format.dwRGBAlphaBitMask);
	}
}

id<MTLTexture> TextureManager::get(DWORD dwHandle)
{
	if (dwHandle >= _textures.size())
	{
		return nil;
	}
	return _textures[dwHandle]->texture.get();
}

MD3DERROR TextureManager::d3dGetTextureFormatData(DWORD dwTexFormatID, M3DTEXTUREFORMAT* pData)
{
	for (const auto& format : texture_formats)
	{
		if (format.dwFormatID == dwTexFormatID && format.bSupported)
		{
			pData->dwTotalBitCount = format.dwRGBBitCount;
			pData->bPalette8 = format.bPalette8;

			pData->dwAlphaBitCount = format.dwAlphaBitCount;
			pData->dwRBitCount = format.dwRBitCount;
			pData->dwGBitCount = format.dwGBitCount;
			pData->dwBBitCount = format.dwBBitCount;

			pData->dwAlphaBitMask = format.dwRGBAlphaBitMask;
			pData->dwRBitMask = format.dwRBitMask;
			pData->dwGBitMask = format.dwGBitMask;
			pData->dwBBitMask = format.dwBBitMask;

			pData->dwAlphaBitShift = format.dwAlphaBitShift;
			pData->dwRBitShift = format.dwRBitShift;
			pData->dwGBitShift = format.dwGBitShift;
			pData->dwBBitShift = format.dwBBitShift;

			return MD3D_OK;
		}
	}
	return MD3DERR_ILLEGALCALL;
}

MD3DERROR TextureManager::d3dCreateTexture(DWORD dwWidth, DWORD dwHeight, DWORD dwTexFormatID, DWORD* lpdwHandle)
{
	assert(dwWidth == dwHeight);
	assert(GetMaskBitCount(dwWidth) == 1);
	assert(GetMaskBitCount(dwHeight) == 1);
	assert(dwHeight <= 256);
	assert(dwWidth <= 256);

	auto p = std::find_if(texture_formats.cbegin(), texture_formats.cend(), [&](const auto& format) {
		return format.dwFormatID == dwTexFormatID && format.bSupported;
	});
	if (p == texture_formats.cend())
	{
		return MD3DERR_ILLEGALCALL;
	}

	auto pixel_format = p->bPalette8 ? MTLPixelFormatA8Unorm : MTLPixelFormatBGRA8Unorm;
	auto* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixel_format
																		  width:dwWidth
																		 height:dwHeight
																	  mipmapped:NO];
	descriptor.storageMode = MTLStorageModeShared;
	objc::Ref<id<MTLTexture>> texture([_device newTextureWithDescriptor:descriptor]);

	*lpdwHandle = _textures.size();

	const DWORD pitch = dwHeight * (p->dwRGBBitCount / 8);
	_textures.push_back(std::make_unique<TextureEntry>(TextureEntry{
		dwTexFormatID,
		texture,
		std::vector<char>(pitch * dwHeight),
		pitch,
		false
	}));

	return MD3D_OK;
}

MD3DERROR TextureManager::d3dDeleteTexture(DWORD dwHandle)
{
	if (dwHandle >= _textures.size())
	{
		return MD3DERR_ILLEGALCALL;
	}

	return MD3D_OK;
}

MD3DERROR TextureManager::d3dLockTexture(DWORD dwHandle, VOID** lplpTexture, DWORD* lpPitch)
{
	if (dwHandle >= _textures.size())
	{
		return MD3DERR_ILLEGALCALL;
	}

	auto entry = _textures[dwHandle].get();
	if (entry->is_locked)
	{
		return MD3DERR_ILLEGALCALL;
	}
	entry->is_locked = true;

	*lplpTexture = reinterpret_cast<VOID*>(entry->lock_buffer.data());
	*lpPitch = entry->pitch;

	return MD3D_OK;
}

MD3DERROR TextureManager::d3dLockTexture(
	DWORD dwHandle,
	DWORD dwLeft,
	DWORD dwTop,
	DWORD dwRight,
	DWORD dwBottom,
	VOID** lplpTexture,
	DWORD* lpPitch
)
{
	return d3dLockTexture(dwHandle, lplpTexture, lpPitch);
}

MD3DERROR TextureManager::d3dUnlockTexture(DWORD dwHandle)
{
	if (dwHandle >= _textures.size())
	{
		return MD3DERR_ILLEGALCALL;
	}

	auto entry = _textures[dwHandle].get();
	if (!entry->is_locked)
	{
		return MD3DERR_ILLEGALCALL;
	}

	update_texture(*entry);
	entry->is_locked = false;

	return MD3D_OK;
}

void TextureManager::update_texture(TextureEntry& entry)
{
	auto t = entry.texture.get();
	const auto image_height = t.height;
	const auto image_width = t.width;

	switch (entry.original_format_id)
	{
		case D3DTEXFMT_RGB565:
		{
			vImage_Buffer source;
			source.data = entry.lock_buffer.data();
			source.height = image_height;
			source.width = image_width;
			source.rowBytes = entry.pitch;

			vImage_Buffer target;
			_argb_buffer.resize(4 * image_width * image_height);
			target.data = _argb_buffer.data();
			target.height = image_height;
			target.width = image_width;
			target.rowBytes = 4 * image_width;

			vImageConvert_RGB565toARGB8888(255, &source, &target, kvImageNoFlags);
			break;
		}

		case D3DTEXFMT_ARGB4444:
		{
			const auto input = reinterpret_cast<const uint16_t*>(entry.lock_buffer.data());

			_argb_buffer.resize(4 * image_width * image_height);
			auto output = reinterpret_cast<uint32_t*>(_argb_buffer.data());

			const uint16_t alpha_mask = 0b1111000000000000;
			const uint16_t red_mask = 0b0000000000001111;
			const uint16_t green_mask = 0b0000000011110000;
			const uint16_t blue_mask = 0b0000111100000000;

			const size_t count = entry.lock_buffer.size() / sizeof(uint16_t);
			std::array<uint8_t, 4> channels{ 0 };
			for (size_t i = 0; i < count; i++)
			{
				const auto color = input[i];

				channels[0] = (color & alpha_mask) >> 12;
				channels[1] = color & red_mask;
				channels[2] = (color & green_mask) >> 4;
				channels[3] = (color & blue_mask)  >> 8;

				uint32_t result = 0;
				int shift = 0;
				for (auto c8 : channels)
				{
					uint32_t c32 = (c8 * 255 + 7) / 15;
					result |= c32 << shift;
					shift += 8;
				}
				output[i] = result;
			}
			break;
		}

		case D3DTEXFMT_RGB555:
		case D3DTEXFMT_ARGB1555:
		{
			vImage_Buffer source;
			source.data = entry.lock_buffer.data();
			source.height = image_height;
			source.width = image_width;
			source.rowBytes = entry.pitch;

			vImage_Buffer target;
			_argb_buffer.resize(4 * image_width * image_height);
			target.data = _argb_buffer.data();
			target.height = image_height;
			target.width = image_width;
			target.rowBytes = 4 * image_width;

			vImageConvert_ARGB1555toARGB8888(&source, &target, kvImageNoFlags);
			break;
		}

		default:
			break;
	}

	_bgra_buffer.resize(4 * image_width * image_height);

	vImage_Buffer source;
	source.data = _argb_buffer.data();
	source.height = image_height;
	source.width = image_width;
	source.rowBytes = 4 * image_width;

	vImage_Buffer target;
	target.data = _bgra_buffer.data();
	target.height = image_height;
	target.width = image_width;
	target.rowBytes = 4 * image_width;

	// from ARGB (0, 1, 2, 3) to BGRA
	const uint8_t permute_map[4] = {3, 2, 1, 0};
	vImagePermuteChannels_ARGB8888(&source, &target, permute_map, kvImageNoFlags);

	MTLRegion region = MTLRegionMake2D(0, 0, image_width, image_height);
	const void* pixel_bytes = _bgra_buffer.data();
	[t replaceRegion:region mipmapLevel:0 withBytes:pixel_bytes bytesPerRow:4 * image_width];
}
