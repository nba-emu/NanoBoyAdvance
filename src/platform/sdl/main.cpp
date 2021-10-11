/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */


#include <nba/core.hpp>
#include <platform/device/sdl_audio_device.hpp>
#include <platform/loader/bios.hpp>
#include <platform/loader/rom.hpp>
#include <platform/config.hpp>

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fmt/format.h>
#include <iterator>
#include <optional>
#include <string>
#include <toml.hpp>
#include <unordered_map>
#include <mutex>

#include <GL/glew.h>

#undef main

using namespace nba;

static constexpr auto kNativeWidth = 240;
static constexpr auto kNativeHeight = 160;

static SDL_Window* g_window;
static SDL_GLContext g_gl_context;
static GLuint g_gl_texture;
static u32 g_framebuffer[kNativeWidth * kNativeHeight];
static auto g_frame_counter = 0;
static auto g_swap_interval = 1;

static std::atomic_bool g_sync_to_audio = true;
static int g_cycles_per_audio_frame = 0;

static auto g_keyboard_input_device = BasicInputDevice{};
static auto g_controller_input_device = BasicInputDevice{};
static SDL_GameController* g_game_controller = nullptr;
static auto g_game_controller_button_x_old = false;
static auto g_fastforward = false;

static auto g_config = std::make_shared<PlatformConfig>();
static auto g_core = nba::CreateCore(g_config);
static auto g_core_lock = std::mutex{};

struct KeyMap {
  SDL_Keycode fastforward = SDLK_SPACE;
  SDL_Keycode reset = SDLK_F9;
  SDL_Keycode fullscreen = SDLK_F10;
  std::unordered_map<SDL_Keycode, InputDevice::Key> gba;
} keymap;

struct CombinedInputDevice : public InputDevice {
  auto Poll(Key key) -> bool final {
    return g_keyboard_input_device.Poll(key) || g_controller_input_device.Poll(key);
  }

  void SetOnChangeCallback(std::function<void(void)> callback) {
    g_keyboard_input_device.SetOnChangeCallback(callback);
    g_controller_input_device.SetOnChangeCallback(callback);
  }
};

struct SDL2_VideoDevice : public VideoDevice {
  void Draw(u32* buffer) final {
    std::memcpy(g_framebuffer, buffer, sizeof(u32) * kNativeWidth * kNativeHeight);
    g_frame_counter++;
  }
};

void load_game(std::string const& rom_path);
void update_fullscreen();
void update_viewport();
void update_key(SDL_KeyboardEvent* event);
void update_controller();
void audio_passthrough(SDL2_AudioDevice* audio_device, s16* stream, int byte_len);

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
      const std::unordered_map<std::string, Config::BackupType> save_types{
        { "detect",     Config::BackupType::Detect    },
        { "none",       Config::BackupType::None      },
        { "sram",       Config::BackupType::SRAM      },
        { "flash64",    Config::BackupType::FLASH_64  },
        { "flash128",   Config::BackupType::FLASH_128 },
        { "eeprom512",  Config::BackupType::EEPROM_4  },
        { "eeprom8192", Config::BackupType::EEPROM_64 }
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
      const std::unordered_map<std::string, Config::Audio::Interpolation> resamplers{
        { "cosine",  Config::Audio::Interpolation::Cosine   },
        { "cubic",   Config::Audio::Interpolation::Cubic    },
        { "sinc64",  Config::Audio::Interpolation::Sinc_64  },
        { "sinc128", Config::Audio::Interpolation::Sinc_128 },
        { "sinc256", Config::Audio::Interpolation::Sinc_256 }
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
  auto& bios_path = g_config->bios_path;

  switch (nba::BIOSLoader::Load(g_core, g_config->bios_path)) {
    case nba::BIOSLoader::Result::CannotFindFile:
    case nba::BIOSLoader::Result::CannotOpenFile: {
      fmt::print("Cannot open BIOS: {}\n", bios_path);
      std::exit(-1);
      break;
    }
    case nba::BIOSLoader::Result::BadImage: {
      fmt::print("Bad BIOS image: {}\n", bios_path);
      std::exit(-1);
      break;
    }
  }

  switch (nba::ROMLoader::Load(g_core, rom_path, g_config->backup_type, g_config->force_rtc)) {
    case nba::ROMLoader::Result::CannotFindFile:
    case nba::ROMLoader::Result::CannotOpenFile: {
      fmt::print("Cannot open ROM: {}\n", rom_path);
      std::exit(-1);
      break;
    }
    case nba::ROMLoader::Result::BadImage: {
      fmt::print("Bad ROM image: {}\n", rom_path);
      std::exit(-1);
      break;
    }
  }
}

void load_keymap() {
  toml::value data;

  try {
    data = toml::parse("keymap.toml");
  } catch (std::exception& ex) {
    Log<Warn>("SDL: failed to load keymap configuration.");
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
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "a", "A").c_str())] = InputDevice::Key::A;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "b", "B").c_str())] = InputDevice::Key::B;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "l", "D").c_str())] = InputDevice::Key::L;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "r", "F").c_str())] = InputDevice::Key::R;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "start", "Return").c_str())] = InputDevice::Key::Start;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "select", "Backspace").c_str())] = InputDevice::Key::Select;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "up", "Up").c_str())] = InputDevice::Key::Up;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "down", "Down").c_str())] = InputDevice::Key::Down;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "left", "Left").c_str())] = InputDevice::Key::Left;
      keymap.gba[SDL_GetKeyFromName(toml::find_or<std::string>(gba, "right", "Right").c_str())] = InputDevice::Key::Right;
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
    Log<Error>("SDL: OpenGL: failed to compile shader:\n{0}", error_log.get());
    glDeleteShader(shader);
    return false;
  }

  return true;
}

