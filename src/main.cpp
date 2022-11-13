#pragma GCC optimize("O3","unroll-loops","omit-frame-pointer","inline") //Optimization flags
#pragma GCC option("arch=native","tune=native","no-zero-upper") //Enable AVX
#pragma GCC target("avx")

#include <x86intrin.h>

#include <random>
#include <iostream>

#include <vector>
#include <unordered_map>

#include <cstdint>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendor/stb/stb_image_write.h"

struct Image 
{
    int width, height, comps;
    int stride;
    uint32_t* pixels;
    
    void updateTextureDisplay(SDL_Renderer* renderer, SDL_Texture* texture)
    {
        SDL_UpdateTexture(texture, nullptr, (void*)pixels, stride);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
};

struct Point
{
    int x, y;
    Point(int x, int y)
    {
        this->x = x;
        this->y = y;
    }
    
    bool operator==(const Point &p) const
    {
        return x == p.x && y == p.y;
    }
};

namespace std {
  template <>
  struct hash<Point>
  {
    std::size_t operator()(const Point& p) const
    {
      using std::size_t;
      using std::hash;
      return (hash<int>()(p.x)^ (hash<int>()(p.y)));
    }
  };
}

Image* loadImage(const char* filePath)
{
    Image* img = new Image;
    uint8_t* data = stbi_load(filePath, &img->width, &img->height, &img->comps, STBI_rgb);
    img->stride = img->width * sizeof(uint32_t);
    img->pixels = new uint32_t[img->width * img->height];
    
    for (int x = 0; x < img->width; x++)
    {
        for (int y = 0; y < img->height; y++)
        {
            int offset = (x + img->width*y) * img->comps;
            img->pixels[x + img->width*y] = (data[offset] << 24) + (data[offset + 1] << 16) + (data[offset + 2] << 8) + 0xFF;
            //printf("[r: %x, g: %x, b: %x] => %x \n", data[offset], data[offset+1], data[offset+2], img->pixels[x * img->height + y]);
        } 
    }
    stbi_image_free(data);
    return img;
}

void makeGray(Image* img, SDL_Renderer* renderer, SDL_Texture* texture)
{
    for (int x = 0; x < img->width; x++)
    {
        for (int y = 0; y < img->height; y++)
        {
            uint32_t u32 = img->pixels[x + img->width*y];
            uint8_t g = (((u32 & 0xff000000) >> 24) + ((u32 & 0x00ff0000) >> 16) + ((u32 & 0x0000ff00) >> 8)) / 3;
            img->pixels[x + img->width*y] = (g << 24) + (g << 16) + (g << 8) + 0xFF;
        }
        img->updateTextureDisplay(renderer, texture);
    }   
}

void makeClusters(Image* img, int numOfClusters, int maxIter, SDL_Renderer* renderer, SDL_Texture* texture)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<int> w(0, img->width - 1);
    std::uniform_int_distribution<int> h(0, img->height - 1);
    
    int clusters[numOfClusters][2];
    for (int i = 0; i < numOfClusters; i++)
    {
        clusters[i][0] = w(rng);
        clusters[i][1] = h(rng);
    }
    
    std::unordered_map<Point, std::vector<Point>> map;
    for (int h = 0; h < maxIter; h++)
    {
        for (int x = 0; x < img->width; x++)
        {
            for (int y = 0; y < img->height; y++)
            {
                int i = -1, j = -1;
                int min_dist = INT_MAX;
                for (auto c : clusters)
                {
                    int dist = (c[0] - x)*(c[0] - x) + (c[1] - y)*(c[1] - y); 
                    if (dist >= min_dist) continue;
                    min_dist = dist;
                    i = c[0];
                    j = c[1];
                }
                map[{i,j}].push_back({x, y});
            }
        }
        //TODO: Calculate the CENTER OF GRAVITY to move clusters
        int k = 0;
        for (auto cp : map)
        {
            int mx = -1, my = -1, n = 0;
            for (auto p : cp.second)
            {
                mx += p.x;
                my += p.y;
                n++; 
            }
            clusters[k][0] = mx / n;
            clusters[k][1] = my / n;
            k++;
        }
        if (h < maxIter - 1) 
        {
            map.clear();
        }
    }
    
    for (auto cp : map)
    {
        for (auto p : cp.second)
        {
            img->pixels[p.x + img->width*p.y] = img->pixels[cp.first.x + img->width*cp.first.y];
        }
        img->updateTextureDisplay(renderer, texture);                
    }
}

int main()
{
    const char filePath[] = "dog.jpg";
    Image* img = loadImage(filePath);
    
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    SDL_Window* window = SDL_CreateWindow( "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, img->width, img->height, SDL_WINDOW_SHOWN );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_STREAMING, img->width, img->height);
    
    SDL_Event event;
    bool done = false;
    
    int numOfClusters = 3000;
    int numOfIterations = 20;
    while (SDL_PollEvent(&event) || !done)
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    done = true;
                if (event.key.keysym.sym == SDLK_g)
                    makeGray(img, renderer, texture);
                if (event.key.keysym.sym == SDLK_s)
                    makeClusters(img, numOfClusters, numOfIterations, renderer, texture);
            break;                
            case SDL_QUIT:
                done = true;
            break;
        }
        img->updateTextureDisplay(renderer, texture);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(window);
    
    delete[] img->pixels;
    delete img;
    
    return 0;
}
