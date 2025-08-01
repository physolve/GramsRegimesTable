.pragma library

function formatTime(minutes) {
    if (typeof minutes !== 'number' || minutes < 0) {
        return "00:00:00";
    }

    var totalSeconds = Math.round(minutes * 60);
    var hours = Math.floor(totalSeconds / 3600);
    var remainingSeconds = totalSeconds % 3600;
    var mins = Math.floor(remainingSeconds / 60);
    var secs = Math.round(remainingSeconds % 60);

    var pad = function(num) {
        var s = String(num);
        while (s.length < 2) {s = "0" + s;}
        return s;
    };

    return pad(hours) + ":" + pad(mins) + ":" + pad(secs);
}

function timeToMinutes(timeString) {
    if (typeof timeString !== 'string' || !/^\d{1,2}:\d{2}:\d{2}$/.test(timeString)) {
        return 0;
    }
    var parts = timeString.split(':');
    var hours = parseInt(parts[0], 10);
    var minutes = parseInt(parts[1], 10);
    var seconds = parseInt(parts[2], 10);

    if (isNaN(hours) || isNaN(minutes) || isNaN(seconds)) {
        return 0;
    }

    return (hours * 60) + minutes + (seconds / 60.0);
}
