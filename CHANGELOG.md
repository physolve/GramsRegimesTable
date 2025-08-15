# Changelog

## 2025-08-14

- **Visual Refinements**: Adjusted UI to better align with Material Design specification, focusing on a cleaner look and feel for the Dark theme. Reduced font sizes and updated the color palette.
- **API Documentation**: Created comprehensive documentation for the module's public C++ API. Documented `RegimeManager` API, clarified module capabilities, and updated in-code comments.
- **Unit Test Refresh**: Ensured the unit test suite is up-to-date and provides comprehensive coverage of the module's logic. Reviewed existing tests, enhanced test coverage for new functionalities, and ran the full test suite.

## 2025-08-13

- **Architectural Flaw in a QML State Delegate Resolved**: Modified `setData` function in `prototablemodel.cpp` to emit the `dataChanged` signal for the entire row whenever a state-related role was changed.
- **Corrected Cycle Visualization Logic in `TimeProgressBar`**: Rewrote `expandRegimesToRepeats` function in `visibleregimemodel.cpp` to correctly handle cycle expansion.
- **Fixed `TimeProgressBar` Order to Match `ProtoTableModel`**: The `expandRegimesToRepeats` function was further refined to process all regimes and cycles sequentially as they appear in the main model.
- **Corrected JSON Serialization for `max_time`**: Modified `toJson` function in `regime.cpp` to divide `max_time` by 60, ensuring it is correctly converted to minutes before serialization.
- **Enforced Consistent JSON Output Order**: Updated `toJson` function in `regime.cpp` to insert keys in a specific, desired order.
- **Implemented State-Based Control Disabling**: Disabled UI controls when a regime's state is not `Waiting`.
- **Fixed Delegate Update Issue for Adjacent Rows**: Enhanced `setData` function in `prototablemodel.cpp` to emit a `dataChanged` signal for both the affected row and the one immediately following it.

## 2025-08-12

- **Complete External Module API Implementation**: Implemented a comprehensive external module API for regime execution control, along with advanced progress visualization features.
- **Enhanced Regime Data Model**: Added execution tracking fields to `Regime` class.
- **Extended ProtoTableModel API**: Added new data roles for execution tracking.
- **Enhanced VisibleRegimeModel with Individual Repeat Visualization**: Implemented a repeat-based architecture in `VisibleRegimeModel`.
- **Advanced TimeProgressBar with Individual Repeat Rectangles**: Redesigned `TimeProgressBar` to show individual repeat rectangles with detailed labels and tooltips.
- **Immediate UI Update System**: Ensured all API calls trigger instant UI refresh.
- **Enhanced Test Function with Repeat Progression**: Updated `testUpdatingRegimes()` function to test repeat progression.
- **Comprehensive Test Suite**: Added comprehensive tests for the new API.

## 2025-08-09

- **TimeProgressBar Enhancements**: The `TimeProgressBar` is now synchronized with the `ProtoTableModel`, ensuring that it automatically updates whenever the model's data changes. The regime name is now displayed inside its corresponding rectangle in the `TimeProgressBar`, improving clarity.

## 2025-08-08

- **Core Logic & Model Enhancements**: Introduced new data roles and properties in `ProtoTableModel` and `Regime` for tracking regime state, repeat counts, and cycle IDs. Implemented comprehensive state management and time calculation logic within `RegimeManager`.
- **QML UI Updates**: Integrated `RegimeDelegate` and `TimeProgressBar` components into `Main.qml`.
- **Build & Configuration**: Updated `CMakeLists.txt` to include new QML files and a new test file.
- **Documentation**: Updated `README.md` to reflect changes in default repeat count behavior.

## 2025-08-07

- **Implemented "State" Property**: Introduced a `RegimeEnums::State` enum and added `m_state` and `m_timePassedInSeconds` members to the `Regime` struct.
- **QML Changes**: Created a new `StateDelegate.qml` to display the regime state and updated `Main.qml` to use it.
- **Build System Changes**: Added the new `StateDelegate.qml` file to the list of QML files in `CMakeLists.txt`.
- **Testing Changes**: Updated tests to reflect the changes in the `RegimeManager` class.

## 2025-08-06

- **QML Delegate Refactoring**: Centralized control logic by moving action buttons out of `ControlDelegate` and into `Main.qml`.
- **"Unsaved Changes" Tracking**: Implemented a "dirty" flag in `RegimeManager` to track unsaved changes, with a visual indicator in the application title and a confirmation dialog on close.

## 2025-08-05

- **Architectural Changes**: Centralized data management with `RegimeManager` as a QML Singleton.
- **C++ Refactoring**: Consolidated all file I/O logic within `RegimeManager` and simplified `ProtoTableModel`.
- **QML UI/UX Improvements**: Reworked the "File" menu and replaced the single `FileDialog` with two distinct dialogs for opening and saving.
- **Build System & Testing**: Updated `CMakeLists.txt` to include the new `regimemanager.h` and `regimemanager.cpp` files and simplified unit tests.

## 2025-08-04

- **QML Refactoring**: Overhauled the `controlsView` to directly integrate selection checkboxes and action buttons within the delegates for each row.
- **C++ Model Enhancements**: Added `StatusRole` to the model, refactored `updateCycleIds()` to correctly re-index cycle IDs, and added several new `Q_INVOKABLE` methods for advanced row manipulation.
- **Build System Updates**: Modularized `prototablemodel.cpp` and `regime.cpp` into a static library and enabled testing in the CMake build.

## 2025-08-02

- **Features Added**: Added a "File" menu with an "Add" submenu to add new regimes to the table.
- **Refactoring and Bug Fixes**: Overhauled the `moveRows` function to correctly handle moving single rows and entire cycles. Improved `groupRows` and `ungroupRows` functions to handle cycle IDs more intelligently.
