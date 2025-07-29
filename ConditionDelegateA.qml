import QtQuick
import QtQuick.Controls

/*Example of condition delegate*/
Row{
    required property bool selected
    required property bool current
    required property var condition

    ComboBox {
        model: ["temp", "time", "both"]
        width: parent.width/2
        currentIndex: condition.type == "temp" ? 0 : (condition.type == "time" ? 1 : 2)
        font.pointSize: 9
    }
    TextField{
        font.pointSize: 9
        validator: DoubleValidator { bottom: 0; top: 1000} //; decimals: 3
        selectByMouse: true
        width: parent.width/4
        horizontalAlignment: TextInput.AlignHCenter
        placeholderText: "°C"
        text: condition.temp
        enabled: condition.type != "time"
    }
    TextField{
        font.pointSize: 9
        validator: DoubleValidator { bottom: 0; top: 1000} //; decimals: 3
        selectByMouse: true
        width: parent.width/4
        horizontalAlignment: TextInput.AlignHCenter
        placeholderText: "мин"
        text: condition.time
        enabled: condition.type != "temp"
    }
}