import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: 820
    height: 130
    color: "#f0f0f0"
    property var model: null
    property int visibleStartTime: 0
    property int visibleEndTime: RegimeManager.getTotalEstimatedTime()
    property real timelineScale: 1.0  // Scale factor for timeline width
    
    Component.onCompleted: {
        updateTimeRange()
        RegimeManager.updateVisibleRegimes(root.visibleStartTime, root.visibleEndTime)
    }

    Connections {
        target: RegimeManager.visibleRegimeModel
        function onTimelineUpdateRequired() {
            updateTimeRange()
            var totalTime = RegimeManager.getTotalEstimatedTime()
            if (totalTime > 0) {
                root.visibleStartTime = 0
                root.visibleEndTime = totalTime
                startTimeField.text = formatTime(0)
                endTimeField.text = formatTime(totalTime)
                updateVisibleRange()
            }
        }
    }
    
    function updateTimeRange() {
        var totalTime = RegimeManager.getTotalEstimatedTime()
        if (totalTime > 0) {
            // Reset scale to 1.0 (perfect fit) on any update
            timelineScale = 1.0
            scaleSpinBox.value = 1
            
            // Update end time if it's currently at max or unset
            if (root.visibleEndTime >= totalTime || root.visibleEndTime === 0) {
                root.visibleEndTime = totalTime
                endTimeField.text = formatTime(totalTime)
            }
            // Update start time field
            startTimeField.text = formatTime(root.visibleStartTime)
            
            // Update time label
            timeLabel.text = formatTime(RegimeManager.getTotalElapsedTime()) + " / " + formatTime(totalTime)
            
            // Update visible regimes
            RegimeManager.updateVisibleRegimes(root.visibleStartTime, root.visibleEndTime)
        }
    }

    function formatTime(seconds) {
        var hours = Math.floor(seconds / 3600)
        var minutes = Math.floor((seconds % 3600) / 60)
        var secs = seconds % 60
        return Qt.formatTime(new Date(0, 0, 0, hours, minutes, secs), "hh:mm:ss")
    }
    
    function timeToSeconds(timeString) {
        var parts = timeString.split(':')
        if (parts.length !== 3) {
            return 0
        }
        var hours = parseInt(parts[0], 10)
        var minutes = parseInt(parts[1], 10)
        var seconds = parseInt(parts[2], 10)
        if (isNaN(hours) || isNaN(minutes) || isNaN(seconds)) {
            return 0
        }
        return hours * 3600 + minutes * 60 + seconds
    }
    
    function updateVisibleRange() {
        // Reset scale to 1.0 when time range changes
        timelineScale = 1.0
        scaleSpinBox.value = 1
        RegimeManager.updateVisibleRegimes(root.visibleStartTime, root.visibleEndTime)
    }

    // ScrollView containing the timeline
    ScrollView {
        id: scrollView
        implicitWidth: parent.width
        // width: parent.width
        height: 50
        clip: true
        
        // Calculate content width: at scale 1.0, all visible regimes fit in available width
        contentWidth: {
            var availableWidth = root.width  // Account for margins
            return Math.max(availableWidth, availableWidth * timelineScale)
        }
        
        ScrollBar.horizontal.policy: ScrollBar.AsNeeded
        
        // Timeline content
        Row {
            y: 5
            id: progressBar
            width: scrollView.contentWidth
            height: 30
            spacing: 1
            
            Repeater {
                id: repeater
                model: RegimeManager.visibleRegimeModel

                delegate: Rectangle {
                    width: {
                        var visibleDuration = root.visibleEndTime - root.visibleStartTime
                        if (visibleDuration <= 0) return 0
                        var baseWidth = root.width  // Same as available width calculation
                        return (model.maxTime * model.repeatCount) / visibleDuration * baseWidth * timelineScale
                    }
                    height: progressBar.height
                    
                    color: {
                        switch (model.state) {
                            case 2: return "lightblue"   // Running
                            case 5: return "lightgreen"  // Done
                            case 4: return "lightgray"   // Skipped
                            case 6: return "lightcoral"  // Error
                            default: return "white"      // Waiting
                        }
                    }
                    
                    border.color: "#c0c0c0"
                    border.width: 1

                    // Progress indicator for running regimes
                    Rectangle {
                        width: model.maxTime > 0 ? (model.timePassedInSeconds / model.maxTime) * parent.width : 0
                        height: parent.height
                        color: "#3399ff"
                        opacity: 0.7
                        visible: model.state === 2 // Running
                    }

                    // Regime name text
                    Text {
                        text: model.name
                        anchors.centerIn: parent
                        color: "black"
                        font.pixelSize: Math.max(8, Math.min(12, parent.width / 8))
                        elide: Text.ElideRight
                        width: parent.width - 4
                        horizontalAlignment: Text.AlignHCenter
                    }
                    
                    // Tooltip on hover
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        
                        ToolTip {
                            contentWidth: 100
                            visible: parent.containsMouse
                            text: `${model.name}\nDuration: ${formatTime(model.maxTime)} \nRepeats:  ${model.repeatCount}\nState: ${getStateName(model.state)}`
                            delay: 500
                        }
                    }
                }
            }
        }
    }

    // Time label below the timeline
    Label {
        id: timeLabel
        y: 50
        width: parent.width
        color: "black"
        text: formatTime(RegimeManager.getTotalElapsedTime()) + " / " + formatTime(RegimeManager.getTotalEstimatedTime())
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    // Time range controls
    RowLayout {
        id: timeControls
        x: 10
        y: 80
        width: parent.width - 20
        spacing: 10
        
        Label {
            text: "Start:"
            color: "black"
        }
        
        TextField {
            id: startTimeField
            Layout.preferredWidth: 80
            text: formatTime(root.visibleStartTime)
            inputMask: "99:99:99"
            inputMethodHints: Qt.ImhTime
            horizontalAlignment: TextInput.AlignHCenter
            
            validator: RegularExpressionValidator {
                regularExpression: /^([01]?[0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9]$/
            }
            
            onEditingFinished: {
                if (acceptableInput) {
                    var newStartTime = timeToSeconds(text)
                    var totalTime = RegimeManager.getTotalEstimatedTime()
                    
                    // Validate range: start must be >= 0 and < end time
                    if (newStartTime >= 0 && newStartTime < root.visibleEndTime && newStartTime <= totalTime) {
                        root.visibleStartTime = newStartTime
                        updateVisibleRange()
                    } else {
                        // Reset to previous valid value
                        text = formatTime(root.visibleStartTime)
                    }
                } else {
                    // Reset to previous valid value
                    text = formatTime(root.visibleStartTime)
                }
            }
        }
        
        Label {
            text: "End:"
            color: "black"
        }
        
        TextField {
            id: endTimeField
            Layout.preferredWidth: 80
            text: formatTime(root.visibleEndTime)
            inputMask: "99:99:99"
            inputMethodHints: Qt.ImhTime
            horizontalAlignment: TextInput.AlignHCenter
            
            validator: RegularExpressionValidator {
                regularExpression: /^([01]?[0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9]$/
            }
            
            onEditingFinished: {
                if (acceptableInput) {
                    var newEndTime = timeToSeconds(text)
                    var totalTime = RegimeManager.getTotalEstimatedTime()
                    
                    // Validate range: end must be > start time and <= total time
                    if (newEndTime > root.visibleStartTime && newEndTime <= totalTime) {
                        root.visibleEndTime = newEndTime
                        updateVisibleRange()
                    } else {
                        // Reset to previous valid value
                        text = formatTime(root.visibleEndTime)
                    }
                } else {
                    // Reset to previous valid value
                    text = formatTime(root.visibleEndTime)
                }
            }
        }
        
        // Scale control for timeline zoom
        Label {
            text: "Scale:"
            color: "black"
        }
        
        SpinBox {
            id: scaleSpinBox
            Layout.preferredWidth: 80
            from: 1
            to: 100
            value: Math.round(timelineScale)
            
            onValueModified: {
                timelineScale = value
            }
        }
        
        // Quick action buttons
        Button {
            text: "Fit All"
            onClicked: {
                var totalTime = RegimeManager.getTotalEstimatedTime()
                if (totalTime > 0) {
                    root.visibleStartTime = 0
                    root.visibleEndTime = totalTime
                    startTimeField.text = formatTime(0)
                    endTimeField.text = formatTime(totalTime)
                    updateVisibleRange()
                }
            }
        }
        
        Button {
            text: "Reset Scale"
            onClicked: {
                timelineScale = 1.0
                scaleSpinBox.value = 1
            }
        }
    }
    
    // Helper function to get state name for tooltip
    function getStateName(state) {
        switch (state) {
            case 0: return "Waiting"
            case 1: return "Stopped"
            case 2: return "Running"
            case 3: return "Paused"
            case 4: return "Skipped"
            case 5: return "Done"
            case 6: return "Error"
            default: return "Unknown"
        }
    }
}