#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include "KMeans.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendor/stb/stb_image_write.h"

static void UpdateImageTexture(Image& img, SDL_Renderer* renderer, SDL_Texture* texture)
{
	SDL_UpdateTexture(texture, nullptr, (void*)img.m_pixels, (int) img.m_pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
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
                {
	                KMeans kmeans(img);
					kmeans.Partition();
				}
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

