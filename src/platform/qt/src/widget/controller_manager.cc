// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <atom/logger/logger.hh>

#include "main_window.hh"
#include "controller_manager.hh"

std::string GetJoystickGUIDStringFromIndex(int device_index) {
  auto guid = SDL_GetJoystickGUIDForID(device_index);
  auto guid_string = std::string{};

  guid_string.resize(sizeof(SDL_GUID) * 2);
  SDL_GUIDToString(guid, guid_string.data(), guid_string.size() + 1);
  return guid_string;
}

ControllerManager::ControllerManager(MainWindow* main_window, std::shared_ptr<QtConfig> config) : m_main_window(main_window), m_config(config) { }

ControllerManager::~ControllerManager() {
  if(m_timer) {
    m_timer->stop();
    Close();
  } else {
    m_is_quitting = true;
    m_thread.join();
  }
}

void ControllerManager::Initialize() {
  SDL_Init(SDL_INIT_JOYSTICK);

  /* On macOS we may not poll SDL events on a separate thread.
   * So what we do instead is handle device connect/remove events
   * from a 16 ms Qt timer and updating the input from the emulator thread each frame.
   */
#if defined(__APPLE__)
  m_timer = new QTimer{this};
  m_timer->start(16);
  connect(m_timer, &QTimer::timeout, std::bind(&ControllerManager::ProcessEvents, this));
  m_main_window->emu_thread->SetPerFrameCallback(std::bind(&ControllerManager::UpdateKeyState, this));
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

  connect(
    this, &ControllerManager::OnJoystickHatMoved,
    this, &ControllerManager::BindCurrentKeyToJoystickHat,
    Qt::QueuedConnection
  );
}

void ControllerManager::UpdateJoystickList() {
  auto input_window = m_main_window->input_window;

  if(input_window) {
    input_window->UpdateJoystickList();
  }
}

void ControllerManager::BindCurrentKeyToJoystickButton(int button) {
  auto input_window = m_main_window->input_window;

  if(input_window) {
    input_window->BindCurrentKeyToJoystickButton(button);
  }
}

void ControllerManager::BindCurrentKeyToJoystickAxis(int axis, bool negative) {
  auto input_window = m_main_window->input_window;

  if(input_window) {
    input_window->BindCurrentKeyToJoystickAxis(axis, negative);
  }
}

void ControllerManager::BindCurrentKeyToJoystickHat(int hat, int direction) {
  auto input_window = m_main_window->input_window;

  if(input_window) {
    input_window->BindCurrentKeyToJoystickHat(hat, direction);
  }
}

void ControllerManager::Open(std::string const& guid) {
  auto joystick_count = 0;
  auto ids = SDL_GetJoysticks(&joystick_count);
  if (ids == nullptr) {
    ATOM_ERROR("SDL_GetJoysticks failed: \"{}\"", SDL_GetError());
  }

  Close();

  for(int i = 0; i < joystick_count; i++) {
    auto id = ids[i];
    if(GetJoystickGUIDStringFromIndex(id) == guid) {
      m_joystick = SDL_OpenJoystick(id);
      m_instance_id = SDL_GetJoystickID(m_joystick);
      break;
    }
  }
}

void ControllerManager::Close() {
  using Key = nba::Key;

  // Unset all keys in case any key is currently pressed.
  m_main_window->SetKeyStatus(1, Key::Up, false);
  m_main_window->SetKeyStatus(1, Key::Down, false);
  m_main_window->SetKeyStatus(1, Key::Left, false);
  m_main_window->SetKeyStatus(1, Key::Right, false);
  m_main_window->SetKeyStatus(1, Key::Start, false);
  m_main_window->SetKeyStatus(1, Key::Select, false);
  m_main_window->SetKeyStatus(1, Key::A, false);
  m_main_window->SetKeyStatus(1, Key::B, false);
  m_main_window->SetKeyStatus(1, Key::L, false);
  m_main_window->SetKeyStatus(1, Key::R, false);

  if(m_joystick) {
    SDL_CloseJoystick(m_joystick);
    m_joystick = nullptr;
  }
}

