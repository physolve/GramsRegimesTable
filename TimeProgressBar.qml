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
    
    Connections {
        target: RegimeManager
        function onRegimeDataUpdated() {
            // Force immediate refresh of timeline when regime data changes
            updateTimeRange()
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
                        // Each entry now represents a single repeat, so use maxTime directly
                        return model.maxTime / visibleDuration * baseWidth * timelineScale
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
                    
                    border.color: model.isCycle ? "#ff8c00" : "#c0c0c0"  // Orange border for cycles
                    border.width: model.isCycle ? 2 : 1                   // Thicker border for cycles

                    // Each rectangle now represents a single repeat
                    // Condition progress indicator (if condition time exists)
                    Rectangle {
                        id: conditionProgress
                        width: {
                            if (model.conditionTime > 0 && model.state === 2) { // Running
                                var conditionTimePassed = model.conditionTimePassed || 0
                                var conditionProgressRatio = conditionTimePassed / model.conditionTime
                                var conditionWidthRatio = model.conditionTime / model.maxTime
                                return Math.min(conditionProgressRatio, 1.0) * conditionWidthRatio * parent.width
                            }
                            return 0
                        }
                        height: parent.height
                        color: "#ff9500"  // Orange color for condition progress
                        opacity: 0.8
                        visible: model.conditionTime > 0 && model.state === 2 // Only show when running and has condition
                    }
                    
                    // Regime execution progress indicator (starts after condition)
                    Rectangle {
                        id: regimeProgress
                        x: {
                            if (model.conditionTime > 0) {
                                var conditionWidthRatio = model.conditionTime / model.maxTime
                                return conditionWidthRatio * parent.width
                            }
                            return 0
                        }
                        width: {
                            if (model.maxTime > 0 && model.state === 2) { // Running
                                var regimeTimePassed = model.regimeTimePassed || 0
                                var regimeExecutionTime = model.regimeExecutionTime
                                
                                if (regimeExecutionTime > 0) {
                                    var regimeProgressRatio = regimeTimePassed / regimeExecutionTime
                                    var regimeWidthRatio = regimeExecutionTime / model.maxTime
                                    return Math.min(regimeProgressRatio, 1.0) * regimeWidthRatio * parent.width
                                }
                            }
                            return 0
                        }
                        height: parent.height
                        color: "#3399ff"  // Blue color for regime execution progress
                        opacity: 0.7
                        visible: model.state === 2 // Running
                    }
                    
                    // Separator line between condition and regime execution (if condition exists)
                    Rectangle {
                        x: {
                            if (model.conditionTime > 0) {
                                var conditionWidthRatio = model.conditionTime / model.maxTime
                                return conditionWidthRatio * parent.width - 1
                            }
                            return 0
                        }
                        width: 2
                        height: parent.height
                        color: "#333333"  // Dark separator line
                        opacity: 0.6
                        visible: model.conditionTime > 0
                    }

                    // Regime name text with repeat info
                    Text {
                        text: {
                            var baseName = model.name
                            var repeatInfo = "R" + (model.repeatIndex + 1)
                            if (model.isCycleEntry) {
                                repeatInfo += "/C" + (model.cycleRepeatIndex + 1)
                            }
                            return baseName + "\n" + repeatInfo
                        }
                        anchors.centerIn: parent
                        color: "black"
                        font.pixelSize: Math.max(6, Math.min(10, parent.width / 12))
                        elide: Text.ElideRight
                        width: parent.width - 4
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                    }
                    
                    // Tooltip on hover
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        
                        ToolTip {
                            contentWidth: 200
                            visible: parent.containsMouse
                            text: {
                                var tooltip = `${model.name}\nRepeat: ${model.repeatIndex + 1}`
                                
                                if (model.isCycleEntry) {
                                    tooltip += ` (Cycle ${model.cycleRepeatIndex + 1})`
                                }
                                
                                tooltip += `\nDuration: ${formatTime(model.maxTime)}\nState: ${getStateName(model.state)}`
                                
                                // Add progress information
                                if (model.conditionTime > 0) {
                                    if (model.conditionCompleted) {
                                        tooltip += `\nCondition: ✓ Complete`
                                        tooltip += `\nExecution Progress: ${formatTime(model.regimeTimePassed || 0)} / ${formatTime(model.regimeExecutionTime)}`
                                    } else {
                                        tooltip += `\nCondition Progress: ${formatTime(model.conditionTimePassed || 0)} / ${formatTime(model.conditionTime)}`
                                    }
                                } else {
                                    tooltip += `\nExecution Progress: ${formatTime(model.regimeTimePassed || 0)} / ${formatTime(model.regimeExecutionTime)}`
                                }
                                
                                // Add condition information if available
                                var regime = RegimeManager.model.getRegime(index)
                                if (regime && regime.condition) {
                                    if (regime.condition.type === "time") {
                                        tooltip += `\nCondition: Wait ${regime.condition.time} min`
                                    } else if (regime.condition.type === "temp") {
                                        tooltip += `\nCondition: ${regime.condition.temp}°C + ${regime.condition.time} min`
                                    } else {
                                        tooltip += `\nCondition: None`
                                    }
                                }
                                
                                // Add execution time breakdown
                                if (model.conditionTime > 0) {
                                    tooltip += `\nCondition Time: ${formatTime(model.conditionTime)}`
                                    tooltip += `\nExecution Time: ${formatTime(model.regimeExecutionTime)}`
                                }
                                
                                // Add repeat statistics if any completed
                                if (regime && (regime.repeatsDone > 0 || regime.repeatsSkipped > 0 || regime.repeatsError > 0)) {
                                    tooltip += `\nCompleted: ${regime.repeatsDone}, Skipped: ${regime.repeatsSkipped}, Errors: ${regime.repeatsError}`
                                }
                                
                                if (model.isCycle) {
                                    tooltip += `\nCycle ID: ${model.cycleId}\nType: Cycle`
                                } else {
                                    tooltip += `\nType: Individual Regime`
                                }
                                return tooltip
                            }
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