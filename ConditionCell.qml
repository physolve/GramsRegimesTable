import QtQuick
import QtQuick.Controls

/*Example of condition delegate*/
Row{
    required property bool selected
    required property bool current
    required property var condition

    signal conditionChanged(var newCondition)

    ComboBox {
        id: typeComboBox
        model: ["temp", "time", "both"]
        width: parent.width/2
        currentIndex: condition.type == "temp" ? 0 : (condition.type == "time" ? 1 : 2)
        font.pointSize: 9

        onCurrentIndexChanged: {
            var newCondition = {
                "type": model[currentIndex],
                "temp": tempTextField.text,
                "time": timeTextField.text
            };
            conditionChanged(newCondition);
        }
    }
    TextField{
        id: tempTextField
        font.pointSize: 9
        validator: DoubleValidator { bottom: 0; top: 1000} //; decimals: 3
        selectByMouse: true
        width: parent.width/4
        horizontalAlignment: TextInput.AlignHCenter
        placeholderText: "°C"
        text: condition.temp
        enabled: typeComboBox.currentIndex !== 1 // Not "time"

        onEditingFinished: {
            var newCondition = {
                "type": typeComboBox.model[typeComboBox.currentIndex],
                "temp": text,
                "time": timeTextField.text
            };
            conditionChanged(newCondition);
        }
    }
    TextField{
        id: timeTextField
        font.pointSize: 9
        validator: DoubleValidator { bottom: 0; top: 1000} //; decimals: 3
        selectByMouse: true
        width: parent.width/4
        horizontalAlignment: TextInput.AlignHCenter
        placeholderText: "мин"
        text: condition.time
        enabled: typeComboBox.currentIndex !== 0 // Not "temp"

        onEditingFinished: {
            var newCondition = {
                "type": typeComboBox.model[typeComboBox.currentIndex],
                "temp": tempTextField.text,
                "time": text
            };
            conditionChanged(newCondition);
        }
    }
}