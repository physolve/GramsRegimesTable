/*
 * MyProtoTable.qml
 *
 * This QML file defines a custom TableView component with headers for managing
 * experimental regimes or process parameters. It is based on ProtoTable.qml but
 * uses HorizontalHeaderView and VerticalHeaderView for a more traditional table experience,
 * including sortable columns.
 *
 * Features:
 * - Horizontal and Vertical headers.
 * - Sortable columns by clicking on the horizontal header.
 * - Customizable column widths.
 * - Dynamic model for table data using a SortFilterModel wrapping a TableModel.
 * - Single row selection with custom selection behavior.
 * - DelegateChooser for column-specific rendering and interaction.
 * - Custom delegates for:
 *   - Regime name (Button with press-and-hold for selection).
 *   - Parameters (ComboBox and TextFields for temperature/time).
 *   - SpinBox for numerical input.
 *   - Read-only TextField for time display.
 *   - Custom ProgressBar for visual feedback.
 * - Integration with external components like `SupplyRegime` for parameter adjustment.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels
import QtQuick.Controls.Material

Rectangle {
    id: root
    width: 800
    height: 600
    color: "#333" // A background color to see the components

        
    TableView {
        id: tableView
        columnSpacing: 1
        rowSpacing: 1
        boundsBehavior: Flickable.StopAtBounds
        selectionBehavior: TableView.SelectRows
        selectionMode: TableView.SingleSelection
        property var columnWidths: [80, 270, 80, 110, 100]
        columnWidthProvider: function (column) { return columnWidths[column] }
        model: TableModel {
            TableModelColumn { display: "name" }
            TableModelColumn { display: "passed" }
            TableModelColumn { display: "from" }
            TableModelColumn { display: "time" }
            TableModelColumn { display: "setting" }
            // Each row is one type of fruit that can be ordered
            rows: [
            ]
        }
        selectionModel: ItemSelectionModel {
        }
    }
}