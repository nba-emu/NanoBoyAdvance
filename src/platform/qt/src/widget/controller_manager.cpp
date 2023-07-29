/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "main_window.hpp"
#include "controller_manager.hpp"

ControllerManager::~ControllerManager() {
  if(timer) {
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
  connect(timer, &QTimer::timeout, std::bind(&ControllerManager::ProcessEvents, this));
  main_window->emu_thread->SetPerFrameCallback(
    std::bind(&ControllerManager::UpdateKeyState, this));
#else
  thread = std::thread{[this]() {
    // SDL_WaitEventTimeout() requires video to be initialised on the same thread.
    SDL_Init(SDL_INIT_VIDEO);

    while(!quitting) {
      SDL_WaitEventTimeout(nullptr, 100);

      ProcessEvents();
      UpdateKeyState();
    }

    Close();
  }};
#endif

  connect(
    this, &ControllerManager::OnJoystickListChanged,
    this, &ControllerManager::UpdateJoystickList,
    Qt::QueuedConnection
  );

  connect(
    this, &ControllerManager::OnJoystickButtonReleased,
    this, &ControllerManager::BindCurrentKeyToJoystickButton,
    Qt::QueuedConnection
  );

  connect(
    this, &ControllerManager::OnJoystickAxisMoved,
    this, &ControllerManager::BindCurrentKeyToJoystickAxis,
    Qt::QueuedConnection
  );
}

void ControllerManager::UpdateJoystickList() {
  auto input_window = main_window->input_window;

  if(input_window) {
    input_window->UpdateJoystickList();
  }
}

void ControllerManager::BindCurrentKeyToJoystickButton(int button) {
  auto input_window = main_window->input_window;

  if(input_window) {
    input_window->BindCurrentKeyToJoystickButton(button);
  }
}

void ControllerManager::BindCurrentKeyToJoystickAxis(int axis, bool negative) {
  auto input_window = main_window->input_window;

  if(input_window) {
    input_window->BindCurrentKeyToJoystickAxis(axis, negative);
  }
}

void ControllerManager::Open(std::string const& guid) {
  auto joystick_count = SDL_NumJoysticks();

  Close();

  for(int device_id = 0; device_id < joystick_count; device_id++) {
    if(GetJoystickGUIDStringFromIndex(device_id) == guid) {
      joystick = SDL_JoystickOpen(device_id);
      instance_id = SDL_JoystickInstanceID(joystick);
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

  if(joystick) {
    SDL_JoystickClose(joystick);
    joystick = nullptr;
  }
}

void ControllerManager::ProcessEvents() {
  auto event = SDL_Event{};
  auto input_window = main_window->input_window;

#if defined(__APPLE__)
  std::lock_guard guard{lock};
#endif

  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_JOYDEVICEADDED: {
        emit OnJoystickListChanged();

        auto device_id = ((SDL_JoyDeviceEvent*)&event)->which;
        auto guid = GetJoystickGUIDStringFromIndex(device_id);

        if(guid == config->input.controller_guid) {
          Open(guid);
        }
        break;
      }
      case SDL_JOYDEVICEREMOVED: {
        emit OnJoystickListChanged();

        if(instance_id == ((SDL_JoyDeviceEvent*)&event)->which) {
          Close();
        }
        break;
      }
      case SDL_JOYBUTTONUP: {
        auto button_event = (SDL_JoyButtonEvent*)&event;

        emit OnJoystickButtonReleased(button_event->button);
        break;
      }
      case SDL_JOYAXISMOTION: {
        const auto threshold = std::numeric_limits<int16_t>::max() / 2;

        auto axis_event = (SDL_JoyAxisEvent*)&event;
        auto value = axis_event->value;

        if(std::abs(value) > threshold) {
          emit OnJoystickAxisMoved(axis_event->axis, value < 0);
        }
        break;
      }
    }
  }

  if(input_window && input_window->has_game_controller_choice_changed) {
    Open(config->input.controller_guid);

    input_window->has_game_controller_choice_changed = false;
  }
}

void ControllerManager::UpdateKeyState() {
  using Key = nba::InputDevice::Key;

#if defined(__APPLE__)
  std::lock_guard guard{lock};
#endif

  if(joystick == nullptr) {
    return;
  }

  auto const& input = config->input;

  const auto evaluate = [&](QtConfig::Input::Map const& mapping) {
    bool pressed = false;
  
    auto button = mapping.controller.button;
    auto axis = mapping.controller.axis;

    if(button != SDL_CONTROLLER_BUTTON_INVALID) {
      pressed = pressed || SDL_JoystickGetButton(joystick, button);
    }

    if(axis != SDL_CONTROLLER_AXIS_INVALID) {
      const auto threshold = std::numeric_limits<int16_t>::max() / 2;

      // @todo: replace the 0x80/negative flag with something more sensible
      // @todo: evaluate if the axis system needs further adjustments.
      auto actual_axis = axis & ~0x80;
      bool negative = axis & 0x80;
      auto value = SDL_JoystickGetAxis(joystick, actual_axis);

      pressed = pressed || (negative ? (value < -threshold) : (value > threshold));
    }

    return pressed;
  };

  for(int i = 0; i < nba::InputDevice::kKeyCount; i++) {
    main_window->SetKeyStatus(
      1, static_cast<nba::InputDevice::Key>(i), evaluate(input.gba[i]));
  }

  bool fast_forward_button = evaluate(input.fast_forward);

  if(fast_forward_button != fast_forward_button_old) {
    main_window->SetFastForward(1, fast_forward_button);
    fast_forward_button_old = fast_forward_button;
  }
}