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
#include <third_party/toml11/toml.hpp>
#include <thread>
#include <unordered_map>

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

struct KeyMap {
  SDL_Keycode fastforward = SDLK_SPACE;
  SDL_Keycode reset = SDLK_F9;
  SDL_Keycode fullscreen = SDLK_F10;
  std::unordered_map<SDL_Keycode, nba::InputDevice::Key> gba;
} keymap;

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

void load_keymap() {
  toml::value data;

  try {
    data = toml::parse("keymap.toml");
  } catch (std::exception& ex) {
    LOG_WARN("Failed to load or parse keymap configuration.");
  }

  if (data.contains("general")) {
    auto general_result = toml::expect<toml::value>(data.at("general"));

    if (general_result.is_ok()) {
      auto general = general_result.unwrap();
      keymap.fastforward = SDL_GetKeyFromName(toml::find_or<std::string>(general, "fastforward", "Space").c_str());
      keymap.reset = SDL_GetKeyFromName(toml::find_or<std::string>(general, "reset", "F9").c_str());
      keymap.fullscreen = SDL_GetKeyFromName(toml::find_or<std::string>(general, "fullscreen", "F10").c_str());
    }
  }

  if (data.contains("gba")) {
    auto gba_result = toml::expect<toml::value>(data.at("gba"));

    if (gba_result.is_ok()) {
      auto gba = gba_result.unwrap();
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "a", "A").c_str())] = nba::InputDevice::Key::A;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "b", "B").c_str())] = nba::InputDevice::Key::B;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "l", "D").c_str())] = nba::InputDevice::Key::L;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "r", "F").c_str())] = nba::InputDevice::Key::R;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "start", "Return").c_str())] = nba::InputDevice::Key::Start;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "select", "Backspace").c_str())] = nba::InputDevice::Key::Select;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "up", "Up").c_str())] = nba::InputDevice::Key::Up;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "down", "Down").c_str())] = nba::InputDevice::Key::Down;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "left", "Left").c_str())] = nba::InputDevice::Key::Left;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "right", "Right").c_str())] = nba::InputDevice::Key::Right;
    }
  }
}

void update_fullscreen() {
  SDL_SetWindowFullscreen(g_window, g_config->video.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void init(int argc, char** argv) {
  config_toml_read(*g_config, "config.toml");
  parse_arguments(argc, argv);
  load_keymap();
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  g_window = SDL_CreateWindow("NanoboyAdvance",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    kNativeWidth * g_config->video.scale,
    kNativeHeight * g_config->video.scale,
    0);
  g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);
  g_texture  = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kNativeWidth, kNativeHeight);
  SDL_RenderSetLogicalSize(g_renderer, kNativeWidth, kNativeHeight);
  update_fullscreen();
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
    } else if (key == "--fullscreen") {
      g_config->video.fullscreen = true;
    } else if (key == "--scale") {
      if (i == limit) {
        usage(argv[0]);
      }
      g_config->video.scale = std::atoi(argv[i++]);
      if (g_config->video.scale == 0) {
        usage(argv[0]);
      }
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
  fmt::print("Usage: {0} [--bios bios_path] [--fullscreen] [--scale factor] rom_path\n", app_name);
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
  auto key = event->keysym.sym;
  
  if (key == keymap.fastforward) {
    g_sync_to_audio = !pressed;
    if (pressed) {
      SDL_GL_SetSwapInterval(0);
    } else {
      SDL_GL_SetSwapInterval(1);
    }
  }

  if (key == keymap.reset && !pressed) {
    g_emulator_lock.lock();
    g_emulator->Reset();
    g_emulator_lock.unlock();
  }

  if (key == keymap.fullscreen && !pressed) {
    g_config->video.fullscreen = !g_config->video.fullscreen;
    update_fullscreen();
  }

  auto match = keymap.gba.find(key);
  if (match != keymap.gba.end()) {
    input_device->SetKeyStatus(match->second, pressed);
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
