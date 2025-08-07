import QtQuick
import QtQuick.Controls
import com.grams.prototable

TextField {
    id: stateDelegate
    function toHHMMSS(secs) { // seems working
        
        let newDate = new Date(secs * 1000).toISOString().slice(11, 19);
        // debug if need, Date use 1970 thing
        return newDate
    }
    function fromHHMMSS(str){
        // let date = new Date(str);
        let a = str.split(':'); // split it at the colons
        // minutes are worth 60 seconds. Hours are worth 60 minutes.
        let seconds = (+a[0]) * 60 * 60 + (+a[1]) * 60 + (+a[2]); 
        return seconds; //
    }
    readOnly: true
    color: "black"
    text: {
        switch (model.state) {
            case RegimeState.Running:
                return toHHMMSS(model.time_passed_in_seconds);
            case RegimeState.Paused:
                return toHHMMSS((model.max_time*60 - model.time_passed_in_seconds));
            case RegimeState.Skipped:
            case RegimeState.Done:
                return toHHMMSS(model.max_time*60);
            case RegimeState.Error:
                return "Error";
            case RegimeState.Stopped:
            default:
                return "Ожидание";
        }
    }

    background: Rectangle {
        color: {
            switch (model.state) {
                case RegimeState.Running:
                    return "#C8E6C9"; // Pastel Green
                case RegimeState.Paused:
                    return "#BBDEFB"; // Pastel Blue
                case RegimeState.Error:
                    return "#FFCDD2"; // Pastel Red
                case RegimeState.Skipped:
                case RegimeState.Done:
                    return "white";
                case RegimeState.Stopped:
                default:
                    return "transparent";
            }
        }
    }
}
