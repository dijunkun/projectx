#include <SDL.h>
#include <stdio.h>
#ifdef _WIN32
#include <Winsock2.h>
#include <iphlpapi.h>
#elif __APPLE__
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <sys/socket.h>
#include <sys/types.h>
#elif __linux__
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "log.h"
#include "screen_capture_wgc.h"
#include "x.h"

#define NV12_BUFFER_SIZE 1280 * 720 * 3 / 2

int screen_w = 1280, screen_h = 720;
const int pixel_w = 1280, pixel_h = 720;

unsigned char dst_buffer[pixel_w * pixel_h * 3 / 2];
SDL_Texture *sdlTexture = nullptr;
SDL_Renderer *sdlRenderer = nullptr;
SDL_Rect sdlRect;
SDL_Window *window;

uint32_t start_time, end_time, elapsed_time;
uint32_t frame_count = 0;
int fps = 0;
std::string window_title = "Remote Desk Client";

// Refresh Event
#define REFRESH_EVENT (SDL_USEREVENT + 1)
#define QUIT_EVENT (SDL_USEREVENT + 2)

int thread_exit = 0;
PeerPtr *peer = nullptr;
bool joined = false;
bool received_frame = false;

ScreenCaptureWgc *screen_capture = nullptr;

char *nv12_buffer_ = nullptr;
std::chrono::steady_clock::time_point last_frame_time_;

typedef enum { mouse = 0, keyboard } ControlType;
typedef enum { move = 0, left_down, left_up, right_down, right_up } MouseFlag;
typedef enum { key_down = 0, key_up } KeyFlag;
typedef struct {
  size_t x;
  size_t y;
  MouseFlag flag;
} Mouse;

typedef struct {
  size_t key_value;
  KeyFlag flag;
} Key;

typedef struct {
  ControlType type;
  union {
    Mouse m;
    Key k;
  };
} RemoteAction;

inline int ProcessMouseKeyEven(SDL_Event &ev) {
  float ratio = 1280.0 / screen_w;

  RemoteAction remote_action;

  if (SDL_KEYDOWN == ev.type)  // SDL_KEYUP
  {
    printf("SDLK_DOWN: %d\n", SDL_KeyCode(ev.key.keysym.sym));
    if (SDLK_DOWN == ev.key.keysym.sym) {
      printf("SDLK_DOWN  \n");

    } else if (SDLK_UP == ev.key.keysym.sym) {
      printf("SDLK_UP  \n");

    } else if (SDLK_LEFT == ev.key.keysym.sym) {
      printf("SDLK_LEFT  \n");

    } else if (SDLK_RIGHT == ev.key.keysym.sym) {
      printf("SDLK_RIGHT  \n");
    }
  } else if (SDL_MOUSEBUTTONDOWN == ev.type) {
    if (SDL_BUTTON_LEFT == ev.button.button) {
      int px = ev.button.x;
      int py = ev.button.y;
      printf("SDL_MOUSEBUTTONDOWN x, y %d %d  \n", px, py);

      remote_action.type = ControlType::mouse;
      remote_action.m.flag = MouseFlag::left_down;
      remote_action.m.x = ev.button.x * ratio;
      remote_action.m.y = ev.button.y * ratio;

      SendData(peer, DATA_TYPE::DATA, (const char *)&remote_action,
               sizeof(remote_action));

    } else if (SDL_BUTTON_RIGHT == ev.button.button) {
      int px = ev.button.x;
      int py = ev.button.y;
      printf("SDL_BUTTON_RIGHT x, y %d %d  \n", px, py);

      remote_action.type = ControlType::mouse;
      remote_action.m.flag = MouseFlag::right_down;
      remote_action.m.x = ev.button.x * ratio;
      remote_action.m.y = ev.button.y * ratio;

      SendData(peer, DATA_TYPE::DATA, (const char *)&remote_action,
               sizeof(remote_action));
    }
  } else if (SDL_MOUSEBUTTONUP == ev.type) {
    if (SDL_BUTTON_LEFT == ev.button.button) {
      int px = ev.button.x;
      int py = ev.button.y;
      printf("SDL_MOUSEBUTTONUP x, y %d %d  \n", px, py);

      remote_action.type = ControlType::mouse;
      remote_action.m.flag = MouseFlag::left_up;
      remote_action.m.x = ev.button.x * ratio;
      remote_action.m.y = ev.button.y * ratio;

      SendData(peer, DATA_TYPE::DATA, (const char *)&remote_action,
               sizeof(remote_action));

    } else if (SDL_BUTTON_RIGHT == ev.button.button) {
      int px = ev.button.x;
      int py = ev.button.y;
      printf("SDL_MOUSEBUTTONUP x, y %d %d  \n", px, py);

      remote_action.type = ControlType::mouse;
      remote_action.m.flag = MouseFlag::right_up;
      remote_action.m.x = ev.button.x * ratio;
      remote_action.m.y = ev.button.y * ratio;

      SendData(peer, DATA_TYPE::DATA, (const char *)&remote_action,
               sizeof(remote_action));
    }
  } else if (SDL_MOUSEMOTION == ev.type) {
    int px = ev.motion.x;
    int py = ev.motion.y;

    printf("SDL_MOUSEMOTION x, y %d %d  \n", px, py);

    remote_action.type = ControlType::mouse;
    remote_action.m.flag = MouseFlag::move;
    remote_action.m.x = ev.button.x * ratio;
    remote_action.m.y = ev.button.y * ratio;

    SendData(peer, DATA_TYPE::DATA, (const char *)&remote_action,
             sizeof(remote_action));
  } else if (SDL_QUIT == ev.type) {
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
    printf("SDL_QUIT\n");
    return 0;
  }

  return 0;
}

