import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.grams.prototable

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("ProtoTable App")

    Rectangle {
        id: tableBackground

        anchors.fill: parent

        color: Application.styleHints.colorScheme === Qt.Light ? palette.mid : palette.midlight
        TableView {
            id: tableView

            width: parent.width
            height: parent.height

            clip: true
            columnSpacing: 1
            rowSpacing: 1
            rowHeightProvider: (row) => 40
            property var columnWidths: [80, 270, 80, 110, 100]
            columnWidthProvider: function (column) { return columnWidths[column] }
            boundsBehavior: TableView.StopAtBounds
            selectionModel: ItemSelectionModel {}
            selectionBehavior: TableView.SelectRows
            model: ProtoTableModel {
            }
            delegate: DelegateChooser {
                DelegateChoice {
                    column: 0
                    delegate: Button {
                        text: model.regime.name
                        onClicked: {
                            // Handle button click here
                        }
                    }
                }
                DelegateChoice {
                    column: 1
                    delegate: ConditionCell {
                    }
                }
                DelegateChoice {
                    column: 2
                    delegate: SpinBox {
                        value: model.repeat
                        onValueModified: {
                            model.repeat = value
                        }
                    }
                }
                DelegateChoice {
                    column: 3
                    delegate: TextField {
                        text: model.max_time
                    }
                }
                DelegateChoice {
                    delegate: Label{
                        text: model.display
                    }
                }
            }
            ScrollBar.horizontal: ScrollBar {}
            ScrollBar.vertical: ScrollBar {}
        }

        Button {
            text: "Save Changes"
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: 10
            onClicked: tableView.model.saveDataToJson()
        }

        SelectionRectangle {
            target: tableView
        }
    }
}
