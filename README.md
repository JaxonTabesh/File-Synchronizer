# File Synchronizer - C++20 Directory Synchronization Utility

## Overview

File Synchronizer is a command-line utility built with **C++20** that performs **one-way directory synchronization**. It ensures a target directory mirrors a source directory by efficiently identifying and processing differences based on file size and modification times. The tool leverages modern C++ standard library features, including `std::filesystem` and `std::chrono`.

## Core Features & Technology

* **One-Way Synchronization:** Replicates changes from a source to a target directory (handles file/directory copy, update, delete).
* **Modern C++ (C++20):** Developed using current C++ standards and best practices.
* **`std::filesystem`:** Utilized for robust and cross-platform path manipulation, directory iteration, and file operations.
* **`std::chrono`:** Employed for precise handling and comparison of file modification timestamps (`file_time_type`, `clock_cast`).
* **Standard Containers & Algorithms:** Effective use of `vector`, `set`, `unordered_map` for state management and difference calculation.
* **Change Detection:** Compares file size and last-write time to determine necessary updates.
* **Recursive Operation:** Processes nested directories and files correctly.
* **Dry Run Mode (`--dry-run`):** Provides a safe preview of all intended file operations without altering the filesystem.
* **Continuous Monitoring (Polling):** Includes a basic continuous operation mode that periodically checks for changes.

## Purpose

This tool serves as a practical demonstration of applying modern C++ features to solve a common system utility task. It highlights proficiency in standard library usage, algorithm design, and command-line application development. While functional for its defined scope, it is presented as a portfolio piece; production deployment would require further enhancements (e.g., OS-level event monitoring, advanced error handling, comprehensive testing).

## Prerequisites

* A C++20 compliant compiler.

## Build Instructions

To build this project on Windows, **you must use the MSVC compiler** (from Visual Studio 2019 or newer). It provides the most reliable support for `std::filesystem`.


**Primary Command:**

`one-way-dir-sync <source> <target>`: Executes the synchronization.

**Options:**

* `--dry-run`: Simulates the sync and prints intended actions. Recommended for verification.
* `--help`: Displays usage details.

(The program runs continuously in sync mode; use Ctrl+C to terminate.)

## Example

**One-Way Directory Synchronization with Dry Run Example:**

```bash
./FileSync.exe one-way-dir-sync /path/to/source /path/to/target --dry-run
```

```bash
One-Way-Sync Active (DryRun Mode)
        Source Path:
                /path/to/source
        Target Path:
                /path/to/target
Deleted directory:
        /path/to/target/New Folder Extra
Deleted file:
        /path/to/target/New Text Document Extra.txt
Created directory:
        /path/to/target/New Folder
Copied file from:
        /path/to/source/New Text Document.txt
        To:
        /path/to/target/New Text Document.txt
```

**One-Way Directory Synchronization without Dry Run Example:**

```bash
./FileSync.exe one-way-dir-sync /path/to/source /path/to/target
```

```bash
One-Way-Sync Active
        Source Path:
                /path/to/source
        Target Path:
                /path/to/target
Deleted directory:
        /path/to/target/New Folder Extra
Deleted file:
        /path/to/target/New Text Document Extra.txt
Created directory:
        /path/to/target/New Folder
Copied file from:
        /path/to/source/New Text Document.txt
        To:
        /path/to/target/New Text Document.txt
```

## Potential Enhancements

Areas for future development to build upon this foundation include:

* Adding recurring backups.
* Ensure cross-platform compatibility for Windows, Linux, and macOS.
* Implementing efficient, OS-native file system event monitoring.
* Developing robust error handling and logging mechanisms.
* Adding two-way synchronization capabilities with conflict resolution.
* Introducing configuration file support for managing sync tasks.
* Building a comprehensive test suite (unit/integration tests).
* Implementing encryption for secure file transfers.
* Creating web interface for backup control.

## License

Distributed under the MIT License. See `LICENSE.txt`.
