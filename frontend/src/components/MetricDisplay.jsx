import './MetricDisplay.css'

/**
 * Render a labelled machine measurement.
 *
 * @param {Object} props - Component properties.
 * @param {string} props.label - Human-readable metric label.
 * @param {string} props.value - Formatted metric value.
 * @returns {import('react').ReactElement} The metric display.
 */
function MetricDisplay({ label, value }) {
    return (
        <div className="metric-display">
            <span>{label}</span>
            <strong>{value}</strong>
        </div>
    )
}

export default MetricDisplay