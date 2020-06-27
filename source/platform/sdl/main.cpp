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
#include <iterator>
#include <optional>
#include <string>
#include <third_party/toml11/toml.hpp>
#include <unordered_map>

#include "device/audio_device.hpp"

#include "GL/glew.h"

#undef main

static constexpr auto kNativeWidth = 240;
static constexpr auto kNativeHeight = 160;

static SDL_Window* g_window;
static SDL_GLContext g_gl_context;
static GLuint g_gl_texture;
static std::uint32_t g_framebuffer[kNativeWidth * kNativeHeight];
static auto g_frame_counter = 0;

static std::atomic_bool g_sync_to_audio = true;
static int g_cycles_per_audio_frame = 0;

static auto g_keyboard_input_device = nba::BasicInputDevice{};
static auto g_controller_input_device = nba::BasicInputDevice{};
static SDL_GameController* g_game_controller = nullptr;
static auto g_game_controller_button_x_old = false;
static auto g_fastforward = false;

static auto g_config = std::make_shared<nba::Config>();
static auto g_emulator = std::make_unique<nba::Emulator>(g_config);
static auto g_emulator_lock = std::mutex{};

struct KeyMap {
  SDL_Keycode fastforward = SDLK_SPACE;
  SDL_Keycode reset = SDLK_F9;
  SDL_Keycode fullscreen = SDLK_F10;
  std::unordered_map<SDL_Keycode, nba::InputDevice::Key> gba;
} keymap;

struct CombinedInputDevice : public nba::InputDevice {
  auto Poll(Key key) -> bool final {
    return g_keyboard_input_device.Poll(key) || g_controller_input_device.Poll(key);
  }
};

struct SDL2_VideoDevice : public nba::VideoDevice {
  void Draw(std::uint32_t* buffer) final {
    std::memcpy(g_framebuffer, buffer, sizeof(std::uint32_t) * kNativeWidth * kNativeHeight);
    g_frame_counter++;
  }
};

void load_game(std::string const& rom_path);
void update_fullscreen();
void update_viewport();
void update_key(SDL_KeyboardEvent* event);
void update_controller();
void audio_passthrough(SDL2_AudioDevice* audio_device, std::int16_t* stream, int byte_len);

