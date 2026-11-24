import { useState, useEffect } from 'react'

const REFRESH_INTERVAL_MS = 60_000
const REQUEST_TIMEOUT_MS = 10_000

/**
 * Retrieve and periodically refresh the latest machine measurements.
 *
 * @returns {{
 *     machines: Object[],
 *     loading: boolean,
 *     error: string|null
 * }} Current machine data and retrieval state.
 */
export function useMachines() {
    const [machines, setMachines] = useState([])
    const [loading, setLoading] = useState(true)
    const [error, setError] = useState(null)

    useEffect(() => {
        const controller = new AbortController()
        let timerId

        /**
         * Retrieve machine data and schedule the next refresh.
         *
         * @returns {Promise<void>} Completion of the retrieval attempt.
         */
        function fetchMachines() {
            return fetch('/api/machines/latest', {
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
                        setMachines(data)
                        setError(null)
                    }
                })
                .catch((error) => {
                    if (!controller.signal.aborted) {
                        console.error('Failed to load machines:', error)
                        setError('Maskindata kunne ikke hentes.')
                    }
                })
                .finally(() => {
                    if (!controller.signal.aborted) {
                        setLoading(false)

                        timerId = setTimeout(
                            fetchMachines,
                            REFRESH_INTERVAL_MS
                        )
                    }
                })
        }

        fetchMachines()

        return () => {
            clearTimeout(timerId)
            controller.abort()
        }
    }, [])

    return { machines, loading, error }
}