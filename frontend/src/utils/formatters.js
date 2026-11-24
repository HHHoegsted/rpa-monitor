/**
 * Format a percentage using Danish number conventions
 * and at most one decimal place.
 *
 * @param {number|null} value - Percentage to format.
 * @returns {string} Formatted percentage or an unavailable indicator.
 */
export function formatPercentage(value) {
    if (value === null || !Number.isFinite(value)) {
        return '–'
    }

    return `${value.toLocaleString('da-DK', {
        maximumFractionDigits: 1
    })} %`
}

/**
 * Format system uptime as days, hours and minutes.
 *
 * @param {number|null} seconds - System uptime in seconds.
 * @returns {string} Formatted uptime or an unavailable indicator.
 */
export function formatUptime(seconds) {
    if (seconds === null || !Number.isFinite(seconds) || seconds < 0) {
        return '–'
    }

    const days = Math.floor(seconds / 86400)
    const hours = Math.floor((seconds % 86400) / 3600)
    const minutes = Math.floor((seconds % 3600) / 60)

    return `${days}d ${hours}t ${minutes}m`
}

/**
 * Format a measurement as a time if recorded today,
 * or as a date if recorded on another day.
 *
 * @param {string|null} recordedAt - ISO-formatted measurement timestamp.
 * @returns {string} Formatted measurement time or date.
 */
export function formatMeasurementTime(recordedAt) {
    if (!recordedAt) {
        return 'Ingen målinger'
    }

    const date = new Date(recordedAt)

    if (Number.isNaN(date.getTime())) {
        return 'Ukendt tidspunkt'
    }

    const today = new Date()

    if (date.toDateString() === today.toDateString()) {
        return date.toLocaleTimeString('da-DK', {
            hour: '2-digit',
            minute: '2-digit'
        })
    }

    return date.toLocaleDateString('da-DK')
}