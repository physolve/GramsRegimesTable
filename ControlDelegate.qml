import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.grams.prototable

Rectangle {
    id: root
    width: 250
    border.color: "black"
    border.width: 1
    color: isSelected ? "lightblue" : "white"
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
            value: model.cycle_repeat
            onValueModified: model.cycle_repeat = value
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
            y: parent.height/2 - 20
            height: 40
            id: upButton
            text: "Up"
            visible: RegimeManager.model.isMoveUpEnabled([index])
            onClicked: {
                RegimeManager.model.moveSelection([index], true)
            }
        }

        Button {
            y: parent.height/2 - 20
            height: 40
            id: downButton
            text: "Down"
            visible: RegimeManager.model.isMoveDownEnabled([index])
            onClicked: {
                RegimeManager.model.moveSelection([index], false)
            }
        }

        Button {
            y: parent.height/2 - 20
            height: 40
            id: deleteButton
            text: "Delete"
            visible: root.isSelected
            onClicked: {
                RegimeManager.model.deleteRows([index])
            }
        }
    }
}