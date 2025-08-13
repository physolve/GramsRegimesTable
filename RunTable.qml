import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.grams.prototable

import QtQuick.Dialogs

Item {
    id: runTable
    width: 700
    height: 650
    visible: true
    // title: qsTr("ProtoTable App") + (RegimeManager.dirty ? " *" : "")

    // onClosing: function(close) {
    //     if (RegimeManager.dirty) {
    //         close.accepted = false;
    //         unsavedChangesDialog.open();
    //     }
    // }

    Dialog {
        id: unsavedChangesDialog
        title: "Unsaved Changes"
        standardButtons: Dialog.Save | Dialog.Discard | Dialog.Cancel
        modal: true
        visible: false

        onAccepted: {
            RegimeManager.saveRegimes();
            // Qt.quit();
        }

        onDiscarded: {
            // Qt.exit(0);
        }
    }

    FileDialog {
        id: openFileDialog
        title: "Please choose a file to open"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Run files (*.json)"]
        onAccepted: {
            if (RegimeManager.dirty) {
                unsavedChangesDialog.open();
            } else {
                RegimeManager.importRegimes(selectedFile)
            }
        }
    }

    FileDialog {
        id: saveAsFileDialog
        title: "Please choose a file to save"
        defaultSuffix: "json"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Run files (*.json)"]
        onAccepted: {
            RegimeManager.exportRegimes(selectedFile)
        }
    }

    TableView {
        id: tableView
        x: 10
        y: 10
        width: 665
        height: 400
        clip: true
        columnSpacing: 1
        rowSpacing: 1
        rowHeightProvider: (row) => 40
        property var columnWidths: [80, 150, 80, 85]
        columnWidthProvider: function (column) { return columnWidths[column] }
        boundsBehavior: TableView.StopAtBounds
        model: RegimeManager.model
        selectionModel: ItemSelectionModel {
            model: RegimeManager.model
        }
        
        // Synchronize with controlsView scrollbar
        property bool syncingScroll: false
        
        ScrollBar.vertical: ScrollBar {
            id: tableScrollBar
            policy: ScrollBar.AsNeeded
            
            onPositionChanged: {
                if (!tableView.syncingScroll && !controlsView.syncingScroll) {
                    controlsView.syncingScroll = true
                    controlsView.ScrollBar.vertical.position = position
                    controlsView.syncingScroll = false
                }
            }
        }

        delegate: DelegateChooser {
            DelegateChoice {
                column: 0
                delegate: RegimeDelegate {
                    onRegimeClicked: (regime) => {
                        console.log("Regime clicked:", regime.name)
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
                delegate: TextField {
                    id: maxTimeDelegate
                    text: formatTime(model.max_time)
                    inputMask: "99:99:99"
                    inputMethodHints: Qt.ImhTime
                    horizontalAlignment: TextInput.AlignHCenter
                    
                    // Custom validator for time range 00:00:01 to 23:59:59
                    validator: RegularExpressionValidator {
                        regularExpression: /^([01]?[0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9]$/
                    }
                    
                    onEditingFinished: {
                        if (acceptableInput) {
                            let seconds = timeToSeconds(text)
                            // Enforce minimum 1 second, maximum 23:59:59 (86399 seconds)
                            if (seconds >= 1 && seconds <= 86399) {
                                model.max_time = seconds
                            } else {
                                // Reset to previous valid value
                                text = formatTime(model.max_time)
                            }
                        } else {
                            // Reset to previous valid value
                            text = formatTime(model.max_time)
                        }
                    }
                }
            }
            DelegateChoice {
                column: 3
                delegate: StateDelegate {
                }
            }
            DelegateChoice {
                delegate: Label{
                    text: model.display
                }
            }
        }
    }

    ScrollView {
        id: controlsView
        x: 410
        y: 10
        width: 255
        height: 400
        clip: true
        // implicitHeight: 400
        // Synchronize with tableView scrollbar
        property bool syncingScroll: false
        
        ScrollBar.vertical: ScrollBar {
            id: controlsScrollBar
            policy: ScrollBar.AsNeeded
            
            onPositionChanged: {
                if (!controlsView.syncingScroll && !tableView.syncingScroll) {
                    tableView.syncingScroll = true
                    tableView.ScrollBar.vertical.position = position
                    tableView.syncingScroll = false
                }
            }
        }
        // contentWidth: controlsColumnLayout.implicitWidth
        contentHeight: 41*repeater.count
        Column {
            // id: controlsColumnLayout
            spacing: 1
            width: 400
            height: repeater.contentHeight
            // Layout.preferredWidth: parent.width
            // Layout.preferredHeight: childrenRect.height
            Repeater {
                id: repeater
                model: RegimeManager.model
                delegate: DelegateChooser {
                    role: "cycle_status"
                    DelegateChoice {
                        roleValue: 1
                        delegate: ControlDelegate {
                            width: 250
                            height: 40 * model.cycle_row_count
                            // Layout.rowSpan: model.cycle_row_count
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
                            visible: false
                            height: 10
                            property bool isSelected: false
                        }
                    }
                }
            }
        }
    }

    TimeProgressBar {
        id: timeProgressBar
        x: 10
        y: 420
        width: 650
        height: 130
    }

    MenuBar {
        id: menuBar
        x: 200
        y: 560
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
                                for (let j = 0; j < repeater.itemAt(i).model.cycle_row_count; j++) {
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
                                for (let j = 0; j < repeater.itemAt(i).model.cycle_row_count; j++) {
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
            // delete later testing
        }
        Menu {
            title: "Тест"
            MenuItem{
                text: "Тест выполенеие условие и время в первом"
                onTriggered: RegimeManager.testUpdatingRegimes()
            }
        }
    }

    function formatTime(seconds) {
        var hours = Math.floor(seconds / 3600)
        var minutes = Math.floor((seconds % 3600) / 60)
        var secs = seconds % 60
        return Qt.formatTime(new Date(0, 0, 0, hours, minutes, secs), "hh:mm:ss")
    }
    
    function timeToSeconds(timeString) {
        var parts = timeString.split(':')
        if (parts.length !== 3) {
            return 0
        }
        var hours = parseInt(parts[0], 10)
        var minutes = parseInt(parts[1], 10)
        var seconds = parseInt(parts[2], 10)
        if (isNaN(hours) || isNaN(minutes) || isNaN(seconds)) {
            return 0
        }
        return hours * 3600 + minutes * 60 + seconds
    }
}