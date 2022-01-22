#include "pch.h"
#include "gnf.h"
#include "converter.h"
#include <DirectXTex/DirectXTex.h>
#include <DirectXTex/DDS.h>
#include <dxgiformat.h>

int ConvertGnfToDDS(const byte* gnfsrc, byte*& ddsout)
{
	Gnf::GnfImage gnfimg;
	gnfimg.ReadImage(gnfsrc);

	DirectX::TexMetadata meta;
	meta.width = gnfimg.header.width + 1;
	meta.height = gnfimg.header.height + 1;
	meta.depth = gnfimg.header.depth + 1;
	meta.arraySize = 1;
	meta.mipLevels = gnfimg.header.mipmaps + 1;
	meta.miscFlags = 0;
	meta.miscFlags2 = 3;
	meta.dimension = DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE2D;

	switch (gnfimg.header.format)
	{
	case Gnf::Format::FormatBC1:
		meta.format = DXGI_FORMAT_BC1_UNORM;
		if (gnfimg.header.formatType == Gnf::FormatType::FormatTypeSRGB)
			meta.format = DXGI_FORMAT_BC1_UNORM_SRGB;
		break;
	case Gnf::Format::FormatBC2:
		meta.format = DXGI_FORMAT_BC2_UNORM;
		if (gnfimg.header.formatType == Gnf::FormatType::FormatTypeSRGB)
			meta.format = DXGI_FORMAT_BC2_UNORM_SRGB;
		break;
	case Gnf::Format::FormatBC3:
		meta.format = DXGI_FORMAT_BC3_UNORM;
		if (gnfimg.header.formatType == Gnf::FormatType::FormatTypeSRGB)
			meta.format = DXGI_FORMAT_BC3_UNORM_SRGB;
		break;
	case Gnf::Format::FormatBC4:
		meta.format = DXGI_FORMAT_BC4_UNORM;
		if (gnfimg.header.formatType == Gnf::FormatType::FormatTypeSNorm)
			meta.format = DXGI_FORMAT_BC4_SNORM;
		break;
	case Gnf::Format::FormatBC5:
		meta.format = DXGI_FORMAT_BC5_UNORM;
		if (gnfimg.header.formatType == Gnf::FormatType::FormatTypeSNorm)
			meta.format = DXGI_FORMAT_BC5_SNORM;
		break;
	case Gnf::Format::FormatBC6:
		meta.format = DXGI_FORMAT_BC6H_UF16;
		break;
	case Gnf::Format::FormatBC7:
		meta.format = DXGI_FORMAT_BC7_UNORM;
		if (gnfimg.header.formatType == Gnf::FormatType::FormatTypeSRGB)
			meta.format = DXGI_FORMAT_BC7_UNORM_SRGB;
		break;
	default:
		throw std::exception("Format not implemented!");
		break;
	}

	// sub to change
	uint16_t bpp = 0;
	switch (gnfimg.header.format)
	{
	case Gnf::Format::FormatBC1:
	case Gnf::Format::FormatBC4:
		bpp = 4;
		break;
	case Gnf::Format::FormatBC2:
	case Gnf::Format::FormatBC3:
	case Gnf::Format::FormatBC5:
	case Gnf::Format::FormatBC6:
	case Gnf::Format::FormatBC7:
		bpp = 8;
		break;
	default:
		throw std::exception("Format not implemented!");
		break;
	}

	size_t size = meta.width * meta.height * bpp / 8;
	byte* put = new byte[size];
	Gnf::GnfImage::UnSwizzle(gnfimg.imageData.get(), put, meta.width, meta.height, bpp);

	return 0;
}