#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb/stb_image.h"

struct Image
{
    uint8_t* data;
    int size;
    int width, height, channels;
    uint8_t* get_pixel_offset(int i, int j) 
    {
        return data + (j + height * i) * channels;
    }
};

void get_data_from_file(Image* img, const char* file_path) 
{
    int width, height, channels;
    int ok = stbi_info(file_path, &width, &height, &channels);
    if (!ok) 
    {
        exit(-1);
    }
    
    img->data     = stbi_load(file_path, &width, &height, &channels, 0);
    img->size     = width * height * channels;
    img->width    = width; 
    img->height   = height;
    img->channels = channels;
}

void write_ppm_file(Image* img, const char* file_path)
{
    std::ofstream f(file_path);
    f << "P3" << std::endl;
    f << img->width << " " << img->height << std::endl;
    f << "255" << std::endl;
    for (int i = 0; i < img->width; i++)
    {
        for (int j = 0; j < img->height; j++)
        {
            uint8_t* o = img->get_pixel_offset(i, j);
            uint8_t r = o[0];
            uint8_t g = o[1];
            uint8_t b = o[2];
            f << (int) r << " " << (int) g << " "  << (int) b << std::endl;
        }
    }

    f.close();
}

void grayscale(Image* img)
{
    for (int i = 0; i <= img->width; i++)
    {
        for (int j = 0; j <= img->height; j++)
        {
            uint8_t* o = img->get_pixel_offset(i, j);
            //                         r      g      b
            uint32_t c = (uint32_t) (o[0] + o[1] + o[2]) / 3;
            o[0] = c; o[1] = c; o[2] = c;
        }
    }
}

int main()
{
    Image img;
    get_data_from_file(&img, "parrot.jpg");
    
    grayscale(&img);
    write_ppm_file(&img, "parrot.ppm");

    stbi_image_free(img.data);
    return 0;
}
