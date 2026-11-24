import MachineCard from './MachineCard'
import './MachineGrid.css'

/**
 * Render a collection of monitored machines.
 *
 * @param {Object} props - Component properties.
 * @param {Array<{
 *     id: number,
 *     hostname: string,
 *     recorded_at: string|null,
 *     cpu_percent: number|null,
 *     ram_percent: number|null,
 *     disk_free_percent: number|null,
 *     robot_running: boolean|null,
 *     uptime_seconds: number|null
 * }>} props.machines - Monitored machines and their measurements.
 * @returns {import('react').ReactElement} The machine grid.
 */
function MachineGrid({ machines }) {
    return (
        <div id="machines">
            {machines.map((machine) => (
                <MachineCard
                    key={machine.id}
                    hostname={machine.hostname}
                    machine={machine}
                />
            ))}
        </div>
    )
}

export default MachineGrid