import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import com.grams.prototable

Pane {
    id: root
    width: 240
    padding: 0
    Material.elevation: 6
    background: Rectangle {
        y: 5
        height: parent.height - 5
        width: parent.width
        border.color: {
            if (isSelected) {
                return "#ff8c00" 
            }
            return "#6A6A6A"     // Black border for individual regimes
        }
        border.width: model.cycle_status === 1 ? 2 : 1  // Thicker border for cycles
        color: {
            // Background highlighting for cycles
            if (model.cycle_status === 1) {
                return "#464646"
            }
            return "#2B2B2B"
        }
    }
    required property int index
    required property var model
    property bool isSelected: false
    property bool controlsEnabled: model.state === RegimeState.State.Waiting

    Row {
        height: parent.height
        width: parent.width
        spacing: 3
        SpinBox {
            y: parent.height/2 - 15
            height: 35
            width: 70
            wheelEnabled: false
            enabled: controlsEnabled
            // Validation constraints for repeat counts
            from: 1
            to: 1000
            // Conditional value binding: use repeat for individual regimes, cycle_repeat for cycles
            value: model.cycle_status === 0 ? model.repeat : model.cycle_repeat
            onValueModified: {
                // Conditional role assignment based on cycle status
                if (model.cycle_status === 0) {
                    // Individual regime: modify repeat count
                    model.repeat = value
                } else {
                    // Cycle (cycle_status === 1): modify cycle repeat count
                    model.cycle_repeat = value
                }
            }
            up.indicator:  ScrollArrow {
                arrowColor: "white"
                transform: Translate {x: 48 ; y: 2}
            }
            down.indicator: ScrollArrow {
                arrowColor: "white"
                rotation: 180
                transform: Translate {x: 48; y: 12}
            }
        }
        CheckBox {
            y: parent.height/2 - 15
            height: 35
            checked: isSelected
            enabled: controlsEnabled
            onClicked: {
                root.isSelected = checked
            }
        }
        Button {
            id: upButton
            y: parent.height/2 - 15
            height: 35
            width: 40
            text: "⬆️"
            font.pointSize: 11
            background: Rectangle {
                color: "transparent" // Or "#00000000" for full transparency
                // You can add borders or other visual elements if desired
                // border.color: "lightgray"
                // border.width: 1
            }
            visible: RegimeManager.model.isMoveUpEnabled([index]) && model.state === RegimeState.State.Waiting
            onClicked: {
                RegimeManager.model.moveSelection([index], true)
            }
        }

        Button {
            id: downButton
            y: parent.height/2 - 15
            height: 35
            width: 40
            text: "⬇️"
            font.pointSize: 11
            background: Rectangle {
                color: "transparent" // Or "#00000000" for full transparency
                // You can add borders or other visual elements if desired
                // border.color: "lightgray"
                // border.width: 1
            }
            visible: RegimeManager.model.isMoveDownEnabled([index]) && model.state === RegimeState.State.Waiting
            onClicked: {
                RegimeManager.model.moveSelection([index], false)
            }
        }

        Button {
            id: deleteButton
            y: parent.height/2 - 15
            height: 35
            width: 40
            text: "🗑️"
            font.pointSize: 11
            background: Rectangle {
                color: "transparent" // Or "#00000000" for full transparency
                // You can add borders or other visual elements if desired
                // border.color: "lightgray"
                // border.width: 1
            }
            visible: root.isSelected && controlsEnabled
            onClicked: {
                RegimeManager.model.deleteRows([index])
            }
        }
    }
}