/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "main_window.hpp"
#include "controller_manager.hpp"

ControllerManager::~ControllerManager() {
  if (timer) {
    timer->stop();
    Close();
  } else {
    quitting = true;
    thread.join();
  }
}

void ControllerManager::Initialize() {
  SDL_Init(SDL_INIT_GAMECONTROLLER);

  /* On macOS we may not poll SDL events on a separate thread.
   * So what we do instead is handle device connect/remove events
   * from a 100 ms Qt timer and updating the input from the emulator thread each frame.
   */
#if defined(__APPLE__)
  timer = new QTimer{this};
  timer->start(100);
  connect(timer, &QTimer::timeout, std::bind(&ControllerManager::Process, this));
  emu_thread->SetPerFrameCallback(std::bind(&ControllerManager::UpdateKeyState, this));
#else
  thread = std::thread{[this]() {
    // SDL_WaitEventTimeout() requires video to be initialised on the same thread.
    SDL_Init(SDL_INIT_VIDEO);

    while (!quitting) {
      SDL_WaitEventTimeout(nullptr, 100);

      ProcessEvents();
      UpdateKeyState();
    }

    Close();
  }};
#endif
}

void ControllerManager::Open(std::string const& guid) {
  auto joystick_count = SDL_NumJoysticks();

  Close();

  for (int device_id = 0; device_id < joystick_count; device_id++) {
    if (SDL_IsGameController(device_id) &&
        GetControllerGUIDStringFromIndex(device_id) == guid
    ) {
      controller = SDL_GameControllerOpen(device_id);
      instance_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
      break;
    }
  }
}

void ControllerManager::Close() {
  using Key = nba::InputDevice::Key;

  // Unset all keys in case any key is currently pressed.
  main_window->SetKeyStatus(1, Key::Up, false);
  main_window->SetKeyStatus(1, Key::Down, false);
  main_window->SetKeyStatus(1, Key::Left, false);
  main_window->SetKeyStatus(1, Key::Right, false);
  main_window->SetKeyStatus(1, Key::Start, false);
  main_window->SetKeyStatus(1, Key::Select, false);
  main_window->SetKeyStatus(1, Key::A, false);
  main_window->SetKeyStatus(1, Key::B, false);
  main_window->SetKeyStatus(1, Key::L, false);
  main_window->SetKeyStatus(1, Key::R, false);

  if (controller) {
    SDL_GameControllerClose(controller);
    controller = nullptr;
  }
}

void ControllerManager::ProcessEvents() {
  auto event = SDL_Event{};
  auto input_window = main_window->input_window;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_JOYDEVICEADDED: {
        if (input_window) {
          input_window->UpdateGameControllerList();
        }

        auto device_id = ((SDL_JoyDeviceEvent*)&event)->which;
        auto guid = GetControllerGUIDStringFromIndex(device_id);

        if (guid == config->input.controller_guid) {
          Open(guid);
        }
        break;
      }
      case SDL_JOYDEVICEREMOVED: {
        if (input_window) {
          input_window->UpdateGameControllerList();
        }

        if (instance_id == ((SDL_JoyDeviceEvent*)&event)->which) {
          Close();
        }
        break;
      }
      case SDL_CONTROLLERBUTTONUP: {
        auto button_event = (SDL_ControllerButtonEvent*)&event;

        input_window->OnControllerButtonUp((SDL_GameControllerButton)button_event->button);
        break;
      }
      case SDL_CONTROLLERAXISMOTION: {
        const auto threshold = std::numeric_limits<int16_t>::max() / 2;

        auto axis_event = (SDL_ControllerAxisEvent*)&event;
        auto value = axis_event->value;

        if (std::abs(value) > threshold) {
          input_window->OnControllerAxisMove((SDL_GameControllerAxis)axis_event->axis, value < 0);
        }
        break;
      }
    }
  }

  if (input_window->has_game_controller_choice_changed) {
    Open(config->input.controller_guid);

    input_window->has_game_controller_choice_changed = false;
  }
}

void ControllerManager::UpdateKeyState() {
  using Key = nba::InputDevice::Key;

  if (controller == nullptr) {
    return;
  }

  auto const& input = config->input;

  const auto evaluate = [&](QtConfig::Input::Map const& mapping) {
    bool pressed = false;
  
    auto button = mapping.controller.button;
    auto axis = mapping.controller.axis;

    if (button != SDL_CONTROLLER_BUTTON_INVALID) {
      pressed = pressed || SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)button);
    }

    if (axis != SDL_CONTROLLER_AXIS_INVALID) {
      const auto threshold = std::numeric_limits<int16_t>::max() / 2;

      auto actual_axis = (SDL_GameControllerAxis)(axis & ~0x80);
      bool negative = axis & 0x80;
      auto value = SDL_GameControllerGetAxis(controller, actual_axis);

      pressed = pressed || (negative ? (value < -threshold) : (value > threshold));
    }

    return pressed;
  };

  for (int i = 0; i < nba::InputDevice::kKeyCount; i++) {
    main_window->SetKeyStatus(
      1, static_cast<nba::InputDevice::Key>(i), evaluate(input.gba[i]));
  }

  bool fast_forward_button = evaluate(input.fast_forward);

  if (input.hold_fast_forward) {
    main_window->emu_thread->SetFastForward(fast_forward_button);
  } else if (!fast_forward_button && fast_forward_button_old) {
    main_window->emu_thread->SetFastForward(
      !main_window->emu_thread->GetFastForward());
  }

  fast_forward_button_old = fast_forward_button;
}