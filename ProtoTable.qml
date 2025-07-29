/*
 * ProtoTable.qml
 *
 * This QML file defines a custom TableView component for managing
 * experimental regimes or process parameters. It provides a flexible
 * table structure with custom delegates for different column types,
 * allowing for various input methods and displays.
 *
 * Features:
 * - Customizable column widths.
 * - Dynamic model for table data using TableModel.
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

import QtQuick // Recommended QML version for modern features
import QtQuick.Controls // For TableView, Button, ComboBox, TextField, SpinBox
import QtQuick.Layouts // For Row layout in delegates
import Qt.labs.qmlmodels // For TableModel and TableModelColumn
import QtQuick.Controls.Material // For Material design styling (e.g., Material.roundedScale)
import com.grams.prototable 1.0

TableView { // The main table component
    id: tableView
    columnSpacing: 1
    rowSpacing: 1
    boundsBehavior: Flickable.StopAtBounds
    selectionBehavior: TableView.SelectRows
    selectionMode: TableView.SingleSelection
    property var columnWidths: [80, 270, 80, 110, 100]
    // Function to provide column widths based on the 'columnWidths' array
    columnWidthProvider: function (column) { return columnWidths[column] }
    model: ProtoTableModel {}
    selectionModel: ItemSelectionModel {
    }
    // Component.onCompleted: {
    //     model.appendRow({
    //         name: "Вакуум",
    //         passed: 0,
    //         from: 3,
    //         time: "00:00:00",
    //         setting: 0.5
    //     })
    // }
    signal openESupply()
    function openParameters(regime, row){
        console.log(regime, row)
        if(regime == "Цел. в камере"){
            // list of regime var ->
            // just show if closed
            supplyAdjustWindow.createObject(parent)
        }
            // tableView.openESupply()
    }
    delegate: DelegateChooser {
        DelegateChoice {
            column: 0
            delegate: Button {
                required property bool current
                required property bool selected
                text: model.display // Display the regime name from the model
                Material.roundedScale: Material.ExtraSmallScale // Apply Material design rounded corners
                highlighted: selected // Highlight button if its row is selected
                MouseArea {
                    anchors.fill: parent
                    // On press and hold, select the entire row
                    onPressAndHold: {
                        tableView.selectionModel.select(tableView.model.index(row, 0), ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Current | ItemSelectionModel.Rows);
                    }
                    // On release, clear the selection (allows for quick interaction without persistent selection)
                    onReleased:{
                        tableView.selectionModel.clearSelection()
                    }
                    // On click, open parameters for the selected regime
                    onClicked: openParameters(text, row)
                }
            }
        }
        DelegateChoice {
            column: 1
            // Delegate for parameters column (e.g., temperature, time settings)
            // This delegate combines a ComboBox and two TextFields within a Row layout.
            delegate:Row{
                required property bool selected
                required property bool current
                property int chosenCase: model.display
                ComboBox {
                    model: ["Темп. камера", "Время"]
                    width: parent.width/2
                    currentIndex: chosenCase
                    font.pointSize: 9
                }
                TextField{
                    font.pointSize: 9
                    validator: DoubleValidator { bottom: 0; top: 1000} //; decimals: 3
                    selectByMouse: true
                    width: parent.width/4
                    horizontalAlignment: TextInput.AlignHCenter
                    placeholderText: "°C"
                }
                TextField{
                    font.pointSize: 9
                    validator: DoubleValidator { bottom: 0; top: 1000} //; decimals: 3
                    selectByMouse: true
                    width: parent.width/4
                    horizontalAlignment: TextInput.AlignHCenter
                    placeholderText: "мин"
                }
            } 
        }
        DelegateChoice {
            column: 2
            delegate: SpinBox {
                required property bool selected
                required property bool current
                id: customSpin
                value: model.display
                onValueModified: model.display = value
            }
        }
        DelegateChoice {
            column: 3
            delegate: TextField {
                // Delegate for displaying time, read-only
                required property bool selected
                required property bool current
                text: model.display
                readOnly: true
                implicitWidth: 90
                font.pointSize: 10
                inputMask: "99:99:99" // Mask for HH:MM:SS format
                inputMethodHints: Qt.ImhTime // Hint for time input
                horizontalAlignment: TextInput.AlignHCenter
                color: selected ? "lightblue" : "white" // Change text color if row is selected
            }
        }
        DelegateChoice {
            column: 4
            // Delegate for a custom progress bar
            delegate: Rectangle{

            }
        }
        DelegateChoice {
            // Default delegate for any other column not explicitly handled above
            delegate: TextField {
                required property bool selected
                required property bool current
                text: model.display
                readOnly: true
                selectByMouse: true
                font.pointSize: 10
                // implicitWidth: 76
                horizontalAlignment: TextInput.AlignHCenter
            }
        }
    }
}