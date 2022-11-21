#include "pch.h"
#include "gnf.h"
#include "converter.h"
#include <DirectXTex/DirectXTex.h>
#include <DirectXTex/DirectXTexP.h>
#include <DirectXTex/DDS.h>
#include <dxgiformat.h>
#include "MathFunctions.h"

size_t ConvertGnfToDDS(const byte* gnfsrc, const size_t& gnfsize, byte*& ddsout)
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
		meta.format = gnfimg.header.formatType == Gnf::FormatType::FormatTypeSRGB ? DXGI_FORMAT_BC1_UNORM_SRGB : DXGI_FORMAT_BC1_UNORM;
		break;
	case Gnf::Format::FormatBC2:
		meta.format = gnfimg.header.formatType == Gnf::FormatType::FormatTypeSRGB ? DXGI_FORMAT_BC2_UNORM_SRGB : DXGI_FORMAT_BC2_UNORM;
		break;
	case Gnf::Format::FormatBC3:
		meta.format = gnfimg.header.formatType == Gnf::FormatType::FormatTypeSRGB ? DXGI_FORMAT_BC3_UNORM_SRGB : DXGI_FORMAT_BC3_UNORM;
		break;
	case Gnf::Format::FormatBC4:
		meta.format = gnfimg.header.formatType == Gnf::FormatType::FormatTypeSNorm ? DXGI_FORMAT_BC4_SNORM : DXGI_FORMAT_BC4_UNORM;
		break;
	case Gnf::Format::FormatBC5:
		meta.format = gnfimg.header.formatType == Gnf::FormatType::FormatTypeSNorm ? DXGI_FORMAT_BC5_SNORM : DXGI_FORMAT_BC5_UNORM;
		break;
	case Gnf::Format::FormatBC6:
		meta.format = gnfimg.header.formatType == Gnf::FormatType::FormatTypeSNorm ? DXGI_FORMAT_BC6H_SF16 : DXGI_FORMAT_BC6H_UF16;
		break;
	case Gnf::Format::FormatBC7:
		meta.format = gnfimg.header.formatType == Gnf::FormatType::FormatTypeSRGB ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
		break;
	case Gnf::Format::Format8:
		if (gnfimg.header.formatType == Gnf::FormatType::FormatTypeUNorm)
			meta.format = DXGI_FORMAT_R8_UNORM;
		else if (gnfimg.header.formatType == Gnf::FormatType::FormatTypeSNorm)
			meta.format = DXGI_FORMAT_R8_SNORM;
		else if (gnfimg.header.formatType == Gnf::FormatType::FormatTypeUInt)
			meta.format = DXGI_FORMAT_R8_UINT;
		else if (gnfimg.header.formatType == Gnf::FormatType::FormatTypeSInt)
			meta.format = DXGI_FORMAT_R8_SINT;
		break;
	default:
		throw std::exception("Format not implemented!");
		break;
	}

	// sub to change
	size_t bpp = 8;
	size_t pixbl = 4;
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
	case Gnf::Format::Format8:
		bpp = 8;
		pixbl = 1;
		break;
	default:
		throw std::exception("Format not implemented!");
		break;
	}

	byte* header = new byte[148];
	size_t required;
	DirectX::DDS_FLAGS flag = DirectX::DDS_FLAGS::DDS_FLAGS_NONE | DirectX::DDS_FLAGS::DDS_FLAGS_ALLOW_LARGE_FILES;
	switch (gnfimg.header.format)
	{
	case Gnf::Format::FormatBC1:
	case Gnf::Format::FormatBC2:
	case Gnf::Format::FormatBC3:
	case Gnf::Format::FormatBC4:
	case Gnf::Format::FormatBC5:
	case Gnf::Format::Format8:
		flag |= (meta.width > 4096) || (meta.height > 4096) ? DirectX::DDS_FLAGS::DDS_FLAGS_FORCE_DX10_EXT : DirectX::DDS_FLAGS::DDS_FLAGS_FORCE_DX9_LEGACY;
		break;
	case Gnf::Format::FormatBC6:
	case Gnf::Format::FormatBC7:
		flag |= DirectX::DDS_FLAGS::DDS_FLAGS_FORCE_DX10_EXT;
		break;
	default:
		throw std::exception("Format not implemented!");
		break;
	}
	DirectX::_EncodeDDSHeader(meta, flag, header, 148, required);

	size_t datasize = 0;
	for (uint32_t i = 0; i < meta.mipLevels; i++)
	{
		size_t w = meta.width;
		size_t h = meta.height;
		w >>= i;
		h >>= i;

		if (w < 1 && h < 1)
			throw std::exception("Invalid Mip count");

		w = std::max<size_t>(w, pixbl);
		h = std::max<size_t>(h, pixbl);

		w = (w + (pixbl - 1)) & (~(pixbl - 1));
		h = (h + (pixbl - 1)) & (~(pixbl - 1));

		size_t size = w * h * bpp / 8;

		datasize += size;
	}

	ddsout = new byte[datasize + required];
	memcpy(ddsout, header, required);

	size_t ddsoff = required;
	size_t gnfoff = 0;
	for (uint32_t i = 0; i < meta.mipLevels; i++)
	{
		size_t w = meta.width;
		size_t h = meta.height;
		w >>= i;
		h >>= i;

		if (w < 1 && h < 1)
			throw std::exception("Invalid Mip count");

		w = std::max<size_t>(w, pixbl);
		h = std::max<size_t>(h, pixbl);

		w = (w + (pixbl - 1)) & (~(pixbl - 1));
		h = (h + (pixbl - 1)) & (~(pixbl - 1));

		size_t tempw = BitHacks::RoundUpTo2(w);
		size_t temph = BitHacks::RoundUpTo2(h);
		//if (i == 0 && tempw != (gnfimg.header.pitch + 1))
		//	throw std::exception("Pitch doesn't match RoundUp2 Width");

		tempw = pixbl == 1 ? std::max<size_t>(tempw, 16) : std::max<size_t>(tempw, 32);
		temph = pixbl == 1 ? std::max<size_t>(temph, 16) : std::max<size_t>(temph, 32);

		size_t size = tempw * temph * bpp / 8;

		byte* tempData = new byte[size];
		Gnf::GnfImage::UnSwizzle(gnfimg.imageData.get() + gnfoff, tempData, tempw, temph, bpp,pixbl);
		gnfoff += size;

		size_t size1 = w * h * bpp / 8;
		size_t scanLineSize = w * pixbl * bpp / 8;
		size_t scanLineSizePadded = tempw * pixbl * bpp / 8;
		size_t off1 = 0;
		size_t off2 = 0;
		for (uint32_t j = 0; j < (h / pixbl); j++)
		{
			memcpy(ddsout + ddsoff + off1, tempData + off2, scanLineSize);
			off1 += scanLineSize;
			off2 += scanLineSizePadded;
		}
		ddsoff += size1;
		delete[] tempData;
	}

	return (datasize + required);
}
size_t ConvertDDSToGnf(const byte* ddssrc,const size_t &ddssize, byte*& gnfout)
{
	DirectX::TexMetadata meta;
	DirectX::DDS_FLAGS flags = DirectX::DDS_FLAGS::DDS_FLAGS_ALLOW_LARGE_FILES;
	std::unique_ptr<DirectX::ScratchImage> image(new (std::nothrow) DirectX::ScratchImage);
	DirectX::LoadFromDDSMemory(ddssrc, ddssize, flags, &meta, *image);

	Gnf::GnfImage gnfImg;

	switch (meta.format)
	{
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		gnfImg.header.format = Gnf::Format::FormatBC1;
		gnfImg.header.formatType = meta.format == DXGI_FORMAT_BC1_UNORM_SRGB ? Gnf::FormatType::FormatTypeSRGB : Gnf::FormatType::FormatTypeUNorm;
		break;
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		gnfImg.header.format = Gnf::Format::FormatBC2;
		gnfImg.header.formatType = meta.format == DXGI_FORMAT_BC2_UNORM_SRGB ? Gnf::FormatType::FormatTypeSRGB : Gnf::FormatType::FormatTypeUNorm;
		break;
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		gnfImg.header.format = Gnf::Format::FormatBC3;
		gnfImg.header.formatType = meta.format == DXGI_FORMAT_BC3_UNORM_SRGB ? Gnf::FormatType::FormatTypeSRGB : Gnf::FormatType::FormatTypeUNorm;
		break;
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		gnfImg.header.format = Gnf::Format::FormatBC4;
		gnfImg.header.formatType = meta.format == DXGI_FORMAT_BC4_SNORM ? Gnf::FormatType::FormatTypeSNorm : Gnf::FormatType::FormatTypeUNorm;
		break;
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
		gnfImg.header.format = Gnf::Format::FormatBC5;
		gnfImg.header.formatType = meta.format == DXGI_FORMAT_BC5_SNORM ? Gnf::FormatType::FormatTypeSNorm : Gnf::FormatType::FormatTypeUNorm;
		break;
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
		gnfImg.header.format = Gnf::Format::FormatBC6;
		gnfImg.header.formatType = meta.format == DXGI_FORMAT_BC6H_SF16 ? Gnf::FormatType::FormatTypeSNorm : Gnf::FormatType::FormatTypeUNorm;
		break;
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		gnfImg.header.format = Gnf::Format::FormatBC7;
		gnfImg.header.formatType = meta.format == DXGI_FORMAT_BC7_UNORM_SRGB ? Gnf::FormatType::FormatTypeSRGB : Gnf::FormatType::FormatTypeUNorm;
		break;
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SINT:
		gnfImg.header.format = Gnf::Format::Format8;
		if(meta.format == DXGI_FORMAT_R8_UNORM)
			gnfImg.header.formatType = Gnf::FormatType::FormatTypeUNorm;
		else if (meta.format == DXGI_FORMAT_R8_SNORM)
			gnfImg.header.formatType = Gnf::FormatType::FormatTypeSNorm;
		else if (meta.format == DXGI_FORMAT_R8_UINT)
			gnfImg.header.formatType = Gnf::FormatType::FormatTypeUInt;
		else if (meta.format == DXGI_FORMAT_R8_SINT)
			gnfImg.header.formatType = Gnf::FormatType::FormatTypeSInt;
	break;
	default:
		throw std::exception("Format not implemented!");
		break;
	}
	
	if (meta.width < 1 || meta.height < 1 || meta.mipLevels < 1)
		throw std::exception("Invalid Resolution and/or Mip");

	gnfImg.header.width = meta.width - 1;
	gnfImg.header.height = meta.height - 1;
	gnfImg.header.mipmaps = meta.mipLevels - 1;
	switch (gnfImg.header.format)
	{
	case Gnf::Format::FormatBC1:
	case Gnf::Format::FormatBC2:
	case Gnf::Format::FormatBC3:
	case Gnf::Format::FormatBC7:
		gnfImg.header.destX = 4;
		gnfImg.header.destY = 5;
		gnfImg.header.destZ = 6;
		gnfImg.header.destW = 7;
		break;
	case Gnf::Format::FormatBC4:
		gnfImg.header.destX = 4;
		gnfImg.header.destY = 0;
		gnfImg.header.destZ = 0;
		gnfImg.header.destW = 1;
		break;
	case Gnf::Format::FormatBC5:
		gnfImg.header.destX = 4;
		gnfImg.header.destY = 5;
		gnfImg.header.destZ = 0;
		gnfImg.header.destW = 1;
		break;
	case Gnf::Format::FormatBC6:
		gnfImg.header.destX = 4;
		gnfImg.header.destY = 5;
		gnfImg.header.destZ = 6;
		gnfImg.header.destW = 1;
		gnfImg.header.unk7 = 0xB6D;
		gnfImg.header.unk9 = 0xA000;
		break;
	case Gnf::Format::Format8:
		gnfImg.header.destX = 4;
		gnfImg.header.destY = 0;
		gnfImg.header.destZ = 0;
		gnfImg.header.destW = 1;
		break;
	default:
		throw std::exception("Format not implemented!");
		break;
	}

	uint16_t bpp = 8;
	uint16_t pixbl = 4;
	switch (gnfImg.header.format)
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
	case Gnf::Format::Format8:
		bpp = 8;
		pixbl = 1;
		break;
	default:
		throw std::exception("Format not implemented!");
		break;
	}
	uint32_t tempPitch = BitHacks::RoundUpTo2(meta.width);
	tempPitch = pixbl == 1 ? std::max<size_t>(tempPitch, 16) : std::max<size_t>(tempPitch, 32);
	gnfImg.header.pitch = tempPitch - 1;

	gnfImg.header.dataSize = 0;
	for (uint32_t i = 0; i < meta.mipLevels; i++)
	{
		auto mipImg = (*image).GetImages()[i];
		size_t w = mipImg.width;
		size_t h = mipImg.height;

		if (w < 1 && h < 1)
			throw std::exception("Invalid Mip count");

		w = std::max<size_t>(w, pixbl);
		h = std::max<size_t>(h, pixbl);

		w = (w + (pixbl - 1)) & (~(pixbl - 1));
		h = (h + (pixbl - 1)) & (~(pixbl - 1));

		w = BitHacks::RoundUpTo2(w);
		h = BitHacks::RoundUpTo2(h);
		if (i == 0 && w != (gnfImg.header.pitch + 1))
			throw std::exception("Pitch doesn't match RoundUp2 Width");

		w = pixbl == 1 ? std::max<size_t>(w, 16) : std::max<size_t>(w, 32);
		h = pixbl == 1 ? std::max<size_t>(h, 16) : std::max<size_t>(h, 32);

		size_t size = w * h * bpp / 8;
		gnfImg.header.dataSize += uint32_t(size);
	}

	gnfImg.imageData = std::make_shared<byte[]>(gnfImg.header.dataSize);
	for (size_t k = 0; k < gnfImg.header.dataSize; k++)
	{
		gnfImg.imageData.get()[k] = byte(0);
	}

	size_t gnfoff = 0;
	for (size_t i = 0; i < meta.mipLevels; i++)
	{
		auto mipImg = (*image).GetImages()[i];
		size_t w = mipImg.width;
		size_t h = mipImg.height;

		if (w < 1 && h < 1)
			throw std::exception("Invalid Mip Count/Resolution");

		w = std::max<size_t>(w, pixbl);
		h = std::max<size_t>(h, pixbl);

		w = (w + (pixbl - 1)) & (~(pixbl - 1));
		h = (h + (pixbl - 1)) & (~(pixbl - 1));

		size_t tempw = BitHacks::RoundUpTo2(w);
		size_t temph = BitHacks::RoundUpTo2(h);

		if (i == 0 && tempw != (gnfImg.header.pitch + 1))
			throw std::exception("Pitch doesn't match RoundUp2 Width");

		tempw = pixbl == 1 ? std::max<size_t>(tempw, 16) : std::max<size_t>(tempw, 32);
		temph = pixbl == 1 ? std::max<size_t>(temph, 16) : std::max<size_t>(temph, 32);

		size_t size = tempw * temph * bpp / 8;
		byte* tempData = new byte[size];
		for (size_t k = 0; k < size; k++)
		{
			tempData[k] = byte(0);
		}
		size_t scanLineSize = w * pixbl * bpp / 8;
		size_t scanLineSizePadded = tempw * pixbl * bpp / 8;
		size_t off1 = 0;
		size_t off2 = 0;
		for (uint32_t j = 0; j < (h / pixbl); j++)
		{
			memcpy(tempData + off2, mipImg.pixels + off1, scanLineSize);
			off1 += scanLineSize;
			off2 += scanLineSizePadded;
		}

		Gnf::GnfImage::Swizzle(tempData, gnfImg.imageData.get() + gnfoff, tempw, temph, bpp, pixbl);
		gnfoff += size;
		delete[] tempData;
	}

	gnfImg.header.fileSize = gnfImg.header.dataSize + 0x100;
	gnfImg.WriteImage(gnfout);

	return gnfImg.header.fileSize;
}