void init(int argc, char** argv) {
  namespace fs = std::filesystem;
  if (argc >= 1) {
    fs::current_path(fs::absolute(argv[0]).replace_filename(fs::path{ }));
  }
  g_config->Load("config.toml");
  parse_arguments(argc, argv);
  load_keymap();
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);
  g_window = SDL_CreateWindow("NanoBoyAdvance",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    kNativeWidth * g_config->video.scale,
    kNativeHeight * g_config->video.scale,
    SDL_WINDOW_OPENGL);
  g_gl_context = SDL_GL_CreateContext(g_window);
  glewInit();
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_DisplayMode mode;
  SDL_GetCurrentDisplayMode(0, &mode);
  if (mode.refresh_rate % 60 == 0) {
    g_swap_interval = mode.refresh_rate / 60;
  }
  SDL_GL_SetSwapInterval(g_swap_interval);

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
        Log<Info>("SDL: detected game controller '{0}'", SDL_GameControllerNameForIndex(i));
        break;
      }
    }
  }
  auto audio_device = std::make_shared<nba::SDL2_AudioDevice>();
  audio_device->SetPassthrough((SDL_AudioCallback)audio_passthrough);
  g_config->audio_dev = audio_device;
  g_config->input_dev = std::make_shared<CombinedInputDevice>();
  g_config->video_dev = std::make_shared<SDL2_VideoDevice>();
  g_core->Reset();
  g_cycles_per_audio_frame = 16777216ULL * audio_device->GetBlockSize() / audio_device->GetSampleRate();
}

void loop() {
  auto event = SDL_Event{};

  auto ticks_start = SDL_GetTicks();

  for (;;) {
    update_controller();
    if (!g_sync_to_audio) {
      g_core_lock.lock();
      g_core->RunForOneFrame();
      g_core_lock.unlock();
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
      auto title = fmt::format("NanoBoyAdvance [{0} fps | {1}%]", g_frame_counter, int(g_frame_counter / 60.0 * 100.0));
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
  g_core_lock.lock();
  if (g_game_controller != nullptr) {
    SDL_GameControllerClose(g_game_controller);
  }
  SDL_GL_DeleteContext(g_gl_context);
  SDL_DestroyWindow(g_window);
  SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);
}

void audio_passthrough(SDL2_AudioDevice* audio_device, s16* stream, int byte_len) {
  if (g_sync_to_audio) {
    g_core_lock.lock();
    g_core->Run(g_cycles_per_audio_frame);
    g_core_lock.unlock();
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
    SDL_GL_SetSwapInterval(g_swap_interval);
  }
}

void update_key(SDL_KeyboardEvent* event) {
  bool pressed = event->type == SDL_KEYDOWN;
  auto key = event->keysym.sym;

  if (key == keymap.fastforward) {
    update_fastforward(pressed);
  }

  if (key == keymap.reset && !pressed) {
    g_core_lock.lock();
    g_core->Reset();
    g_core_lock.unlock();
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

  static const std::unordered_map<SDL_GameControllerButton, InputDevice::Key> buttons{
    { SDL_CONTROLLER_BUTTON_A, InputDevice::Key::A },
    { SDL_CONTROLLER_BUTTON_B, InputDevice::Key::B },
    { SDL_CONTROLLER_BUTTON_LEFTSHOULDER , InputDevice::Key::L },
    { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, InputDevice::Key::R },
    { SDL_CONTROLLER_BUTTON_START, InputDevice::Key::Start },
    { SDL_CONTROLLER_BUTTON_BACK, InputDevice::Key::Select }
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

  g_controller_input_device.SetKeyStatus(InputDevice::Key::Left, x < -threshold);
  g_controller_input_device.SetKeyStatus(InputDevice::Key::Right, x > threshold);
  g_controller_input_device.SetKeyStatus(InputDevice::Key::Up, y < -threshold);
  g_controller_input_device.SetKeyStatus(InputDevice::Key::Down, y > threshold);
}

int main(int argc, char** argv) {
  init(argc, argv);
  loop();
  destroy();
  return 0;
}
