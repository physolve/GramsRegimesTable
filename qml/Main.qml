import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import com.grams.prototable
ApplicationWindow {
    width: 700
    height: 650
    visible: true
    title: qsTr("ProtoTable App")
    RunTable{
        width: parent.width
        height: parent.height
    }
}