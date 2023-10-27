#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
// Windows
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include "SDL2/SDL.h"
};
#else
// Linux...
#ifdef __cplusplus
extern "C" {
#endif
#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
};
#endif
#endif

// Output YUV420P
#define OUTPUT_YUV420P 0
//'1' Use Dshow
//'0' Use GDIgrab
#define USE_DSHOW 0

// Refresh Event
#define SFM_REFRESH_EVENT (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT (SDL_USEREVENT + 2)

int thread_exit = 0;
SDL_Texture *sdlTexture = nullptr;
SDL_Renderer *sdlRenderer = nullptr;
SDL_Rect sdlRect;

int YUV420ToNV12FFmpeg(unsigned char *src_buffer, int width, int height,
                       unsigned char *des_buffer) {
  AVFrame *Input_pFrame = av_frame_alloc();
  AVFrame *Output_pFrame = av_frame_alloc();
  struct SwsContext *img_convert_ctx = sws_getContext(
      width, height, AV_PIX_FMT_NV12, width, height, AV_PIX_FMT_YUV420P,
      SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

  av_image_fill_arrays(Input_pFrame->data, Input_pFrame->linesize, src_buffer,
                       AV_PIX_FMT_NV12, width, height, 1);
  av_image_fill_arrays(Output_pFrame->data, Output_pFrame->linesize, des_buffer,
                       AV_PIX_FMT_YUV420P, width, height, 1);

  sws_scale(img_convert_ctx, (uint8_t const **)Input_pFrame->data,
            Input_pFrame->linesize, 0, height, Output_pFrame->data,
            Output_pFrame->linesize);

  if (Input_pFrame) av_free(Input_pFrame);
  if (Output_pFrame) av_free(Output_pFrame);
  if (img_convert_ctx) sws_freeContext(img_convert_ctx);

  return 0;
}

int sfp_refresh_thread(void *opaque) {
  thread_exit = 0;
  while (!thread_exit) {
    SDL_Event event;
    event.type = SFM_REFRESH_EVENT;
    SDL_PushEvent(&event);
    SDL_Delay(40);
  }
  thread_exit = 0;
  // Break
  SDL_Event event;
  event.type = SFM_BREAK_EVENT;
  SDL_PushEvent(&event);

  return 0;
}

// Show Dshow Device
// void show_dshow_device() {
//   AVFormatContext *pFormatCtx = avformat_alloc_context();
//   AVDictionary *options = NULL;
//   av_dict_set(&options, "list_devices", "true", 0);
//   AVInputFormat *iformat = av_find_input_format("dshow");
//   printf("========Device Info=============\n");
//   avformat_open_input(&pFormatCtx, "video=dummy", iformat, &options);
//   printf("================================\n");
// }

// Show AVFoundation Device
void show_avfoundation_device() {
  const AVFormatContext *const_pFormatCtx = avformat_alloc_context();
  AVFormatContext *pFormatCtx = const_cast<AVFormatContext *>(const_pFormatCtx);

  AVDictionary *options = NULL;
  av_dict_set(&options, "list_devices", "true", 0);
  const AVInputFormat *const_iformat = av_find_input_format("avfoundation");
  AVInputFormat *iformat = const_cast<AVInputFormat *>(const_iformat);

  printf("==AVFoundation Device Info===\n");
  avformat_open_input(&pFormatCtx, "", iformat, &options);
  printf("=============================\n");
}

int main(int argc, char *argv[]) {
  AVFormatContext *pFormatCtx;
  int i, videoindex;
  AVCodecContext *pCodecCtx;
  AVCodec *pCodec;

  // avformat_network_init();
  pFormatCtx = avformat_alloc_context();

  // Open File
  // char filepath[]="src01_480x272_22.h265";
  // avformat_open_input(&pFormatCtx,filepath,NULL,NULL)

  // Register Device
  avdevice_register_all();
  // Windows

  // Linux
  AVDictionary *options = NULL;
  // Set some options
  // grabbing frame rate
  // av_dict_set(&options, "framerate", "5", 0);
  // Make the grabbed area follow the mouse
  // av_dict_set(&options, "follow_mouse", "centered", 0);
  // Video frame size. The default is to capture the full screen
  // av_dict_set(&options, "video_size", "640x480", 0);
  AVInputFormat *ifmt = (AVInputFormat *)av_find_input_format("x11grab");
  if(!ifmt){
    printf("Couldn't find_input_format\n");
  }

  // Grab at position 10,20
  if (avformat_open_input(&pFormatCtx, ":0.0", ifmt, &options) != 0) {
    printf("Couldn't open input stream.\n");
    return -1;
  }

  if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
    printf("Couldn't find stream information.\n");
    return -1;
  }
  videoindex = -1;
  for (i = 0; i < pFormatCtx->nb_streams; i++)
    if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoindex = i;
      break;
    }
  if (videoindex == -1) {
    printf("Didn't find a video stream.\n");
    return -1;
  }
  // pCodecCtx = pFormatCtx->streams[videoindex]->codec;

  pCodecCtx = avcodec_alloc_context3(NULL);
  avcodec_parameters_to_context(pCodecCtx,
                                pFormatCtx->streams[videoindex]->codecpar);

  pCodec = const_cast<AVCodec *>(avcodec_find_decoder(pCodecCtx->codec_id));
  // pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
  if (pCodec == NULL) {
    printf("Codec not found.\n");
    return -1;
  }
  if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
    printf("Could not open codec.\n");
    return -1;
  }
  AVFrame *pFrame, *pFrameYUV;
  pFrame = av_frame_alloc();
  pFrameYUV = av_frame_alloc();
  // unsigned char *out_buffer=(unsigned char
  // *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
  // pCodecCtx->height)); avpicture_fill((AVPicture *)pFrameYUV, out_buffer,
  // AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
  // SDL----------------------------
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
    printf("Could not initialize SDL - %s\n", SDL_GetError());
    return -1;
  }
  const int pixel_w = 640, pixel_h = 360;
  int screen_w = 640, screen_h = 360;
  // const SDL_VideoInfo *vi = SDL_GetVideoInfo();
  // Half of the Desktop's width and height.
  screen_w = 640;
  screen_h = 360;
  // SDL_Surface *screen;
  // screen = SDL_SetVideoMode(screen_w, screen_h, 0, 0);
  SDL_Window *screen;
  screen = SDL_CreateWindow("RTS Receiver", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h,
                            SDL_WINDOW_RESIZABLE);

  if (!screen) {
    printf("SDL: could not set video mode - exiting:%s\n", SDL_GetError());
    return -1;
  }
  // SDL_Overlay *bmp;
  // bmp = SDL_CreateYUVOverlay(pCodecCtx->width, pCodecCtx->height,
  //                            SDL_YV12_OVERLAY, screen);

  sdlRenderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);

  Uint32 pixformat = 0;
  pixformat = SDL_PIXELFORMAT_NV12;

  SDL_Texture *sdlTexture = nullptr;
  sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat,
                                 SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);

  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = screen_w;
  rect.h = screen_h;
  // SDL End------------------------
  int ret, got_picture;

  AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));

