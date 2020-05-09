/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <atomic>
#include <common/log.hpp>
#include <cstdlib>
#include <cstring>
#include <emulator/config/config_toml.hpp>
#include <emulator/device/input_device.hpp>
#include <emulator/device/video_device.hpp>
#include <emulator/emulator.hpp>
#include <fmt/format.h>
#include <string>
#include <thread>

#include "device/audio_device.hpp"

static constexpr auto kNativeWidth = 240;
static constexpr auto kNativeHeight = 160;

static SDL_Window* g_window;

static std::atomic_bool g_sync_to_audio = true;
static int g_cycles_per_audio_frame = 0;

static auto input_device = std::make_shared<nba::BasicInputDevice>();

static SDL_Renderer* g_renderer;
static SDL_Texture* g_texture;
static std::uint32_t g_framebuffer[kNativeWidth * kNativeHeight];

static auto g_config = std::make_shared<nba::Config>();
static auto g_emulator = std::make_unique<nba::Emulator>(g_config);
static auto g_emulator_lock = std::mutex{};

void parse_arguments(int argc, char** argv);
void usage(char* app_name);
void load_game(std::string const& rom_path);

struct SDL2_VideoDevice : public nba::VideoDevice {
  void Draw(std::uint32_t* buffer) final {
    std::memcpy(g_framebuffer, buffer, sizeof(std::uint32_t) * kNativeWidth * kNativeHeight);
  }
};

void audio_passthrough(SDL2_AudioDevice* audio_device, std::int16_t* stream, int byte_len) {
  if (g_sync_to_audio) {
    g_emulator_lock.lock();
    g_emulator->Run(g_cycles_per_audio_frame);
    g_emulator_lock.unlock();
  }

  audio_device->InvokeCallback(stream, byte_len);
}

void init(int argc, char** argv) {
  config_toml_read(*g_config, "config.toml");
  parse_arguments(argc, argv);
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  g_window = SDL_CreateWindow("NanoboyAdvance",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    480,
    320,
    0);
  g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);
  g_texture  = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kNativeWidth, kNativeHeight);
  SDL_RenderSetLogicalSize(g_renderer, kNativeWidth, kNativeHeight);
  auto audio_device = std::make_shared<SDL2_AudioDevice>();
  audio_device->SetPassthrough((SDL_AudioCallback)audio_passthrough);
  g_config->audio_dev = audio_device;
  g_config->input_dev = input_device;
  g_config->video_dev = std::make_shared<SDL2_VideoDevice>();
  g_emulator->Reset();
  g_cycles_per_audio_frame = 16777216ULL * audio_device->GetBlockSize() / audio_device->GetSampleRate();
}

void destroy() {
  SDL_DestroyTexture(g_texture);
  SDL_DestroyRenderer(g_renderer);
  SDL_DestroyWindow(g_window);
}

void parse_arguments(int argc, char** argv) {
  auto i = 1;
  auto limit = argc - 1;
  while (i < limit) {
    auto key = std::string{argv[i++]};
    if (key == "--bios") {
      if (i == limit) {
        usage(argv[0]);
      }
      g_config->bios_path = std::string{argv[i++]};
    } else {
      usage(argv[0]);
    }
  }
  if (i == argc) {
    usage(argv[0]);
  }
  load_game(argv[i]);
}

void usage(char* app_name) {
  fmt::print("Usage: {0} [--bios bios_path] rom_path\n", app_name);
  std::exit(-1);
}

void load_game(std::string const& rom_path) {
  using StatusCode = nba::Emulator::StatusCode;

  switch (g_emulator->LoadGame(rom_path)) {
  case StatusCode::GameNotFound:
    fmt::print("Cannot open ROM: {0}\n", rom_path);
    std::exit(-2);
  case StatusCode::BiosNotFound:
    fmt::print("Cannot open BIOS: {0}\n", g_config->bios_path);
    std::exit(-3);
  case StatusCode::GameWrongSize:
    fmt::print("The provided ROM file is larger than the maximum 32 MiB.\n");
    std::exit(-4);
  case StatusCode::BiosWrongSize:
    fmt::print("The provided BIOS file does not match the expected size of 16 KiB.\n");
    std::exit(-5);
  }
}

void update_key(SDL_KeyboardEvent* event) {
  bool pressed = event->type == SDL_KEYDOWN;

  switch (event->keysym.sym) {
    case SDLK_a:
      input_device->SetKeyStatus(nba::InputDevice::Key::A, pressed);
      break;
    case SDLK_s:
      input_device->SetKeyStatus(nba::InputDevice::Key::B, pressed);
      break;
    case SDLK_d:
      input_device->SetKeyStatus(nba::InputDevice::Key::L, pressed);
      break;
    case SDLK_f:
      input_device->SetKeyStatus(nba::InputDevice::Key::R, pressed);
      break;
    case SDLK_UP:
      input_device->SetKeyStatus(nba::InputDevice::Key::Up, pressed);
      break;
    case SDLK_DOWN:
      input_device->SetKeyStatus(nba::InputDevice::Key::Down, pressed);
      break;
    case SDLK_LEFT:
      input_device->SetKeyStatus(nba::InputDevice::Key::Left, pressed);
      break;
    case SDLK_RIGHT:
      input_device->SetKeyStatus(nba::InputDevice::Key::Right, pressed);
      break;
    case SDLK_BACKSPACE:
      input_device->SetKeyStatus(nba::InputDevice::Key::Select, pressed);
      break;
    case SDLK_RETURN:
      input_device->SetKeyStatus(nba::InputDevice::Key::Start, pressed);
      break;
    case SDLK_SPACE:
      g_sync_to_audio = !pressed;
      if (pressed) {
        SDL_GL_SetSwapInterval(0);
      } else {
        SDL_GL_SetSwapInterval(1);
      }
      break;
  }
}

void loop() {
  auto event = SDL_Event{};

  for (;;) {
    if (!g_sync_to_audio) {
      g_emulator_lock.lock();
      g_emulator->Frame();
      g_emulator_lock.unlock();
    }
    if (g_framebuffer != nullptr) {
      SDL_UpdateTexture(g_texture, nullptr, g_framebuffer, kNativeWidth * sizeof(std::uint32_t));
      SDL_RenderClear(g_renderer);
      SDL_RenderCopy(g_renderer, g_texture, nullptr, nullptr);
      SDL_RenderPresent(g_renderer);
    }
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        return;
      if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
        update_key(reinterpret_cast<SDL_KeyboardEvent*>(&event));
    }
  }
}

int main(int argc, char** argv) {
  init(argc, argv);
  loop();
  destroy();
  return 0;
}
