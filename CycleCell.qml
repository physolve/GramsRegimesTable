import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.grams.prototable

DelegateChooser {
    id: root
    role: "cycle_status"

    required property var model
    property int index

    DelegateChoice {
        roleValue: 1 // First in cycle
        delegate: Row {
            height: 40 * model.span

            SpinBox {
                value: model.cycle_repeat
                onValueModified: model.cycle_repeat = value
            }

            Rectangle {
                border.color: "black"
                border.width: 1
                color: root.controlsView.selectedRows.indexOf(index) !== -1 ? "lightblue" : "white"

                RowLayout {
                    anchors.fill: parent
                    CheckBox {
                        checked: root.controlsView.selectedRows.indexOf(index) !== -1
                        onClicked: root.controlsView.toggleSelection(index)
                    }
                    Button {
                        text: "Up"
                        enabled: root.model.isMoveUpEnabled([index])
                        onClicked: root.controlsView.selectedRows = root.model.moveSelection([index], true)
                    }
                    Button {
                        text: "Down"
                        enabled: root.model.isMoveDownEnabled([index])
                        onClicked: root.controlsView.selectedRows = root.model.moveSelection([index], false)
                    }
                    Button {
                        text: "Group"
                        enabled: root.model.isSelectionGroupable(root.controlsView.selectedRows)
                        onClicked: root.model.groupRows(root.controlsView.selectedRows.sort())
                    }
                    Button {
                        text: "Ungroup"
                        enabled: root.model.isSelectionUngroupable(root.controlsView.selectedRows)
                        onClicked: root.model.ungroupRows(root.controlsView.selectedRows.sort())
                    }
                    Button {
                        text: "Delete"
                        onClicked: root.model.deleteRows([index])
                    }
                }
            }
        }
    }

    DelegateChoice {
        roleValue: 0 // Not in cycle
        delegate: Row {
            height: 40
            SpinBox {
                value: model.repeat
                onValueModified: model.repeat = value
            }
            Rectangle {
                border.color: "black"
                border.width: 1
                color: root.controlsView.selectedRows.indexOf(index) !== -1 ? "lightblue" : "white"

                RowLayout {
                    anchors.fill: parent
                    CheckBox {
                        checked: root.controlsView.selectedRows.indexOf(index) !== -1
                        onClicked: root.controlsView.toggleSelection(index)
                    }
                    Button {
                        text: "Up"
                        enabled: root.model.isMoveUpEnabled([index])
                        onClicked: root.controlsView.selectedRows = root.model.moveSelection([index], true)
                    }
                    Button {
                        text: "Down"
                        enabled: root.model.isMoveDownEnabled([index])
                        onClicked: root.controlsView.selectedRows = root.model.moveSelection([index], false)
                    }
                    Button {
                        text: "Group"
                        enabled: root.model.isSelectionGroupable(root.controlsView.selectedRows)
                        onClicked: root.model.groupRows(root.controlsView.selectedRows.sort())
                    }
                    Button {
                        text: "Ungroup"
                        enabled: root.model.isSelectionUngroupable(root.controlsView.selectedRows)
                        onClicked: root.model.ungroupRows(root.controlsView.selectedRows.sort())
                    }
                    Button {
                        text: "Delete"
                        onClicked: root.model.deleteRows([index])
                    }
                }
            }
        }
    }

    DelegateChoice {
        roleValue: 2 // Subsequent in cycle
        delegate: Item { height: 0 }
    }
}