void ControllerManager::ProcessEvents() {
  auto event = SDL_Event{};
  auto input_window = m_main_window->input_window;

#if defined(__APPLE__)
  std::lock_guard guard{m_lock};
#endif

  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_EVENT_JOYSTICK_ADDED: {
        emit OnJoystickListChanged();

        auto guid = GetJoystickGUIDStringFromIndex(event.jdevice.which);
        if(guid == m_config->input.controller_guid) {
          Open(guid);
        }

        break;
      }

      case SDL_EVENT_JOYSTICK_REMOVED: {
        if(m_instance_id == event.jdevice.which) {
          Close();
        }

        emit OnJoystickListChanged();

        break;
      }

      case SDL_EVENT_JOYSTICK_BUTTON_UP: {
        emit OnJoystickButtonReleased(event.jbutton.button);
        break;
      }

      case SDL_EVENT_JOYSTICK_AXIS_MOTION: {
        const auto threshold = std::numeric_limits<int16_t>::max() / 2;

        // FIXME: negative axes cause fast-forward to be default-on, positive axes tend not to be able to trigger fast-forward at all
        auto value = event.jaxis.value;
        if(std::abs(value) > threshold) {
          emit OnJoystickAxisMoved(event.jaxis.axis, value < 0);
        }

        break;
      }

      case SDL_EVENT_JOYSTICK_HAT_MOTION: {
        /**
         * Make sure to pick a single direction when the hat is triggered in
         * two directions at once (i.e. left + up).
         * To achieve this we extract the lowest set bit with some bit magic.
         */
        const int direction = event.jhat.value & ~(event.jhat.value - 1);
        if(direction != 0) {
          // TODO: wrap hats as buttons (who the fuck calls dpads hats in the big 26????? then again Linux implements controller HID drivers in kernel. madness all the way down)
          emit OnJoystickHatMoved(event.jhat.hat, direction);
        }

        break;
      }
    }
  }

  if(input_window && input_window->has_game_controller_choice_changed) {
    Open(m_config->input.controller_guid);

    input_window->has_game_controller_choice_changed = false;
  }
}

void ControllerManager::UpdateKeyState() {
  using Key = nba::Key;

#if defined(__APPLE__)
  std::lock_guard guard{m_lock};
#endif

  if(m_joystick == nullptr) {
    return;
  }

  auto const& input = m_config->input;

  const auto evaluate = [&](QtConfig::Input::Map const& mapping) {
    bool pressed = false;

    const int button = mapping.controller.button;
    const int axis = mapping.controller.axis;
    const int hat = mapping.controller.hat; // TODO: implement hats as buttons

    if(button != -1) {
      pressed = pressed || SDL_GetJoystickButton(m_joystick, button);
    }

    if(axis != -1) {
      const auto threshold = std::numeric_limits<int16_t>::max() / 2;

      auto actual_axis = axis & ~0x100;
      bool negative = axis & 0x100;
      auto value = SDL_GetJoystickAxis(m_joystick, actual_axis);

      pressed = pressed || (negative ? (value < -threshold) : (value > threshold));
    }

    if(hat != -1) {
      const int hat_direction = mapping.controller.hat_direction;

      pressed = pressed || ((SDL_GetJoystickHat(m_joystick, hat) & hat_direction) != 0);
    }

    return pressed;
  };

  for(int i = 0; i < (int)nba::Key::Count; i++) {
    m_main_window->SetKeyStatus(
      1, static_cast<nba::Key>(i), evaluate(input.gba[i]));
  }

  bool fast_forward_button = evaluate(input.fast_forward);

  if(fast_forward_button != m_ff_button_previous_state) {
    m_main_window->SetFastForward(1, fast_forward_button);
    m_ff_button_previous_state = fast_forward_button;
  }
}