void usage(char* app_name) {
  fmt::print("Usage: {0} [--bios bios_path] [--force-rtc] [--save-type type] [--fullscreen] [--scale factor] [--resampler type] [--sync-to-audio yes/no] rom_path\n", app_name);
  std::exit(-1);
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
    } else if (key == "--sync-to-audio") {
      if (i == limit) {
        usage(argv[0]);
      }
      auto value = std::string{argv[i++]};
      if (value == "yes") {
        g_config->sync_to_audio = true;
      } else if (value == "no") {
        g_config->sync_to_audio = false;
      } else {
        usage(argv[0]);
      }
    } else if (key == "--force-rtc") {
      g_config->force_rtc = true;
    } else if (key == "--save-type") {
      /* TODO: deduplicate this piece of code? */
      const std::unordered_map<std::string, nba::Config::BackupType> save_types{
        { "detect",     nba::Config::BackupType::Detect    },
        { "none",       nba::Config::BackupType::None      },
        { "sram",       nba::Config::BackupType::SRAM      },
        { "flash64",    nba::Config::BackupType::FLASH_64  },
        { "flash128",   nba::Config::BackupType::FLASH_128 },
        { "eeprom512",  nba::Config::BackupType::EEPROM_4  },
        { "eeprom8192", nba::Config::BackupType::EEPROM_64 }
      };
      if (i == limit) {
        usage(argv[0]);
      }
      auto match = save_types.find(argv[i++]);
      if (match != save_types.end()) {
        g_config->backup_type = match->second;
      } else {
        fmt::print("Bad save type, refer to config.toml for documentation.\n\n");
        usage(argv[0]);
      }
    } else if (key == "--resampler") {
      /* TODO: deduplicate this piece of code. */
      const std::unordered_map<std::string, nba::Config::Audio::Interpolation> resamplers{
        { "cosine",  nba::Config::Audio::Interpolation::Cosine   },
        { "cubic",   nba::Config::Audio::Interpolation::Cubic    },
        { "sinc64",  nba::Config::Audio::Interpolation::Sinc_64  },
        { "sinc128", nba::Config::Audio::Interpolation::Sinc_128 },
        { "sinc256", nba::Config::Audio::Interpolation::Sinc_256 }
      };
      if (i == limit) {
        usage(argv[0]);
      }
      auto match = resamplers.find(argv[i++]);
      if (match != resamplers.end()) {
        g_config->audio.interpolation = match->second;
      } else {
        fmt::print("Bad resampler type, refer to config.toml for documentation.\n\n");
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

void load_keymap() {
  toml::value data;

  try {
    data = toml::parse("keymap.toml");
  } catch (std::exception& ex) {
    LOG_WARN("Failed to load or parse keymap configuration.");
    return;
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

auto load_as_string(std::string const& path) -> std::optional<std::string> {
  std::string ret;
  std::ifstream file { path };
  if (!file.good()) {
    return {};
  }
  file.seekg(0, std::ios::end);
  ret.reserve(file.tellg());
  file.seekg(0);
  ret.assign(std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{});
  return ret;
}

auto compile_shader(GLuint shader, const char* source) -> bool {
  GLint compiled = 0;
  const char* source_array[] = { source };
  glShaderSource(shader, 1, source_array, nullptr);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if(compiled == GL_FALSE) {
    GLint max_length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

    auto error_log = std::make_unique<GLchar[]>(max_length);
    glGetShaderInfoLog(shader, max_length, &max_length, error_log.get());
    LOG_ERROR("Failed to compile shader:\n{0}", error_log.get());
    glDeleteShader(shader);
    return false;
  }

  return true;
}

void init(int argc, char** argv) {
  common::logger::init();
  config_toml_read(*g_config, "config.toml");
  parse_arguments(argc, argv);
  load_keymap();
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);
  g_window = SDL_CreateWindow("NanoboyAdvance",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    kNativeWidth * g_config->video.scale,
    kNativeHeight * g_config->video.scale,
    SDL_WINDOW_OPENGL);
  g_gl_context = SDL_GL_CreateContext(g_window);
  #ifndef __APPLE__
  glewExperimental = GL_TRUE;
  glewInit();
  #endif
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetSwapInterval(1);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &g_gl_texture);
  glBindTexture(GL_TEXTURE_2D, g_gl_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  if (!g_config->video.shader.path_vs.empty() && !g_config->video.shader.path_fs.empty()) {
    auto vert_src = load_as_string(g_config->video.shader.path_vs);
    auto frag_src = load_as_string(g_config->video.shader.path_fs);
    if (vert_src.has_value() && frag_src.has_value()) {
      auto vid = glCreateShader(GL_VERTEX_SHADER);
      auto fid = glCreateShader(GL_FRAGMENT_SHADER);
      if (compile_shader(vid, vert_src.value().c_str()) && compile_shader(fid, frag_src.value().c_str())) {
        auto pid = glCreateProgram();
        glAttachShader(pid, vid);
        glAttachShader(pid, fid);
        glLinkProgram(pid);
        glUseProgram(pid);
      }
    }
  }
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  g_sync_to_audio = g_config->sync_to_audio;
  update_fullscreen();
  update_viewport();
  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (SDL_IsGameController(i)) {
      g_game_controller = SDL_GameControllerOpen(i);
      if (g_game_controller != nullptr) {
        LOG_INFO("Detected game controller '{0}'", SDL_GameControllerNameForIndex(i));
        break;
      }
    }
  }
  auto audio_device = std::make_shared<SDL2_AudioDevice>();
  audio_device->SetPassthrough((SDL_AudioCallback)audio_passthrough);
  g_config->audio_dev = audio_device;
  g_config->input_dev = std::make_shared<CombinedInputDevice>();
  g_config->video_dev = std::make_shared<SDL2_VideoDevice>();
  g_emulator->Reset();
  g_cycles_per_audio_frame = 16777216ULL * audio_device->GetBlockSize() / audio_device->GetSampleRate();
}

void loop() {
  auto event = SDL_Event{};

  auto ticks_start = SDL_GetTicks();

  for (;;) {
    update_controller();
    if (!g_sync_to_audio) {
      g_emulator_lock.lock();
      g_emulator->Frame();
      g_emulator_lock.unlock();
    }
    update_viewport();
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, g_gl_texture);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      kNativeWidth,
      kNativeHeight,
      0,
      GL_BGRA,
      GL_UNSIGNED_BYTE,
      g_framebuffer
    );
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(-1.0f, 1.0f);
    glTexCoord2f(1.0f, 0);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);
    glTexCoord2f(0, 1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
    SDL_GL_SwapWindow(g_window);
    auto ticks_end = SDL_GetTicks();
    if ((ticks_end - ticks_start) >= 1000) {
      auto title = fmt::format("NanoboyAdvance [{0} fps | {1}%]", g_frame_counter, int(g_frame_counter / 60.0 * 100.0));
      SDL_SetWindowTitle(g_window, title.c_str());
      g_frame_counter = 0;
      ticks_start = ticks_end;
    }
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        return;
      if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
        update_key(reinterpret_cast<SDL_KeyboardEvent*>(&event));
    }
  }
}

