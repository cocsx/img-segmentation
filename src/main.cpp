#if 1
	#pragma GCC optimize("O3","unroll-loops","omit-frame-pointer","inline") //Optimization flags
	#pragma GCC option("arch=native","tune=native","no-zero-upper") //Enable AVX
	#pragma GCC target("avx")
	#include <x86intrin.h>
#endif

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

#include "Image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendor/stb/stb_image_write.h"


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


static void UpdateImageTexture(Image& img, SDL_Renderer* renderer, SDL_Texture* texture)
{
	SDL_UpdateTexture(texture, nullptr, (void*)img.m_pixels, (int) img.m_pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}

int DistanceBetweenTwoPixelsColor(const Pixel& p1, const Pixel& p2)
{
    return (p1.r-p2.r)*(p1.r-p2.r) + (p1.g-p2.g)*(p1.g-p2.g) + (p1.b-p2.b)*(p1.b-p2.b);
}

using ClusterMap = std::unordered_map<Point, std::vector<Point>>;

void CalculateCentroids(const ClusterMap& map, int centroids[][2])
{
	int k = 0;
	for (const auto& cp : map)
	{
		int mx = -1, my = -1, n = 0;
		for (auto p : cp.second)
		{
			mx += p.x;
			my += p.y;
			n++;
		}
		centroids[k][0] = mx / n;
		centroids[k][1] = my / n;
		k++;
	}
}

void MakeClusters(ClusterMap& map, Image& img, int centroids[][2], int numOfClusters)
{
	for (int x = 0; x < img.m_width; x++)
	{
		for (int y = 0; y < img.m_height; y++)
		{
			Pixel p1 = img.m_pixels[x + img.m_width*y];
			int i = -1, j = -1;
			int min_dist = INT_MAX;
			for (int p = 0; p < numOfClusters; ++p)
			{
				auto c = centroids[p];
				Pixel p2 = img.m_pixels[c[0] + img.m_width*c[1]];
				int dist = DistanceBetweenTwoPixelsColor(p1, p2);

				if (dist >= min_dist) continue;
				min_dist = dist;
				i = c[0];
				j = c[1];
			}
			map[{i,j}].emplace_back(x, y);
		}
	}
}

void KMeans(Image& img, int numOfClusters, int maxIter)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<int> w(0, img.m_width - 1);
    std::uniform_int_distribution<int> h(0, img.m_height - 1);
    
    int centroids[numOfClusters][2];
    for (int i = 0; i < numOfClusters; i++)
    {
	    centroids[i][0] = w(rng);
	    centroids[i][1] = h(rng);
    }

	ClusterMap map;
	for (int k = 0; k < maxIter; k++)
	{
		MakeClusters(map, img, centroids, numOfClusters);
		CalculateCentroids(map, centroids);
        if (k < maxIter - 1)
        {
            map.clear();
        }
    }
    
    for (const auto& cp : map)
    {
        for (auto p : cp.second)
        {
            img.m_pixels[p.x + img.m_width*p.y] = img.m_pixels[cp.first.x + img.m_width*cp.first.y];
        }
    }
}

int main()
{
    const char filePath[] = "assets/panda.jpg";
    Image img{};
	img.LoadImage(filePath);

    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    SDL_Window* window = SDL_CreateWindow( "KMeans",
										   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
										   img.m_width, img.m_height, SDL_WINDOW_SHOWN
	);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
											 img.m_width, img.m_height
	);

    SDL_Event event;
    bool done = false;
    
    int numOfClusters = 10, maxIterations = 50;
    while (SDL_PollEvent(&event) || !done)
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    done = true;
                if (event.key.keysym.sym == SDLK_g)
	                img.MakeGray();
                if (event.key.keysym.sym == SDLK_s)
	                KMeans(img, numOfClusters, maxIterations);
            break;                
            case SDL_QUIT:
                done = true;
            break;
        }
	    UpdateImageTexture(img, renderer, texture);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(window);
    
    stbi_write_jpg("assets/panda_after.jpg",
				   img.m_width, img.m_height,
				   3, img.m_pixels, 0);

	img.ClearPixels();
    return 0;
}
