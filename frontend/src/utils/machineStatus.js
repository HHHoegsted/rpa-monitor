const STALE_AFTER_MS = 35 * 60 * 1000

/**
 * Determine the reporting status of a machine based on
 * the timestamp of its latest measurement.
 *
 * @param {string|null} recordedAt - Latest measurement timestamp.
 * @returns {'reporting'|'stale'|'unknown'} Machine reporting status.
 */
export function getReportingStatus(recordedAt) {
    if (!recordedAt) {
        return 'unknown'
    }

    const reportedAt = Date.parse(recordedAt)
    const ageMs = Date.now() - reportedAt

    if (!Number.isFinite(reportedAt) || ageMs < 0) {
        return 'unknown'
    }

    return ageMs < STALE_AFTER_MS
        ? 'reporting'
        : 'stale'
}