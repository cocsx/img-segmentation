#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb/stb_image.h"

struct __attribute__ ((packed)) Pixel {
	uint8_t r, g, b;
};

struct Image
{
	int m_width, m_height, m_comps;
	unsigned long int m_pitch;
	Pixel* m_pixels;

	void LoadImage(const char* filePath)
	{
		uint8_t* data = stbi_load(filePath,&m_width, &m_height, &m_comps, STBI_rgb);
		m_pitch = m_width * sizeof(Pixel);
		m_pixels = new Pixel[m_width * m_height];

		for (int x = 0; x < m_width; ++x)
		{
			for (int y = 0; y < m_height; ++y)
			{
				int offset = (x + m_width * y) * m_comps;
				m_pixels[x + m_width * y] = {
						.r = data[offset + 0],
						.g = data[offset + 1],
						.b = data[offset + 2],
				};
			}
		}
		stbi_image_free(data);
	}

	void MakeGray() const
	{
		for (int x = 0; x < m_width; x++)
		{
			for (int y = 0; y < m_height; y++)
			{
				Pixel p = m_pixels[x + m_width*y];
				uint8_t g = (p.r + p.g + p.b) / 3;
				m_pixels[x + m_width*y] = {g, g, g};
			}
		}
	}

	void ClearPixels() const
	{
		delete[] m_pixels;
	}
};