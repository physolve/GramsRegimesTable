import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.grams.prototable
import "Utils.js" as Utils

ApplicationWindow {
    width: 960
    height: 480
    visible: true
    title: qsTr("ProtoTable App")

    ProtoTableModel {
        id: protoTableModel
    }

    Connections {
        target: protoTableModel
        function onSelectionShouldBeCleared() {
            controlsGridLayout.selectedRows = []
        }
    }

    TableView {
        id: tableView
        x: 10
        y: 10
        width: 540
        height: 400
        clip: true
        columnSpacing: 1
        rowSpacing: 1
        rowHeightProvider: (row) => 40
        property var columnWidths: [80, 270, 110, 80]
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
                column: 3
                delegate: ComboBox {
                    model: ["Активен", "Пауза", "Пропущен"]
                    currentIndex: model.status
                    onCurrentIndexChanged: model.status = currentIndex
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
        id: controlsView
        x: 500
        y: 10
        width: 260
        height: 400
        GridLayout {
            id: controlsGridLayout
            columns: 1
            rowSpacing: 1
            property var selectedRows: []

            function toggleSelection(index) {
                var newSelection = selectedRows.slice();
                var idx = newSelection.indexOf(index);
                if (idx === -1) {
                    newSelection.push(index);
                } else {
                    newSelection.splice(idx, 1);
                }
                selectedRows = newSelection;
            }

            Repeater {
                model: protoTableModel
                delegate: DelegateChooser {
                    role: "cycle_status"
                    DelegateChoice {
                        roleValue: 1 // First in cycle
                        delegate: Rectangle {
                            width: 100
                            height: 40 * model.span
                            Layout.rowSpan: model.span
                            border.color: "black"
                            border.width: 1
                            color: controlsGridLayout.selectedRows.indexOf(index) !== -1 ? "lightblue" : "white"

                            RowLayout {
                                anchors.fill: parent
                                CheckBox {
                                    checked: controlsGridLayout.selectedRows.indexOf(index) !== -1
                                    onClicked: controlsGridLayout.toggleSelection(index)
                                }
                                Button {
                                    text: "Up"
                                    enabled: protoTableModel.isMoveUpEnabled([index])
                                    onClicked: controlsGridLayout.selectedRows = protoTableModel.moveSelection([index], true)
                                }
                                Button {
                                    text: "Down"
                                    enabled: protoTableModel.isMoveDownEnabled([index])
                                    onClicked: controlsGridLayout.selectedRows = protoTableModel.moveSelection([index], false)
                                }
                                Button {
                                    text: "Group"
                                    enabled: protoTableModel.isSelectionGroupable(controlsGridLayout.selectedRows)
                                    onClicked: {
                                        protoTableModel.groupRows(controlsGridLayout.selectedRows.sort())
                                        controlsGridLayout.selectedRows = []
                                    }
                                }
                                Button {
                                    text: "Ungroup"
                                    enabled: protoTableModel.isSelectionUngroupable(controlsGridLayout.selectedRows)
                                    onClicked: {
                                        protoTableModel.ungroupRows(controlsGridLayout.selectedRows.sort())
                                        controlsGridLayout.selectedRows = []
                                    }
                                }
                            }
                        }
                    }
                    DelegateChoice {
                        roleValue: 0 // Not in cycle
                        delegate: Rectangle {
                            width: 100
                            height: 40
                            border.color: "black"
                            border.width: 1
                            color: controlsGridLayout.selectedRows.indexOf(index) !== -1 ? "lightblue" : "white"

                            RowLayout {
                                anchors.fill: parent
                                CheckBox {
                                    checked: controlsGridLayout.selectedRows.indexOf(index) !== -1
                                    onClicked: controlsGridLayout.toggleSelection(index)
                                }
                                Button {
                                    text: "Up"
                                    enabled: protoTableModel.isMoveUpEnabled([index])
                                    onClicked: controlsGridLayout.selectedRows = protoTableModel.moveSelection([index], true)
                                }
                                Button {
                                    text: "Down"
                                    enabled: protoTableModel.isMoveDownEnabled([index])
                                    onClicked: controlsGridLayout.selectedRows = protoTableModel.moveSelection([index], false)
                                }
                                Button {
                                    text: "Group"
                                    enabled: protoTableModel.isSelectionGroupable(controlsGridLayout.selectedRows)
                                    onClicked: {
                                        protoTableModel.groupRows(controlsGridLayout.selectedRows.sort())
                                        controlsGridLayout.selectedRows = []
                                    }
                                }
                                Button {
                                    text: "Ungroup"
                                    enabled: protoTableModel.isSelectionUngroupable(controlsGridLayout.selectedRows)
                                    onClicked: {
                                        protoTableModel.ungroupRows(controlsGridLayout.selectedRows.sort())
                                        controlsGridLayout.selectedRows = []
                                    }
                                }
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

    MenuBar {
        id: menuBar
        x: 10
        y: 410
        Menu {
            title: "Добавить"
            MenuItem {
                text: qsTr("Вакуум")
                onTriggered: protoTableModel.addRow("Вакуum")
            }
            MenuItem {
                text: qsTr("Режим в")
                onTriggered: protoTableModel.addRow("Режим в")
            }
            MenuItem {
                text: qsTr("Режим г")
                onTriggered: protoTableModel.addRow("Режим г")
            }
        }
        Menu {
            title: "Удалить"
            MenuItem {
                text: "Удалить выбранные"
                enabled: controlsGridLayout.selectedRows.length > 0
                onTriggered: {
                    protoTableModel.deleteRows(getSelectedRows())
                    controlsGridLayout.selectedRows = [];
                }
            }
        }
    }
}