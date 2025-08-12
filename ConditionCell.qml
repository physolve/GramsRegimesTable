import QtQuick
import QtQuick.Controls

Row {
    id: container
    required property int row
    required property int column
    required property bool current
    required property bool editing
    required property bool selected
    required property var model
    ComboBox {
        id: typeComboBox
        model: ["none", "time", "temp"]
        width: 70
        font.pointSize: 9
        
        currentIndex: {
            switch (container.model.condition.type) {
                case "none": return 0
                case "time": return 1
                case "temp": return 2
                default: return 0  // Default to "none"
            }
        }
        
        onCurrentTextChanged: {
            Qt.callLater(makeChanges)
        }
        
        function makeChanges() {
            let newCond = container.model.condition
            newCond.type = typeComboBox.currentText
            
            // Auto-set values to 0 when disabled
            if (typeComboBox.currentText === "none") {
                newCond.temp = 0
                newCond.time = 0
            } else if (typeComboBox.currentText === "time") {
                newCond.temp = 0  // Disable temp when only time is selected
            }
            // For "temp" option, both fields remain enabled
            
            container.model.condition = newCond
        }
    }

    TextField {
        id: tempTextField
        width: 40
        font.pointSize: 9
        validator: DoubleValidator { bottom: 0; top: 1000; decimals: 3 }
        selectByMouse: true
        horizontalAlignment: TextInput.AlignHCenter
        placeholderText: "°C"
        text: container.model.condition.temp
        enabled: typeComboBox.currentText === "temp"
        opacity: enabled ? 1.0 : 0.5  // Visual feedback for disabled state
        onEditingFinished: {
            if (enabled) {
                let newCond = container.model.condition
                newCond.temp = parseFloat(tempTextField.text) || 0
                container.model.condition = newCond
            }
        }
    }

    TextField {
        id: timeTextField
        width: 40
        font.pointSize: 9
        validator: IntValidator { bottom: 0; top: 1000 }
        selectByMouse: true
        horizontalAlignment: TextInput.AlignHCenter
        placeholderText: "мин"
        text: container.model.condition.time
        enabled: typeComboBox.currentText === "time" || typeComboBox.currentText === "temp"
        opacity: enabled ? 1.0 : 0.5  // Visual feedback for disabled state
        onEditingFinished: {
            if (enabled) {
                let newCond = container.model.condition
                newCond.time = parseInt(timeTextField.text) || 0
                container.model.condition = newCond
            }
        }
    }
}