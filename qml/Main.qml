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
    title: qsTr("ProtoTable App") + (RegimeManager.dirty ? " *" : "")

    onClosing: function(close) {
        if (RegimeManager.dirty) {
            close.accepted = false;
            unsavedChangesDialog.open();
        }
    }

    Dialog {
        id: unsavedChangesDialog
        title: "Unsaved Changes"
        standardButtons: Dialog.Save | Dialog.Discard | Dialog.Cancel
        modal: true
        visible: false

        onAccepted: {
            RegimeManager.saveRegimes();
            Qt.quit();
        }

        onDiscarded: {
            Qt.exit(0);
        }
    }

    FileDialog {
        id: openFileDialog
        title: "Please choose a file to open"
        fileMode: FileDialog.OpenFile
        onAccepted: {
            if (RegimeManager.dirty) {
                unsavedChangesDialog.open();
            } else {
                RegimeManager.importRegimes(fileUrl)
            }
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
        function getSelectedRows(){
            let selectedRows = []
            for(let i = 0; i < repeater.count; i++){
                if(repeater.itemAt(i).isSelected)
                {
                    selectedRows.push(i)
                }
            }                        
            return selectedRows
        }
        GridLayout {
            id: controlsGridLayout
            columns: 1
            rowSpacing: 1
            Repeater {
                id: repeater
                model: RegimeManager.model
                delegate: DelegateChooser {
                    role: "cycle_status"
                    DelegateChoice {
                        roleValue: 1
                        delegate: ControlDelegate {
                            width: 250
                            height: 40 * model.span
                            Layout.rowSpan: model.span
                            isSelected: false
                        }
                    }
                    DelegateChoice {
                        roleValue: 0
                        delegate: ControlDelegate {
                            width: 250
                            height: 40
                            isSelected: false
                        }
                    }
                    DelegateChoice {
                        roleValue: 2
                        delegate: Item {
                            height: 0
                            property bool isSelected: false
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
                // enabled: controlsGridLayout.selectedRows.length > 0
                onTriggered: {
                    let tSelectedRows = []
                    for(let i = 0; i < repeater.count; i++){
                        if(repeater.itemAt(i).isSelected)
                        {
                            tSelectedRows.push(i)
                        }
                    }
                    RegimeManager.model.deleteRows(tSelectedRows)
                    for(let i = 0; i < repeater.count; i++)
                        repeater.itemAt(i).isSelected = false
                }
            }
        }
        Menu {
            title: "Цикл"
            MenuItem {
                text: "Сгруппировать"
                // enabled: controlsGridLayout.isSelectionGroupable
                onTriggered: {
                    let tSelectedRows = []
                    for(let i = 0; i < repeater.count; i++){
                        if(repeater.itemAt(i).isSelected)
                        {
                            if (repeater.itemAt(i).model.cycle_status === 1) {
                                for (let j = 0; j < repeater.itemAt(i).model.span; j++) {
                                    tSelectedRows.push(i + j)
                                }
                            } else {
                                tSelectedRows.push(i)
                            }
                        }
                    }
                    RegimeManager.model.groupRows(tSelectedRows)
                    for(let i = 0; i < repeater.count; i++)
                        repeater.itemAt(i).isSelected = false
                }
            }
            MenuItem {
                text: "Разгруппировать"
                // enabled: controlsGridLayout.isSelectionUngroupable
                onTriggered: {
                    let tSelectedRows = []
                    for(let i = 0; i < repeater.count; i++){
                        if(repeater.itemAt(i).isSelected)
                        {
                            if (repeater.itemAt(i).model.cycle_status === 1) {
                                for (let j = 0; j < repeater.itemAt(i).model.span; j++) {
                                    tSelectedRows.push(i + j)
                                }
                            } else {
                                tSelectedRows.push(i)
                            }
                        }
                    }
                    RegimeManager.model.ungroupRows(tSelectedRows)
                    for(let i = 0; i < repeater.count; i++)
                        repeater.itemAt(i).isSelected = false
                }
            }
        }
    }
}
