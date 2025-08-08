import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 540
    height: 60
    color: "#f0f0f0"

    property var model: null

    function formatTime(seconds) {
        var hours = Math.floor(seconds / 3600);
        var minutes = Math.floor((seconds % 3600) / 60);
        var secs = seconds % 60;
        return Qt.formatTime(new Date(0, 0, 0, hours, minutes, secs), "hh:mm:ss");
    }

    Row {
        id: progressBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 10
        height: 20

        Repeater {
            model: root.model

            delegate: Rectangle {
                width: (model.max_time * model.repeat) / RegimeManager.getTotalEstimatedTime() * progressBar.width
                height: progressBar.height
                color: {
                    switch (model.state) {
                        case 2: return "lightblue"; // Running
                        case 5: return "lightgreen"; // Done
                        case 4: return "lightgray"; // Skipped
                        case 6: return "lightcoral"; // Error
                        default: return "white"; // Waiting
                    }
                }

                Rectangle {
                    width: (model.time_passed_in_seconds / model.max_time) * parent.width
                    height: parent.height
                    color: "#3399ff"
                    visible: model.state === 2 // Running
                }
            }
        }
    }

    Label {
        id: timeLabel
        anchors.top: progressBar.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 5
        color: "black"
        text: formatTime(RegimeManager.getTotalElapsedTime()) + " / " + formatTime(RegimeManager.getTotalEstimatedTime())
    }
}