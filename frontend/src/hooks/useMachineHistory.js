import { useState, useEffect } from 'react'

const REFRESH_INTERVAL_MS = 60_000
const REQUEST_TIMEOUT_MS = 10_000

/**
 * Retrieve and periodically refresh historical measurements
 * for a specific machine.
 *
 * @param {number} machineId - Database ID of the machine.
 * @returns {{
 *     measurements: Object[],
 *     loading: boolean,
 *     error: string|null
 * }} Historical measurements and retrieval state.
 */
export function useMachineHistory(machineId) {
    const [measurements, setMeasurements] = useState([])
    const [loading, setLoading] = useState(true)
    const [error, setError] = useState(null)

    useEffect(() => {
        const controller = new AbortController()
        let timerId

        /**
         * Retrieve historical measurements and schedule
         * the next refresh.
         *
         * @returns {Promise<void>} Completion of the retrieval attempt.
         */
        function fetchHistory() {
            return fetch(`/api/machines/${machineId}/measurements`, {
                signal: AbortSignal.any([
                    controller.signal,
                    AbortSignal.timeout(REQUEST_TIMEOUT_MS)
                ])
            })
                .then((response) => {
                    if (!response.ok) {
                        throw new Error(`HTTP ${response.status}`)
                    }

                    return response.json()
                })
                .then((data) => {
                    if (!controller.signal.aborted) {
                        setMeasurements(data)
                        setError(null)
                    }
                })
                .catch((error) => {
                    if (!controller.signal.aborted) {
                        console.error('Failed to load machine history:', error)
                        setError('Historiske målinger kunne ikke hentes.')
                    }
                })
                .finally(() => {
                    if (!controller.signal.aborted) {
                        setLoading(false)

                        timerId = setTimeout(
                            fetchHistory,
                            REFRESH_INTERVAL_MS
                        )
                    }
                })
        }

        fetchHistory()

        return () => {
            clearTimeout(timerId)
            controller.abort()
        }
    }, [machineId])

    return { measurements, loading, error }
}