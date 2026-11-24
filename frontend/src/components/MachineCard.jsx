import { useState } from 'react'
import MetricDisplay from './MetricDisplay'
import MachineHistory from './MachineHistory'
import {
    formatPercentage,
    formatUptime,
    formatMeasurementTime
} from '../utils/formatters'
import { getReportingStatus } from '../utils/machineStatus'
import './MachineCard.css'

/**
 * Render a monitoring card with a clickable summary
 * and expandable historical measurements.
 *
 * @param {Object} props - Component properties.
 * @param {Object} props.machine - Machine and its latest measurements.
 * @returns {import('react').ReactElement} The machine card.
 */
function MachineCard({ machine }) {
    const [historyExpanded, setHistoryExpanded] = useState(false)

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
            <div className="machine-summary">
                <div className="machine-identity">
                    <h3>{machine.hostname}</h3>

                    <span className="machine-last-measurement">
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

                <button
                    type="button"
                    className="machine-history-toggle"
                    title={reportingLabels[reportingStatus]}
                    aria-expanded={historyExpanded}
                    aria-label={`${historyExpanded ? 'Skjul' : 'Vis'} historik for ${machine.hostname}. ${reportingLabels[reportingStatus]}`}
                    onClick={() =>
                        setHistoryExpanded((previous) => !previous)
                    }
                >
                    <svg
                        className="machine-history-chevron"
                        viewBox="0 0 24 24"
                        width="20"
                        height="20"
                        fill="none"
                        stroke="currentColor"
                        strokeWidth="2"
                        strokeLinecap="round"
                        strokeLinejoin="round"
                        aria-hidden="true"
                    >
                        <path d="m6 9 6 6 6-6" />
                    </svg>
                </button>
            </div>

            {historyExpanded && (
                <MachineHistory machineId={machine.id} />
            )}
        </article>
    )
}

export default MachineCard