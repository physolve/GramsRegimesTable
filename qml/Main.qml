import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.grams.prototable
import "Utils.js" as Utils

ApplicationWindow {
    width: 800
    height: 480
    visible: true
    title: qsTr("ProtoTable App")

    ProtoTableModel {
        id: protoTableModel
    }

    TableView {
        id: tableView
        x: 10
        y: 10
        width: 460
        height: 400
        clip: true
        columnSpacing: 1
        rowSpacing: 1
        rowHeightProvider: (row) => 40
        property var columnWidths: [80, 270, 110]
        columnWidthProvider: function (column) { return columnWidths[column] }
        boundsBehavior: TableView.StopAtBounds
        model: protoTableModel
        selectionModel: ItemSelectionModel {
            model: protoTableModel
        }

        delegate: DelegateChooser {
            DelegateChoice {
                column: 0
                delegate: Button {
                    text: model.regime.name
                }
            }
            DelegateChoice {
                column: 1
                delegate: ConditionCell {
                }
            }
            DelegateChoice {
                column: 2
                delegate: TextField {
                    id: maxTimeDelegate
                    text: Utils.formatTime(model.max_time)
                    inputMask: "99:99:99"
                    inputMethodHints: Qt.ImhTime
                    horizontalAlignment: TextInput.AlignHCenter
                    onEditingFinished: {
                        if (acceptableInput) {
                            model.max_time = Utils.timeToMinutes(text)
                        } else {
                            text = Utils.formatTime(model.max_time)
                        }
                    }
                }
            }
            DelegateChoice {
                delegate: Label{
                    text: model.display
                }
            }
        }
    }

    Item {
        x: 465
        y: 10
        width: 140
        height: 400
        GridLayout {
            id: repeatLayout
            columns: 1
            rowSpacing: 1
            Repeater {
                model: protoTableModel
                delegate: DelegateChooser {
                    role: "cycle_status"
                    DelegateChoice {
                        roleValue: 1 // First in cycle
                        delegate: Rectangle {
                            height: 40 * model.span
                            width: 120
                            Layout.rowSpan: model.span
                            border.width: 1
                            SpinBox {
                                anchors.fill: parent
                                value: model.cycle_repeat
                                onValueModified: model.cycle_repeat = value
                            }
                        }
                    }
                    DelegateChoice {
                        roleValue: 0 // Not in cycle
                        delegate: Rectangle {
                            height: 40
                            width: 120
                            border.width: 1
                            SpinBox {
                                anchors.fill: parent
                                value: model.repeat
                                onValueModified: model.repeat = value
                            }
                        }
                    }
                    DelegateChoice {
                        roleValue: 2 // Subsequent in cycle
                        delegate: Item {}
                    }
                }
            }
        }
    }
    Item {
        x: 610
        y: 10
        width: 60
        height: 400
        GridLayout {
            id: selectLayout
            columns: 1
            rowSpacing: 1
            property var selectedRows: []

            Repeater {
                model: protoTableModel
                delegate: Rectangle {
                    width: 60
                    height: 40
                    border.color: "black"
                    border.width: 1
                    color: selectLayout.selectedRows.indexOf(index) !== -1 ? "lightblue" : "white"

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: (mouse) => {
                            if (mouse.button === Qt.LeftButton) {
                                var newSelection = selectLayout.selectedRows.slice();
                                var idx = newSelection.indexOf(index);
                                if (idx === -1) {
                                    newSelection.push(index);
                                } else {
                                    newSelection.splice(idx, 1);
                                }
                                selectLayout.selectedRows = newSelection;
                            } else if (mouse.button === Qt.RightButton) {
                                contextMenu.popup()
                            }
                        }
                    }
                }
            }
        }
    }

    Menu {
        id: contextMenu
        MenuItem {
            text: qsTr("Group")
            onTriggered: {
                protoTableModel.groupRows(selectLayout.selectedRows.sort())
                selectLayout.selectedRows = []
            }
        }
        MenuItem {
            text: qsTr("Ungroup")
            onTriggered: {
                protoTableModel.ungroupRows(selectLayout.selectedRows.sort())
                selectLayout.selectedRows = []
            }
        }
    }
}