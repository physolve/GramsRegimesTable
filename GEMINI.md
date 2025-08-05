# Gemini Project Overview: ProtoTable

## Project Overview

This project is a C++/QML desktop application that provides a table-based interface for managing and editing "regimes." The application allows users to:

*   View and edit a list of regimes in a table.
*   Group and ungroup regimes into cycles.
*   Reorder regimes and cycles.
*   Add and delete regimes.
*   Persist changes to a JSON file.

The application is built using the Qt framework, with the UI implemented in QML and the business logic in C++.

### Key Technologies

*   **C++:** Used for the core application logic, including the data model.
*   **Qt:** The underlying application framework.
*   **QML:** Used for the user interface.
*   **CMake:** The build system.
*   **Google Test:** Used for unit testing.

### Architecture

The application follows a Model-View-ViewModel (MVVM) architecture:

*   **Model:** The `ProtoTableModel` class, which is a `QAbstractTableModel` that manages the list of regimes.
*   **View:** The `Main.qml` file, which defines the user interface.
*   **ViewModel:** The `ProtoTableModel` also acts as the ViewModel, exposing data and commands to the QML view.

## Building and Running

### Building

To build the project, run the following commands from the root directory:

```bash
cmake -S . -B build
cmake --build build
```

### Running

To run the application, execute the following command from the root directory:

```bash
./build/prototype_table
```

### Testing

To run the unit tests, execute the following command from the root directory:

```bash
./build/tests/ProtoTableTests
```

## Development Conventions

### Coding Style

The C++ code follows the Qt coding style.

### Testing

Unit tests are written using the Google Test framework and are located in the `tests` directory. The tests cover the core functionality of the `ProtoTableModel` class.

### Data Model

The data model is defined in the `regime.h` file. The `Regime` and `Condition` structs are exposed to the meta-object system using `Q_GADGET`, which allows them to be passed by value between C++ and QML.

### QML

The QML code is located in the `qml` directory. The main view is `Main.qml`, which uses a `TableView` to display the data from the `ProtoTableModel`. The `controlsView` provides the user interface for grouping, ungrouping, and reordering rows.
