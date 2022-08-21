#include <SDL.h>

// #include <EGL/egl.h>  // EGL library
// #include <EGL/eglext.h> // EGL extensions
// #include <glad/glad.h>  // glad library (OpenGL loader)

#include <nba/core.hpp>
#include <platform/device/ogl_video_device.hpp>
#include <platform/device/sdl_audio_device.hpp>
#include <platform/loader/bios.hpp>
#include <platform/loader/rom.hpp>
#include <platform/config.hpp>

// For input code:
#include <unordered_map>
#include <limits>

#include <switch.h>

extern "C" {
#include <unistd.h> // for close(socket)

static int nxlink_socket = 0;

void userAppInit() {
  appletLockExit();

  // TODO: disable nxlink stdio in release builds
  socketInitializeDefault();
  nxlink_socket = nxlinkStdio();
}

void userAppExit() {
  close(nxlink_socket);
  socketExit();

  appletUnlockExit();
}

} // extern "C"

int main() {
  unsigned int width = 1280;
  unsigned int height = 720;

  // create window
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_Window* window = SDL_CreateWindow("NanoBoyAdvance", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
  SDL_GLContext context = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1);

  gladLoadGL(); // TODO: not sure where this fits

  auto config = std::make_shared<nba::PlatformConfig>();

  auto audio_dev = std::make_shared<nba::SDL2_AudioDevice>();
  auto input_dev = std::make_shared<nba::BasicInputDevice>();

  auto video_dev = std::make_shared<nba::OGLVideoDevice>(config);
  video_dev->Initialize();
  video_dev->SetViewport(100, 0, 1080, 720); // TODO

  config->audio_dev = audio_dev;
  config->input_dev = input_dev;
  config->video_dev = video_dev;

  auto core = nba::CreateCore(config);

  // TODO: handle failure
  nba::BIOSLoader::Load(core, "sdmc:/bios.bin");
  nba::ROMLoader::Load(core, "sdmc:/pokeemerald.gba", nba::Config::BackupType::Detect, nba::GPIODeviceType::RTC);

  // I'm not sure if this is necessary
  core->Reset();

  auto event = SDL_Event{};

  // TODO: proper error handling
  SDL_GameController* game_controller = nullptr;
  if (SDL_IsGameController(0)) {
    game_controller = SDL_GameControllerOpen(0);
  }

  while(appletMainLoop()) {
    while(SDL_PollEvent(&event)){
      if(event.type == SDL_QUIT) {
        goto done;
      }
    }

    // TODO: we might as well use events?
    if (game_controller) {
      static const std::unordered_map<SDL_GameControllerButton, nba::InputDevice::Key> buttons{
        { SDL_CONTROLLER_BUTTON_A, nba::InputDevice::Key::A },
        { SDL_CONTROLLER_BUTTON_B, nba::InputDevice::Key::B },
        { SDL_CONTROLLER_BUTTON_LEFTSHOULDER , nba::InputDevice::Key::L },
        { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, nba::InputDevice::Key::R },
        { SDL_CONTROLLER_BUTTON_START, nba::InputDevice::Key::Start },
        { SDL_CONTROLLER_BUTTON_BACK, nba::InputDevice::Key::Select }
      };

      for (auto& button : buttons) {
        input_dev->SetKeyStatus(button.second,
          SDL_GameControllerGetButton(game_controller, button.first));
      }

      constexpr auto threshold = std::numeric_limits<int16_t>::max() / 2;
      auto x = SDL_GameControllerGetAxis(game_controller, SDL_CONTROLLER_AXIS_LEFTX);
      auto y = SDL_GameControllerGetAxis(game_controller, SDL_CONTROLLER_AXIS_LEFTY);

      input_dev->SetKeyStatus(nba::InputDevice::Key::Left,  x < -threshold);
      input_dev->SetKeyStatus(nba::InputDevice::Key::Right, x >  threshold);
      input_dev->SetKeyStatus(nba::InputDevice::Key::Up,    y < -threshold);
      input_dev->SetKeyStatus(nba::InputDevice::Key::Down,  y >  threshold);
    }

    core->Run(280896);

    SDL_GL_SwapWindow(window);
  }
  
done:
  // TODO: this is unsafe, devices will be destroyed after main exits.
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