void destroy() {
  // Make sure that the audio thread no longer accesses the emulator.
  g_emulator_lock.lock();
  if (g_game_controller != nullptr) {
    SDL_GameControllerClose(g_game_controller);
  }
  SDL_GL_DeleteContext(g_gl_context);
  SDL_DestroyWindow(g_window);
  SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);
}

void audio_passthrough(SDL2_AudioDevice* audio_device, std::int16_t* stream, int byte_len) {
  if (g_sync_to_audio) {
    g_emulator_lock.lock();
    g_emulator->Run(g_cycles_per_audio_frame);
    g_emulator_lock.unlock();
  }
  audio_device->InvokeCallback(stream, byte_len);
}

void update_fullscreen() {
  SDL_SetWindowFullscreen(g_window, g_config->video.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void update_viewport() {
  int width;
  int height;
  SDL_GL_GetDrawableSize(g_window, &width, &height);

  int viewport_width = height + height / 2;
  glViewport((width - viewport_width) / 2, 0, viewport_width, height);
}

void update_fastforward(bool fastforward) {
  g_fastforward = fastforward;
  g_sync_to_audio = !fastforward && g_config->sync_to_audio;
  if (fastforward) {
    SDL_GL_SetSwapInterval(0);
  } else {
    SDL_GL_SetSwapInterval(1);
  }
}

void update_key(SDL_KeyboardEvent* event) {
  bool pressed = event->type == SDL_KEYDOWN;
  auto key = event->keysym.sym;

  if (key == keymap.fastforward) {
    update_fastforward(pressed);
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
    g_keyboard_input_device.SetKeyStatus(match->second, pressed);
  }
}

void update_controller() {
  if (g_game_controller == nullptr)
    return;
  SDL_GameControllerUpdate();

  auto button_x = SDL_GameControllerGetButton(g_game_controller, SDL_CONTROLLER_BUTTON_X);
  if (g_game_controller_button_x_old && !button_x) {
    update_fastforward(!g_fastforward);
  }
  g_game_controller_button_x_old = button_x;

  static const std::unordered_map<SDL_GameControllerButton, nba::InputDevice::Key> buttons{
    { SDL_CONTROLLER_BUTTON_A, nba::InputDevice::Key::A },
    { SDL_CONTROLLER_BUTTON_B, nba::InputDevice::Key::B },
    { SDL_CONTROLLER_BUTTON_LEFTSHOULDER , nba::InputDevice::Key::L },
    { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, nba::InputDevice::Key::R },
    { SDL_CONTROLLER_BUTTON_START, nba::InputDevice::Key::Start },
    { SDL_CONTROLLER_BUTTON_BACK, nba::InputDevice::Key::Select }
  };

  for (auto& button : buttons) {
    if (SDL_GameControllerGetButton(g_game_controller, button.first)) {
      g_controller_input_device.SetKeyStatus(button.second, true);
    } else {
      g_controller_input_device.SetKeyStatus(button.second, false);
    }
  }

  constexpr auto threshold = std::numeric_limits<int16_t>::max() / 2;
  auto x = SDL_GameControllerGetAxis(g_game_controller, SDL_CONTROLLER_AXIS_LEFTX);
  auto y = SDL_GameControllerGetAxis(g_game_controller, SDL_CONTROLLER_AXIS_LEFTY);

  g_controller_input_device.SetKeyStatus(nba::InputDevice::Key::Left, x < -threshold);
  g_controller_input_device.SetKeyStatus(nba::InputDevice::Key::Right, x > threshold);
  g_controller_input_device.SetKeyStatus(nba::InputDevice::Key::Up, y < -threshold);
  g_controller_input_device.SetKeyStatus(nba::InputDevice::Key::Down, y > threshold);
}

int main(int argc, char** argv) {
  init(argc, argv);
  loop();
  destroy();
  return 0;
}
