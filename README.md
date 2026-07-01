# ESP32-S3 Robot Platform

A modular embedded robotics platform built with vanilla FreeRTOS and C++.

## Overview

This repository contains a flexible robot firmware project designed as a learning and experimentation platform. It is intended for rapid development of robotics behaviors, sensor-actuator interactions, and interactive games.

## Core Goal

Create an architecturally clean, universal embedded-constructor platform for safe experimentation with robotics algorithms and interactive behaviors.

## Hardware Stack

- ESP32-S3-WROOM-1 microcontroller
- 2WD + 1 caster wheel differential drive chassis
- DRV8833 dual H-bridge motor driver for brushed DC motors
- HC-SR04 ultrasonic distance sensor
- 3-channel RGB LED
- LEDC-driven buzzer
- Wi-Fi for remote commands and configuration

## Architecture Summary

The system is built around a mode-based architecture with strict separation between hardware, task logic, and behavior modes.

- Each behavior is implemented as an `IMode` class.
- A central `ModeManagerTask` dispatches events and activates one mode at a time.
- The design uses an asynchronous, non-blocking event-driven pattern:
  - "Events flow upward" into a single type-safe `RobotEventQueue`.
  - "Commands flow downward" through non-blocking proxies in `RobotContext`.
- Actuators are driven by iterative `Player::tick` loops to avoid blocking.
- All core objects are declared as `static` in `app_main()`.
- There is no runtime dynamic allocation during normal operation: zero heap allocation for maximum stability.

## Repository Layout

- `Firmware/Robot/`
  - Main robot firmware project
  - Contains `src/main.cpp`, robot modes, mode manager, HTTP control pages, and PlatformIO/ESP-IDF build config
- `Firmware/SharedComponents/`
  - Reusable drivers, tasks, services, network, messages, and data structures
  - Shared by the main robot project and experiment projects
- `Firmware/Experiments/`
  - Separate module-level test projects for individual sensors, actuators, and subsystems
  - Useful for isolated development and validation without the full robot app
- `Firmware/Architecture.md`
  - Architecture documentation design
- `Hardware/KiCad/Robot/`
  - KiCad files for the hardware design

The existing `Firmware/Architecture.md` is a great complementary reference for deeper architecture details.

## How to Build

The main firmware can be built using PlatformIO from `Firmware/Robot/`.

```sh
cd Firmware/Robot
platformio run -e esp32-s3-devkitc-1
platformio run -e esp32-s3-devkitc-1 --target upload
platformio run -e esp32-s3-devkitc-1 --target monitor
```

The `Firmware/Robot/platformio.ini` environment is configured for the `esp32-s3-devkitc-1` board.

## Notes

- `Firmware/Robot/src/main.cpp` acts as the wiring diagram: it creates hardware drivers, connects tasks, and starts the system.
- `Firmware/Architecture.md` contains the high-level architecture overview and state machine description.
- `Firmware/Experiments/` is for testing modules independently and does not contain the main robot application.
- `Firmware/SharedComponents/` contains the platform's reusable code, drivers, and services.

## Adding a New Mode

To extend the robot with a new behavior:

1. Implement `IMode` in a new mode class.
2. Use `RobotContext` proxies for indicators, motion, and sensor interactions.
3. Avoid blocking inside `onEvent()`.
4. Register the new mode in `Firmware/Robot/src/main.cpp` with `ModeManagerTask::instance().addMode(&myMode)`.
