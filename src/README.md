# BitTorrent Simulation in OpenMPI

## Project Overview

This project implements a BitTorrent simulation using the OpenMPI framework, following the BitTorrent protocol:

- **Peer-to-Peer File Sharing**: Efficient file distribution among multiple peers.
- **Tracker Management**: A centralized tracker to monitor files and manage peers/seeders.

The project follows [Google's C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

---

## Project Structure

The project is organized into the following components:

| **File**                  | **Description**                                                         |
|---------------------------|-------------------------------------------------------------------------|
| `main.cpp`                | Entry point of the program.                                            |
| `tracker.cpp`             | Implementation of the tracker class, responsible for file and peer management. |
| `tracker.h`               | Header file for the tracker class.                                     |
| `client.cpp`              | Implementation of the client class, simulating a BitTorrent peer.      |
| `client.h`                | Header file for the client class.                                      |
| `auxillary.cpp`           | Implementation of auxiliary utility functions, used by both tracker and peers                        |
| `auxillary.h`             | Header file for auxiliary functions.                                   |
| `thread_functions.cpp`    | Implementation of thread functions utilized by the client class.       |
| `thread_functions.h`      | Header file for thread-related functions.                              |
| `type.h`                  | Header file defining OpenMPI-specific data types / structures used throughout the project. |
| `type.cpp`               | Functions     to  create the OpenMPI types           |
| `Makefile`                | Makefile for compiling the project.                                    |

---

## Key Features

1. **Peer-to-Peer Architecture**: Efficient file sharing among peers using a distributed system model.
2. **Centralized Tracker**: Manages metadata about files and connected peers.
3. **Multithreading**: Implements thread-based operations for handling concurrent client tasks.
4. **MPI Integration**: Utilizes OpenMPI for parallel processing and message passing between components.

---

## Coding Standards

This project strictly adheres to [Google's C++ Style Guide](https://google.github.io/styleguide/cppguide.html), ensuring:

- Consistent naming conventions.
- Readable and maintainable code.
- Proper documentation of all classes and functions.

---

## Getting Started

### Prerequisites
- OpenMPI installed on your system.
- A compatible C++ compiler.

### Compilation and Execution
1. Clone the repository.
2. Compile the project using a suitable build system (e.g., `Makefile` or `CMake`).
3. Run the program using an MPI command (e.g., `mpirun`).

For detailed instructions, refer to the **README** included in the repository.

---

## License

This project is licensed under the MIT License. See the LICENSE file for details.
