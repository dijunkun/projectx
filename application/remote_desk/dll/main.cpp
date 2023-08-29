

#include <stdio.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "screen_capture_wgc.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
};

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

int screen_w = 2560, screen_h = 1440;
const int pixel_w = 1280, pixel_h = 720;

unsigned char buffer[pixel_w * pixel_h * 3 / 2];
unsigned char dst_buffer[pixel_w * pixel_h * 3 / 2];
unsigned char rgbData[pixel_w * pixel_h * 4];
SDL_Texture *sdlTexture = nullptr;
SDL_Renderer *sdlRenderer = nullptr;
SDL_Rect sdlRect;

// Refresh Event
#define REFRESH_EVENT (SDL_USEREVENT + 1)

int thread_exit = 0;

int refresh_video(void *opaque) {
  while (thread_exit == 0) {
    SDL_Event event;
    event.type = REFRESH_EVENT;
    SDL_PushEvent(&event);
    SDL_Delay(10);
  }
  return 0;
}

int YUV420ToNV12FFmpeg(unsigned char *src_buffer, int width, int height,
                       unsigned char *des_buffer) {
  AVFrame *Input_pFrame = av_frame_alloc();
  AVFrame *Output_pFrame = av_frame_alloc();
  struct SwsContext *img_convert_ctx = sws_getContext(
      width, height, AV_PIX_FMT_YUV420P, 1280, 720, AV_PIX_FMT_NV12,
      SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

  av_image_fill_arrays(Input_pFrame->data, Input_pFrame->linesize, src_buffer,
                       AV_PIX_FMT_YUV420P, width, height, 1);
  av_image_fill_arrays(Output_pFrame->data, Output_pFrame->linesize, des_buffer,
                       AV_PIX_FMT_NV12, 1280, 720, 1);

  sws_scale(img_convert_ctx, (uint8_t const **)Input_pFrame->data,
            Input_pFrame->linesize, 0, height, Output_pFrame->data,
            Output_pFrame->linesize);

  if (Input_pFrame) av_free(Input_pFrame);
  if (Output_pFrame) av_free(Output_pFrame);
  if (img_convert_ctx) sws_freeContext(img_convert_ctx);

  return 0;
}

int BGRAToNV12FFmpeg(unsigned char *src_buffer, int width, int height,
                     unsigned char *dst_buffer) {
  AVFrame *Input_pFrame = av_frame_alloc();
  AVFrame *Output_pFrame = av_frame_alloc();
  struct SwsContext *img_convert_ctx =
      sws_getContext(width, height, AV_PIX_FMT_BGRA, 1280, 720, AV_PIX_FMT_NV12,
                     SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

  av_image_fill_arrays(Input_pFrame->data, Input_pFrame->linesize, src_buffer,
                       AV_PIX_FMT_BGRA, width, height, 1);
  av_image_fill_arrays(Output_pFrame->data, Output_pFrame->linesize, dst_buffer,
                       AV_PIX_FMT_NV12, 1280, 720, 1);

  sws_scale(img_convert_ctx, (uint8_t const **)Input_pFrame->data,
            Input_pFrame->linesize, 0, height, Output_pFrame->data,
            Output_pFrame->linesize);

  if (Input_pFrame) av_free(Input_pFrame);
  if (Output_pFrame) av_free(Output_pFrame);
  if (img_convert_ctx) sws_freeContext(img_convert_ctx);

  return 0;
}

void OnFrame(unsigned char *data, int size, int width, int height) {
  std::cout << "Receive frame: w:" << width << " h:" << height
            << " size:" << size << std::endl;
  // YUV420ToNV12FFmpeg(data, width, height, dst_buffer);
  BGRAToNV12FFmpeg(data, width, height, dst_buffer);

  SDL_UpdateTexture(sdlTexture, NULL, dst_buffer, pixel_w);
  // memcpy(rgbData, data, width * height);

  // FIX: If window is resize
  sdlRect.x = 0;
  sdlRect.y = 0;
  sdlRect.w = screen_w;
  sdlRect.h = screen_h;

  SDL_RenderClear(sdlRenderer);
  SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
  SDL_RenderPresent(sdlRenderer);
}

int main() {
  ScreenCaptureWgc *recorder = new ScreenCaptureWgc();

  RECORD_DESKTOP_RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = GetSystemMetrics(SM_CXSCREEN);
  rect.bottom = GetSystemMetrics(SM_CYSCREEN);

  recorder->Init(rect, 60, OnFrame);

  recorder->Start();

  if (SDL_Init(SDL_INIT_VIDEO)) {
    printf("Could not initialize SDL - %s\n", SDL_GetError());
    return -1;
  }

  SDL_Window *screen;
  screen = SDL_CreateWindow("RTS Receiver", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, screen_w / 2, screen_h / 2,
                            SDL_WINDOW_RESIZABLE);
  if (!screen) {
    printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
    return -1;
  }
  sdlRenderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);

  Uint32 pixformat = 0;
  pixformat = SDL_PIXELFORMAT_NV12;

  sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat,
                                 SDL_TEXTUREACCESS_STREAMING, 1280, 720);

  // SDL_Surface *surface =
  //     SDL_CreateRGBSurfaceFrom(rgbData, pixel_w, pixel_h, 24, pixel_w * 3,
  //                              0x000000FF, 0x0000FF00, 0x00FF0000, 0);

  // sdlTexture = SDL_CreateTextureFromSurface(sdlRenderer, surface);

  SDL_Thread *refresh_thread = SDL_CreateThread(refresh_video, NULL, NULL);
  SDL_Event event;
  while (1) {
    // Wait
    SDL_WaitEvent(&event);
    if (event.type == REFRESH_EVENT) {
    } else if (event.type == SDL_WINDOWEVENT) {
      // If Resize
      SDL_GetWindowSize(screen, &screen_w, &screen_h);
    } else if (event.type == SDL_QUIT) {
      break;
    }
  }

  return 0;
}
