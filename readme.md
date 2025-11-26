# C++ HashMap Implementation (Custom std::unordered_map Clone)

## Overview

This repository contains a complete, header-only implementation of a generic C++ hash map (`HashMap<K, V>`). The design closely follows the principles and interface of the C++ Standard Library's `std::unordered_map`, using **separate chaining** (vectors of pointers to entries) to handle collisions.

The primary goal of this project was to implement essential container features, including memory safety, efficient lookup, and correct iterator/const-iterator behavior.

## Features

* **Generic Templates:** Supports any key (`K`) and value (`V`) types compatible with `std::hash`.
* **Collision Handling:** Uses a `std::vector` of `std::vector`s to implement separate chaining.
* **Load Factor Management:** Automatic resizing (rehashing) to maintain performance based on a customizable `max_load_factor`.
* **Full Iterator Support:** Includes non-const (`Iterator`) and const (`Const_Iterator`) classes for traversing the entire map and individual buckets.
* **C++ Modern Semantics:** Implements the **Rule of Five** (Destructor, Copy Constructor, Copy Assignment, Move Constructor, Move Assignment) for robust memory management and efficient resource transfer.
* **Exception Safety:** Uses `std::out_of_range` for failed lookups (e.g., in `at()`).

## Build and Usage

This is a header-only library, requiring only the inclusion of `hashmap.h`.

### Prerequisites

* A C++ compiler supporting C++11 or newer (e.g., g++ or clang).

## Author
Ido Hassidim

GitHub: https://github.com/Pishoto

LinkedIn: https://www.linkedin.com/in/ido-hassidim-12705125b/

Feel free to reach out for a job offer or feedback.