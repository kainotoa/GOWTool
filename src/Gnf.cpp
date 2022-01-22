#include "pch.h"
#include "gnf.h"

namespace Gnf
{
	void GnfImage::ReadImage(const byte* file)
	{
		memcpy(&header, file, sizeof(Header));
		imageData = std::make_shared<byte[]>(header.dataSize);
		memcpy(imageData.get(), file + sizeof(Header) + 0xC0, header.dataSize);

		return;
	}
	void GnfImage::UnSwizzle(const byte* src, byte* dst, const uint16_t& w, const uint16_t& h,const uint16_t &bpp)
	{
		uint32_t num2 = w * h * bpp / 8;
		if (num2 < 1)
			return;

		uint32_t num4 = 4; // sub to change
		uint32_t num5 = bpp * 2;
		byte* array1 = new byte[num2 * 2];
		byte* array2 = new byte[16];
		int num6 = h / num4;
		int num7 = w / num4;
		int roff = 0;
		for (int i = 0; i < (num6 + 7) / 8; i++)
		{
			for (int j = 0; j < (num7 + 7) / 8; j++)
			{
				for (int k = 0; k < 64; k++)
				{
					int num8 = morton(k, 8, 8);
					int num9 = num8 / 8;
					int num10 = num8 % 8;
					memcpy(array2, src + roff, num5);
					roff += num5;
					if (j * 8 + num10 < num7 && i * 8 + num9 < num6)
					{
						int destinationIndex = num5 * ((i * 8 + num9) * num7 + j * 8 + num10);
						memcpy(array1 + destinationIndex, array2, num5);
					}
				}
			}
		}

		memcpy(dst, array1, num2);

		delete[] array1;
		delete[] array2;
		return;
	}
	int GnfImage::morton(int t, int sx, int sy)
	{
		int num = 0;
		int num2 = 0;
		int num3;
		int num4 = (num3 = 1);
		int num5 = t;
		int num6 = sx;
		int num7 = sy;
		num = 0;
		num2 = 0;
		while (num6 > 1 || num7 > 1)
		{
			if (num6 > 1)
			{
				num += num4 * (num5 & 1);
				num5 >>= 1;
				num4 *= 2;
				num6 >>= 1;
			}
			if (num7 > 1)
			{
				num2 += num3 * (num5 & 1);
				num5 >>= 1;
				num3 *= 2;
				num7 >>= 1;
			}
		}
		return num2 * sx + num;
	}
}