#if OUTPUT_YUV420P
  FILE *fp_yuv = fopen("output.yuv", "wb+");
#endif

  struct SwsContext *img_convert_ctx;
  img_convert_ctx = sws_getContext(
      pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width,
      pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
  //------------------------------
  SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);
  //
  // SDL_WM_SetCaption("Simplest FFmpeg Grab Desktop", NULL);
  // Event Loop
  SDL_Event event;

  for (;;) {
    // Wait
    SDL_WaitEvent(&event);
    if (event.type == SFM_REFRESH_EVENT) {
      //------------------------------
      if (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == videoindex) {
          avcodec_send_packet(pCodecCtx, packet);
          got_picture = avcodec_receive_frame(pCodecCtx, pFrame);
          // ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,
          // packet);
          if (ret < 0) {
            printf("Decode Error.\n");
            return -1;
          }
          if (got_picture) {
            //             SDL_LockYUVOverlay(bmp);
            //             pFrameYUV->data[0] = bmp->pixels[0];
            //             pFrameYUV->data[1] = bmp->pixels[2];
            //             pFrameYUV->data[2] = bmp->pixels[1];
            //             pFrameYUV->linesize[0] = bmp->pitches[0];
            //             pFrameYUV->linesize[1] = bmp->pitches[2];
            //             pFrameYUV->linesize[2] = bmp->pitches[1];
            //             sws_scale(img_convert_ctx,
            //                       (const unsigned char *const *)pFrame->data,
            //                       pFrame->linesize, 0, pCodecCtx->height,
            //                       pFrameYUV->data, pFrameYUV->linesize);

            // #if OUTPUT_YUV420P
            //             int y_size = pCodecCtx->width * pCodecCtx->height;
            //             fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv); // Y
            //             fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv); //
            //             U fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);
            //             // V
            // #endif
            //             SDL_UnlockYUVOverlay(bmp);

            //             SDL_DisplayYUVOverlay(bmp, &rect);

            // YUV420ToNV12FFmpeg(buffer, pixel_w, pixel_h, dst_buffer);

            // SDL_UpdateTexture(sdlTexture, NULL, dst_buffer, pixel_w);

            // // FIX: If window is resize
            // sdlRect.x = 0;
            // sdlRect.y = 0;
            // sdlRect.w = screen_w;
            // sdlRect.h = screen_h;

            // SDL_RenderClear(sdlRenderer);
            // SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
            // SDL_RenderPresent(sdlRenderer);
          }
        }
        // av_free_packet(packet);
      } else {
        // Exit Thread
        thread_exit = 1;
      }
    } else if (event.type == SDL_QUIT) {
      thread_exit = 1;
    } else if (event.type == SFM_BREAK_EVENT) {
      break;
    }
  }

  sws_freeContext(img_convert_ctx);

#if OUTPUT_YUV420P
  fclose(fp_yuv);
#endif

  SDL_Quit();

  // av_free(out_buffer);
  av_free(pFrameYUV);
  avcodec_close(pCodecCtx);
  avformat_close_input(&pFormatCtx);

  return 0;
}