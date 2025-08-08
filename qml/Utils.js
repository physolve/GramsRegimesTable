function formatTime(seconds) {
    if (isNaN(seconds) || seconds < 0) {
        return "00:00:00";
    }

    const h = Math.floor(seconds / 3600).toString().padStart(2, '0');
    const m = Math.floor((seconds % 3600) / 60).toString().padStart(2, '0');
    const s = Math.floor(seconds % 60).toString().padStart(2, '0');
    return `${h}:${m}:${s}`;
}

function timeToMinutes(timeString) {
    const parts = timeString.split(':');
    if (parts.length !== 3) {
        return 0;
    }
    const hours = parseInt(parts[0], 10);
    const minutes = parseInt(parts[1], 10);
    const seconds = parseInt(parts[2], 10);
    if (isNaN(hours) || isNaN(minutes) || isNaN(seconds)) {
        return 0;
    }
    return hours * 60 + minutes + seconds / 60;
}

function timeToSeconds(timeString) {
    const parts = timeString.split(':');
    if (parts.length !== 3) {
        return 0;
    }
    const hours = parseInt(parts[0], 10);
    const minutes = parseInt(parts[1], 10);
    const seconds = parseInt(parts[2], 10);
    if (isNaN(hours) || isNaN(minutes) || isNaN(seconds)) {
        return 0;
    }
    return hours * 3600 + minutes * 60 + seconds;
}