import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "#f0f0f0"

    property var model: null
    property int visibleStartTime: 0
    property int visibleEndTime: RegimeManager.getTotalEstimatedTime()

    Component.onCompleted: {
        rangeSlider.to = RegimeManager.getTotalEstimatedTime();
        rangeSlider.second.value = RegimeManager.getTotalEstimatedTime();
        RegimeManager.updateVisibleRegimes(root.visibleStartTime, root.visibleEndTime);
    }

    Connections {
        target: RegimeManager
        function onTotalTimeChanged() {
            timeLabel.text = formatTime(RegimeManager.getTotalElapsedTime()) + " / " + formatTime(RegimeManager.getTotalEstimatedTime());
            rangeSlider.to = RegimeManager.getTotalEstimatedTime();
            rangeSlider.second.value = RegimeManager.getTotalEstimatedTime();
            RegimeManager.updateVisibleRegimes(root.visibleStartTime, root.visibleEndTime);
        }
    }

    function formatTime(seconds) {
        var hours = Math.floor(seconds / 3600);
        var minutes = Math.floor((seconds % 3600) / 60);
        var secs = seconds % 60;
        return Qt.formatTime(new Date(0, 0, 0, hours, minutes, secs), "hh:mm:ss");
    }

    Row {
        id: progressBar
        width: 540
        height: 20

        Repeater {
            id: repeater
            model: RegimeManager.visibleRegimeModel

            delegate: Rectangle {
                width: (model.maxTime * model.repeatCount) / (root.visibleEndTime - root.visibleStartTime) * progressBar.width
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
                    width: (model.timePassedInSeconds / model.maxTime) * parent.width
                    height: parent.height
                    color: "#3399ff"
                    visible: model.state === 2 // Running
                }

                Text {
                    text: model.name
                    anchors.centerIn: parent
                    color: "black"
                    font.pixelSize: 10
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

    RangeSlider {
        id: rangeSlider
        anchors.top: timeLabel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        from: 0
        to: RegimeManager.getTotalEstimatedTime()
        first.value: root.visibleStartTime
        second.value: root.visibleEndTime
        first.onMoved:{
            root.visibleStartTime = first.value
            RegimeManager.updateVisibleRegimes(root.visibleStartTime, root.visibleEndTime)    
        }
        // onFirstValueChanged: {
        //     root.visibleStartTime = first.value
        //     RegimeManager.updateVisibleRegimes(root.visibleStartTime, root.visibleEndTime)
        // }
        second.onMoved:{
            root.visibleEndTime = second.value
            RegimeManager.updateVisibleRegimes(root.visibleStartTime, root.visibleEndTime)    
        }
        // onSecondValueChanged: {
        //     root.visibleEndTime = second.value
        //     RegimeManager.updateVisibleRegimes(root.visibleStartTime, root.visibleEndTime)
        // }
    }
}