void ReceiveVideoBuffer(const char *data, size_t size, const char *user_id,
                        size_t user_id_size) {
  // std::cout << "Receive: [" << user_id << "] " << std::endl;
  memcpy(dst_buffer, data, size);

  SDL_Event event;
  event.type = REFRESH_EVENT;
  SDL_PushEvent(&event);
  received_frame = true;
}

void ReceiveAudioBuffer(const char *data, size_t size, const char *user_id,
                        size_t user_id_size) {
  std::cout << "Receive audio, size " << size << ", user [" << user_id << "] "
            << std::endl;
}

void ReceiveDataBuffer(const char *data, size_t size, const char *user_id,
                       size_t user_id_size) {
  std::cout << "Receive data, size " << size << ", user [" << user_id << "] "
            << std::endl;
}

std::string GetMac(char *mac_addr) {
  int len = 0;
#ifdef _WIN32
  IP_ADAPTER_INFO adapterInfo[16];
  DWORD bufferSize = sizeof(adapterInfo);

  DWORD result = GetAdaptersInfo(adapterInfo, &bufferSize);
  if (result == ERROR_SUCCESS) {
    PIP_ADAPTER_INFO adapter = adapterInfo;
    while (adapter) {
      for (UINT i = 0; i < adapter->AddressLength; i++) {
        len += sprintf(mac_addr + len, "%.2X", adapter->Address[i]);
      }
      break;
    }
  }
#elif __APPLE__
  std::string if_name = "en0";

  struct ifaddrs *addrs;
  struct ifaddrs *cursor;
  const struct sockaddr_dl *dlAddr;

  if (!getifaddrs(&addrs)) {
    cursor = addrs;
    while (cursor != 0) {
      const struct sockaddr_dl *socAddr =
          (const struct sockaddr_dl *)cursor->ifa_addr;
      if ((cursor->ifa_addr->sa_family == AF_LINK) &&
          (socAddr->sdl_type == IFT_ETHER) &&
          strcmp(if_name.c_str(), cursor->ifa_name) == 0) {
        dlAddr = (const struct sockaddr_dl *)cursor->ifa_addr;
        const unsigned char *base =
            (const unsigned char *)&dlAddr->sdl_data[dlAddr->sdl_nlen];
        for (int i = 0; i < dlAddr->sdl_alen; i++) {
          len += sprintf(mac_addr + len, "%.2X", base[i]);
        }
      }
      cursor = cursor->ifa_next;
    }
    freeifaddrs(addrs);
  }
#elif __linux__
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return "";
  }
  struct ifreq ifr;
  struct ifconf ifc;
  char buf[1024];
  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sock, SIOCGIFCONF, &ifc) < 0) {
    close(sock);
    return "";
  }
  struct ifreq *it = ifc.ifc_req;
  const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct ifreq));
  for (; it != end; ++it) {
    std::strcpy(ifr.ifr_name, it->ifr_name);
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
      continue;
    }
    if (ifr.ifr_flags & IFF_LOOPBACK) {
      continue;
    }
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
      continue;
    }
    std::string mac_address;
    for (int i = 0; i < 6; ++i) {
      len += sprintf(mac_addr + len, "%.2X", ifr.ifr_hwaddr.sa_data[i] & 0xff);
    }
    break;
  }
  close(sock);
