import {
    Chart as ChartJS,
    LinearScale,
    PointElement,
    LineElement,
    Tooltip,
    Legend
} from 'chart.js'
import { Line } from 'react-chartjs-2'
import { useMachineHistory } from '../hooks/useMachineHistory'
import { formatPercentage } from '../utils/formatters'

ChartJS.register(
    LinearScale,
    PointElement,
    LineElement,
    Tooltip,
    Legend
)

/**
 * Format a historical measurement timestamp.
 *
 * @param {number|string} timestamp - Measurement timestamp.
 * @returns {string} Danish date and time.
 */
function formatHistoryTimestamp(timestamp) {
    return new Date(timestamp).toLocaleString('da-DK', {
        day: '2-digit',
        month: '2-digit',
        hour: '2-digit',
        minute: '2-digit'
    })
}

/**
 * Display historical resource measurements and ranked CPU peaks
 * for a single machine.
 *
 * @param {Object} props - Component properties.
 * @param {number} props.machineId - Database ID of the machine.
 * @returns {import('react').ReactElement} Historical machine data.
 */
function MachineHistory({ machineId }) {
    const { measurements, loading, error } = useMachineHistory(machineId)

    const chronologicalMeasurements = [...measurements].reverse()

    const cpuPeaks = measurements
        .filter((measurement) =>
            Number.isInteger(measurement.cpu_rank) &&
            measurement.cpu_rank <= 3
        )
        .sort((a, b) => a.cpu_rank - b.cpu_rank)

    const metrics = [
        {
            label: 'CPU',
            property: 'cpu_percent',
            color: '#005e82'
        },
        {
            label: 'RAM',
            property: 'ram_percent',
            color: '#9467bd'
        },
        {
            label: 'Ledig disk',
            property: 'disk_free_percent',
            color: '#e68a00'
        }
    ]

    const chartData = {
        datasets: metrics.map((metric) => ({
            label: metric.label,
            data: chronologicalMeasurements.map((measurement) => ({
                x: new Date(measurement.recorded_at).getTime(),
                y: measurement[metric.property]
            })),
            borderColor: metric.color,
            backgroundColor: metric.color,
            borderWidth: 2,
            pointRadius: 3,
            tension: 0,
            spanGaps: 35 * 60_000
        }))
    }

    const chartOptions = {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        locale: 'da-DK',
        interaction: {
            mode: 'index',
            intersect: false
        },
        scales: {
            x: {
                type: 'linear',
                ticks: {
                    maxTicksLimit: 8,
                    callback: (value) =>
                        formatHistoryTimestamp(Number(value))
                }
            },
            y: {
                min: 0,
                max: 100,
                ticks: {
                    callback: (value) => `${value} %`
                }
            }
        },
        plugins: {
            legend: {
                position: 'bottom'
            },
            tooltip: {
                callbacks: {
                    title: (items) =>
                        formatHistoryTimestamp(items[0].parsed.x),
                    label: (item) =>
                        `${item.dataset.label}: ${formatPercentage(item.parsed.y)}`
                }
            }
        }
    }

    return (
        <section className="machine-history">
            <h4>Historiske målinger</h4>

            {loading && <p>Henter historiske målinger...</p>}

            {error && (
                <p role="alert">{error}</p>
            )}

            {!loading && !error && measurements.length === 0 && (
                <p>Ingen historiske målinger.</p>
            )}

            {measurements.length > 0 && (
                <>
                    <div className="machine-history-chart">
                        <Line
                            data={chartData}
                            options={chartOptions}
                        />
                    </div>

                    <p>Viser {measurements.length} seneste målinger.</p>

                    {cpuPeaks.length > 0 && (
                        <div className="machine-history-peaks">
                            <h4>Højeste CPU-målinger</h4>

                            <table>
                                <thead>
                                    <tr>
                                        <th>Placering</th>
                                        <th>Tidspunkt</th>
                                        <th>CPU</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    {cpuPeaks.map((measurement) => (
                                        <tr key={measurement.id}>
                                            <td>{measurement.cpu_rank}</td>
                                            <td>
                                                {formatHistoryTimestamp(
                                                    measurement.recorded_at
                                                )}
                                            </td>
                                            <td>
                                                {formatPercentage(
                                                    measurement.cpu_percent
                                                )}
                                            </td>
                                        </tr>
                                    ))}
                                </tbody>
                            </table>
                        </div>
                    )}
                </>
            )}
        </section>
    )
}

export default MachineHistory