import MetricDisplay from './MetricDisplay'
import {
    formatPercentage,
    formatUptime,
    formatMeasurementTime
} from '../utils/formatters'
import { getReportingStatus } from '../utils/machineStatus'
import './MachineCard.css'

/**
 * Render a monitoring card for a single machine.
 *
 * @param {Object} props - Component properties.
 * @param {Object} props.machine - Machine and its latest measurements.
 * @returns {import('react').ReactElement} The machine card.
 */
function MachineCard({ machine }) {
    const reportingStatus = getReportingStatus(machine.recorded_at)

    const reportingLabels = {
        reporting: 'Rapporterer',
        stale: 'Manglende rapportering',
        unknown: 'Ukendt status'
    }

    const robotStatus = machine.robot_running === null
        ? '–'
        : machine.robot_running ? 'Kører' : 'Ledig'

    return (
        <article className={`machine-card machine-card--${reportingStatus}`}>
            <div className="machine-identity">
                <h3>{machine.hostname}</h3>

                <span
                    className="machine-last-measurement"
                    title={reportingLabels[reportingStatus]}
                >
                    {formatMeasurementTime(machine.recorded_at)}
                </span>
            </div>

            <div className="machine-metrics">
                <MetricDisplay
                    label="CPU"
                    value={formatPercentage(machine.cpu_percent)}
                />

                <MetricDisplay
                    label="RAM"
                    value={formatPercentage(machine.ram_percent)}
                />

                <MetricDisplay
                    label="Ledig disk"
                    value={formatPercentage(machine.disk_free_percent)}
                />

                <MetricDisplay
                    label="Robot"
                    value={robotStatus}
                />

                <MetricDisplay
                    label="Oppetid"
                    value={formatUptime(machine.uptime_seconds)}
                />
            </div>
        </article>
    )
}

export default MachineCard