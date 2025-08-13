import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.grams.prototable

Rectangle {
    id: root
    width: 250
    border.color: {
        // Cycle highlighting: orange border for cycles, black for individual regimes
        if (model.cycle_status === 1) {
            return "#ff8c00"  // Orange border for cycles
        }
        return "black"     // Black border for individual regimes
    }
    border.width: model.cycle_status === 1 ? 2 : 1  // Thicker border for cycles
    color: {
        if (isSelected) {
            return "lightblue"
        }
        // Background highlighting for cycles
        if (model.cycle_status === 1) {
            return "#fff8dc"  // Light cream background for cycles
        }
        return "white"     // White background for individual regimes
    }
    required property int index
    required property var model
    property bool isSelected: false
    Row {
        height: parent.height
        width: parent.width
        spacing: 5
        SpinBox {
            y: parent.height/2 - 20
            height: 40
            width: 100
            wheelEnabled: false
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
        }
        CheckBox {
            y: parent.height/2 - 20
            height: 40
            checked: isSelected
            onClicked: {
                root.isSelected = checked
            }
        }
        Button {
            id: upButton
            y: parent.height/2 - 20
            height: 40
            width: 40
            text: "⬆️"
            font.pointSize: 15
            background: Rectangle {
                color: "transparent" // Or "#00000000" for full transparency
                // You can add borders or other visual elements if desired
                // border.color: "lightgray"
                // border.width: 1
            }
            visible: RegimeManager.model.isMoveUpEnabled([index])
            onClicked: {
                RegimeManager.model.moveSelection([index], true)
            }
        }

        Button {
            id: downButton
            y: parent.height/2 - 20
            height: 40
            width: 40
            text: "⬇️"
            font.pointSize: 15
            background: Rectangle {
                color: "transparent" // Or "#00000000" for full transparency
                // You can add borders or other visual elements if desired
                // border.color: "lightgray"
                // border.width: 1
            }
            visible: RegimeManager.model.isMoveDownEnabled([index])
            onClicked: {
                RegimeManager.model.moveSelection([index], false)
            }
        }

        Button {
            id: deleteButton
            y: parent.height/2 - 20
            height: 40
            width: 40
            text: "🗑️"
            font.pointSize: 15
            background: Rectangle {
                color: "transparent" // Or "#00000000" for full transparency
                // You can add borders or other visual elements if desired
                // border.color: "lightgray"
                // border.width: 1
            }
            visible: root.isSelected
            onClicked: {
                RegimeManager.model.deleteRows([index])
            }
        }
    }
}