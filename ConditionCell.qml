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
        model: ["temp", "time", "both"]
        width: parent.width / 2
        font.pointSize: 9
        // function fromIndex(index) {
        //     switch (index) {
        //         case 0: return "temp"
        //         case 1: return "time"
        //         case 2: return "both"
        //     }
        // }
        currentIndex: container.model.condition.type === "temp" ? 0 : (container.model.condition.type === "time" ? 1 : 2)
        onCurrentTextChanged:{
            Qt.callLater(makeChanges)
        }
        function makeChanges(){
            let newCond = container.model.condition
            newCond.type = typeComboBox.currentText
            container.model.condition = newCond
        }
    }

    TextField {
        id: tempTextField
        font.pointSize: 9
        validator: DoubleValidator { bottom: 0; top: 1000; decimals: 3 }
        selectByMouse: true
        width: parent.width / 4
        horizontalAlignment: TextInput.AlignHCenter
        placeholderText: "°C"
        text: container.model.condition.temp
        enabled: typeComboBox.currentText === "temp" || typeComboBox.currentText === "both"
        onEditingFinished: {
            let newCond = container.model.condition
            newCond.temp = tempTextField.text
            container.model.condition = newCond
        }
    }

    TextField {
        id: timeTextField
        font.pointSize: 9
        validator: IntValidator { bottom: 0; top: 1000 }
        selectByMouse: true
        width: parent.width / 4
        horizontalAlignment: TextInput.AlignHCenter
        placeholderText: "мин"
        text: container.model.condition.time
        enabled: typeComboBox.currentText === "time" || typeComboBox.currentText === "both"
        onEditingFinished: {
            let newCond = container.model.condition
            newCond.time = timeTextField.text
            container.model.condition = newCond
        }
    }
}