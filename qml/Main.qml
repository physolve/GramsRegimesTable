import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.grams.prototable
import "Utils.js" as Utils

import QtQuick.Dialogs

ApplicationWindow {
    width: 960
    height: 480
    visible: true
    title: qsTr("ProtoTable App")

    FileDialog {
        id: openFileDialog
        title: "Please choose a file to open"
        fileMode: FileDialog.OpenFile
        onAccepted: {
            RegimeManager.importRegimes(fileUrl)
        }
    }

    FileDialog {
        id: saveAsFileDialog
        title: "Please choose a file to save"
        fileMode: FileDialog.SaveFile
        onAccepted: {
            RegimeManager.saveRegimesAs(fileUrl)
        }
    }

    Connections {
        target: RegimeManager.model
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
        model: RegimeManager.model
        selectionModel: ItemSelectionModel {
            model: RegimeManager.model
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
                model: RegimeManager.model
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
                                    enabled: RegimeManager.model.isMoveUpEnabled([index])
                                    onClicked: controlsGridLayout.selectedRows = RegimeManager.model.moveSelection([index], true)
                                }
                                Button {
                                    text: "Down"
                                    enabled: RegimeManager.model.isMoveDownEnabled([index])
                                    onClicked: controlsGridLayout.selectedRows = RegimeManager.model.moveSelection([index], false)
                                }
                                Button {
                                    text: "Group"
                                    enabled: RegimeManager.model.isSelectionGroupable(controlsGridLayout.selectedRows)
                                    onClicked: {
                                        RegimeManager.model.groupRows(controlsGridLayout.selectedRows.sort())
                                    }
                                }
                                Button {
                                    text: "Ungroup"
                                    enabled: RegimeManager.model.isSelectionUngroupable(controlsGridLayout.selectedRows)
                                    onClicked: {
                                        RegimeManager.model.ungroupRows(controlsGridLayout.selectedRows.sort())
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
                                    enabled: RegimeManager.model.isMoveUpEnabled([index])
                                    onClicked: controlsGridLayout.selectedRows = RegimeManager.model.moveSelection([index], true)
                                }
                                Button {
                                    text: "Down"
                                    enabled: RegimeManager.model.isMoveDownEnabled([index])
                                    onClicked: controlsGridLayout.selectedRows = RegimeManager.model.moveSelection([index], false)
                                }
                                Button {
                                    text: "Group"
                                    enabled: RegimeManager.model.isSelectionGroupable(controlsGridLayout.selectedRows)
                                    onClicked: {
                                        RegimeManager.model.groupRows(controlsGridLayout.selectedRows.sort())
                                    }
                                }
                                Button {
                                    text: "Ungroup"
                                    enabled: RegimeManager.model.isSelectionUngroupable(controlsGridLayout.selectedRows)
                                    onClicked: {
                                        RegimeManager.model.ungroupRows(controlsGridLayout.selectedRows.sort())
                                    }
                                }
                            }
                        }
                    }
                    DelegateChoice {
                        roleValue: 2
                        delegate: Rectangle {
                            width: 100
                            height: 2
                        }
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
            title: "Файл"
            MenuItem {
                text: qsTr("Сохранить")
                enabled: RegimeManager.currentFilePath
                onTriggered: RegimeManager.saveRegimes()
            }
            MenuItem {
                text: qsTr("Сбросить")
                onTriggered: RegimeManager.loadDefaultRegimes()
            }
            MenuItem {
                text: qsTr("Импорт")
                onTriggered: openFileDialog.open()
            }
            MenuItem {
                text: qsTr("Экспорт")
                onTriggered: saveAsFileDialog.open()
            }
        }
        Menu {
            title: "Добавить"
            MenuItem {
                text: qsTr("Вакуум")
                onTriggered: RegimeManager.model.addRow("Вакуум")
            }
            MenuItem {
                text: qsTr("Режим в")
                onTriggered: RegimeManager.model.addRow("Режим в")
            }
            MenuItem {
                text: qsTr("Режим г")
                onTriggered: RegimeManager.model.addRow("Режим г")
            }
        }
        Menu {
            title: "Удалить"
            MenuItem {
                text: "Удалить выбранные"
                enabled: controlsGridLayout.selectedRows.length > 0
                onTriggered: {
                    RegimeManager.model.deleteRows(controlsGridLayout.selectedRows)
                    controlsGridLayout.selectedRows = [];
                }
            }
        }
    }
}