#endif
  return mac_addr;
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

int main() {
  LOG_INFO("Remote desk");
  std::string default_cfg_path = "../../../../config/config.ini";
  std::ifstream f(default_cfg_path.c_str());

  Params params;
  params.cfg_path = f.good() ? "../../../../config/config.ini" : "config.ini";
  params.on_receive_video_buffer = ReceiveVideoBuffer;
  params.on_receive_audio_buffer = ReceiveAudioBuffer;
  params.on_receive_data_buffer = ReceiveDataBuffer;

  std::string transmission_id = "000001";
  char mac_addr[10];
  GetMac(mac_addr);

  peer = CreatePeer(&params);

  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
      0) {
    printf("Error: %s\n", SDL_GetError());
    return -1;
  }

  // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  // Create window with SDL_Renderer graphics context
  SDL_WindowFlags window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            1280, 720, window_flags);
  sdlRenderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (sdlRenderer == nullptr) {
    SDL_Log("Error creating SDL_Renderer!");
    return 0;
  }

  Uint32 pixformat = 0;
  pixformat = SDL_PIXELFORMAT_NV12;

  sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat,
                                 SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);

  // SDL_RendererInfo info;
  // SDL_GetRendererInfo(sdlRenderer, &info);
  // SDL_Log("Current SDL_Renderer: %s", info.name);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForSDLRenderer(window, sdlRenderer);
  ImGui_ImplSDLRenderer2_Init(sdlRenderer);

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  bool done = false;
  while (!done) {
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    {
      static float f = 0.0f;
      static int counter = 0;

      const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
      ImGui::SetNextWindowSize(ImVec2(180, 185));

      ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoResize);

      {
        ImGui::Text("LOCAL  ID:");
        ImGui::SameLine();

        ImGui::Selectable(mac_addr, false,
                          ImGuiSelectableFlags_AllowDoubleClick);

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
          ImGui::SetClipboardText(mac_addr);
        }

        ImGui::Spacing();
        static bool buttonPressed = false;
        static const char *label = "Online";

        if (ImGui::Button(label)) {
          std::string user_id = "S-" + std::string(GetMac(mac_addr));

          if (strcmp(label, "Online") == 0) {
            CreateConnection(peer, mac_addr, user_id.c_str());

            nv12_buffer_ = new char[NV12_BUFFER_SIZE];

            screen_capture = new ScreenCaptureWgc();

            RECORD_DESKTOP_RECT rect;
            rect.left = 0;
            rect.top = 0;
            rect.right = GetSystemMetrics(SM_CXSCREEN);
            rect.bottom = GetSystemMetrics(SM_CYSCREEN);

            screen_w = GetSystemMetrics(SM_CXSCREEN);
            screen_h = GetSystemMetrics(SM_CYSCREEN);

            last_frame_time_ = std::chrono::high_resolution_clock::now();
            screen_capture->Init(
                rect, 60,
                [](unsigned char *data, int size, int width,
                   int height) -> void {
                  // std::cout << "Send" << std::endl;

                  auto now_time = std::chrono::high_resolution_clock::now();
                  std::chrono::duration<double> duration =
                      now_time - last_frame_time_;
                  auto tc = duration.count() * 1000;

                  if (tc >= 0) {
                    BGRAToNV12FFmpeg(data, width, height,
                                     (unsigned char *)nv12_buffer_);
                    SendData(peer, DATA_TYPE::VIDEO, (const char *)nv12_buffer_,
                             NV12_BUFFER_SIZE);
                    last_frame_time_ = now_time;
                  }
                });

            screen_capture->Start();
          } else {
            LeaveConnection(peer);
          }
          buttonPressed = !buttonPressed;
          label = buttonPressed ? "Offline" : "Online";
        }
      }

      ImGui::Spacing();

      ImGui::Separator();

      ImGui::Spacing();

      {
        static char buf[20] = "";
        ImGui::Text("REMOTE ID:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(114);
        ImGui::InputTextWithHint("", "000000", buf, IM_ARRAYSIZE(buf),
                                 ImGuiInputTextFlags_AllowTabInput);

        ImGui::Spacing();
        if (ImGui::Button("Connect") && !joined) {
          std::string user_id = "C-" + std::string(GetMac(mac_addr));
          JoinConnection(peer, buf, user_id.c_str());
          joined = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Disconnect")) {
          LeaveConnection(peer);
          joined = false;
        }
      }

      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    SDL_RenderSetScale(sdlRenderer, io.DisplayFramebufferScale.x,
                       io.DisplayFramebufferScale.y);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) {
        done = true;
      } else if (event.type == SDL_WINDOWEVENT &&
                 event.window.event == SDL_WINDOWEVENT_RESIZED) {
        // int new_screen_w = 0;
        // int new_screen_h = 0;
        // SDL_GetWindowSize(window, &new_screen_w, &new_screen_h);

        // if (new_screen_w != screen_w) {
        //   screen_w = new_screen_w;
        //   screen_h = new_screen_w * 9 / 16;
        // } else if (new_screen_h != screen_h) {
        //   screen_w = new_screen_h * 16 / 9;
        //   screen_h = new_screen_h;
        // }
        // SDL_SetWindowSize(window, screen_w, screen_h);
        printf("Resize windows: %dx%d\n", screen_w, screen_h);
      } else if (event.type == SDL_WINDOWEVENT &&
                 event.window.event == SDL_WINDOWEVENT_CLOSE &&
                 event.window.windowID == SDL_GetWindowID(window)) {
        done = true;
      } else if (event.type == REFRESH_EVENT) {
        sdlRect.x = 0;
        sdlRect.y = 0;
        sdlRect.w = screen_w;
        sdlRect.h = screen_h;

        SDL_UpdateTexture(sdlTexture, NULL, dst_buffer, pixel_w);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
      } else {
        if (joined) {
          ProcessMouseKeyEven(event);
        }
      }
    }

    if (!joined || !received_frame) {
      SDL_RenderClear(sdlRenderer);
      SDL_SetRenderDrawColor(
          sdlRenderer, (Uint8)(clear_color.x * 0), (Uint8)(clear_color.y * 0),
          (Uint8)(clear_color.z * 0), (Uint8)(clear_color.w * 0));
    }

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(sdlRenderer);

    frame_count++;
    end_time = SDL_GetTicks();
    elapsed_time = end_time - start_time;
    if (elapsed_time >= 1000) {
      fps = frame_count / (elapsed_time / 1000);
      frame_count = 0;
      window_title = "Remote Desk Client FPS [" + std::to_string(fps) + "]";
      // For MacOS, UI frameworks can only be called from the main thread
      SDL_SetWindowTitle(window, window_title.c_str());
      start_time = end_time;
    }
  }

  // Cleanup
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(sdlRenderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}