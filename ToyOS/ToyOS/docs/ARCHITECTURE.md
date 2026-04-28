# Architecture

The project is organized around two layers:

- `user/`
  - the user-facing shell interface
- `core/`
  - operating-system-style internal state and subsystems

## Current Subsystems

- `user/shell`
  - prompt, parsing, built-ins, and external command execution
- `core/kernel`
  - top-level kernel state used by the shell
- `core/process`
  - PCB and process table management
- `core/scheduler`
  - scheduler configuration and default queues
- `common`
  - shared queue and utility helpers

## Reserved For Future Expansion

- `core/memory`
  - simulated memory management
- `core/filesystem`
  - simulated file-system state
- `core/io`
  - simulated device and I/O events
- `runtime`
  - generated runtime artifacts such as memory images and logs
