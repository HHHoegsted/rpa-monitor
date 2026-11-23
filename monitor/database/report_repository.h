
#pragma once

#include "../models/machine_report.h"

/**
 * Persist a complete machine health report to PostgreSQL.
 *
 * Args:
 *     report: Complete machine report to persist.
 *
 * Returns:
 *     True if the report was saved successfully,
 *     otherwise false.
 */
bool save_machine_report(const MachineReport& report);
