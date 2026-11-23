
/**
 * Format a percentage for display, using at most one decimal place.
 *
 * @param {number|null} value - Original percentage.
 * @returns {string} Formatted percentage or N/A.
 */
function formatPercentage(value) {
    if (value === null) {
        return "N/A";
    }

    return `${Number(value.toFixed(1))}%`;
}


/**
 * Format system uptime as days, hours, and minutes.
 *
 * @param {number|null} seconds - System uptime in seconds.
 * @returns {string} Human-readable uptime or N/A.
 */
function formatUptime(seconds) {
    if (seconds === null) {
        return "N/A";
    }

    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);

    return `${days}d ${hours}h ${minutes}m`;
}



/**
 * Load machine measurements and render monitoring cards.
 *
 * Reporting freshness is evaluated once when the page loads.
 *
 * @returns {Promise<void>} Resolves when the dashboard is updated.
 */
async function loadMachines() {
    const container = document.getElementById("machines");
    const staleAfterMs = 35 * 60 * 1000;

    try {
        const response = await fetch(
            "http://127.0.0.1:8000/machines/latest"
        );

        if (!response.ok) {
            throw new Error(`API returned HTTP ${response.status}`);
        }

        const machines = await response.json();
        const now = Date.now();

        container.replaceChildren();

        if (machines.length === 0) {
            container.textContent = "No machines registered.";
            return;
        }

        for (const machine of machines) {
            const card = document.createElement("article");
            card.className = "machine-card";

            // Determine reporting freshness.
            let freshness = "Never reported";

            if (machine.recorded_at) {
                const reportedAt = Date.parse(machine.recorded_at);
                const ageMs = now - reportedAt;

                if (Number.isFinite(reportedAt) && ageMs >= 0) {
                    if (ageMs < staleAfterMs) {
                        freshness = "Reporting";
                        card.classList.add("machine-card--reporting");
                    } else {
                        freshness = "Stale";
                        card.classList.add("machine-card--stale");
                    }
                } else {
                    freshness = "Unknown";
                }
            }

            const heading = document.createElement("h3");
            heading.textContent = machine.hostname;
            card.appendChild(heading);

            const status = document.createElement("p");
            status.className = "machine-status";

            status.textContent = machine.recorded_at
                ? `${freshness} · Last report: ${new Date(machine.recorded_at).toLocaleString()}`
                : "Never reported";

            card.appendChild(status);

            const metrics = document.createElement("div");
            metrics.className = "machine-metrics";

            const values = [
                ["CPU", formatPercentage(machine.cpu_percent)],
                ["RAM", formatPercentage(machine.ram_percent)],
                ["Disk free", formatPercentage(machine.disk_free_percent)],
                [
                    "Robot",
                    machine.robot_running === null
                        ? "N/A"
                        : machine.robot_running ? "Running" : "Idle"
                ],
                ["Uptime", formatUptime(machine.uptime_seconds)]
            ];

            for (const [label, value] of values) {
                const metric = document.createElement("div");

                const labelElement = document.createElement("div");
                labelElement.className = "metric-label";
                labelElement.textContent = label;

                const valueElement = document.createElement("div");
                valueElement.className = "metric-value";
                valueElement.textContent = value;

                metric.append(labelElement, valueElement);
                metrics.appendChild(metric);
            }

            card.appendChild(metrics);
            container.appendChild(card);
        }
    } catch (error) {
        console.error("Failed to load machines:", error);
        container.textContent = "Unable to load machine data.";
    }
}


loadMachines();
