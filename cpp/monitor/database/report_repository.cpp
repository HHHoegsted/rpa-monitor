
#include "report_repository.h"

#include <pqxx/pqxx>

#include <exception>
#include <iostream>

/**
 * Persist a complete machine health report to PostgreSQL.
 *
 * Registers the machine if it does not already exist, then
 * inserts its measurements within the same transaction.
 *
 * Args:
 *     report: Complete machine report to persist.
 *
 * Returns:
 *     True if the transaction commits successfully,
 *     otherwise false.
 */
bool save_machine_report(const MachineReport& report)
{
    if (!report.is_complete())
    {
        std::cerr << "Cannot save incomplete machine report.\n";
        return false;
    }

    try
    {
        pqxx::connection connection;
        pqxx::work transaction{connection};

        transaction.exec(
            "INSERT INTO machines (hostname) "
            "VALUES ($1) "
            "ON CONFLICT (hostname) DO NOTHING",
            pqxx::params{report.hostname()}
        );

        const auto machine = transaction.exec(
            "SELECT id FROM machines WHERE hostname = $1",
            pqxx::params{report.hostname()}
        ).one_row();

        const long long machine_id = machine[0].as<long long>();

        transaction.exec(
            "INSERT INTO measurements ("
            "machine_id, cpu_percent, ram_percent, "
            "disk_free_percent, robot_running, uptime_seconds"
            ") VALUES ($1, $2, $3, $4, $5, $6)",
            pqxx::params{
                machine_id,
                report.cpu_percent(),
                report.ram_percent(),
                report.disk_free_percent(),
                report.robot_running(),
                report.uptime_seconds()
            }
        );

        transaction.commit();

        return true;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Failed to save machine report: "
                  << error.what()
                  << '\n';

        return false;
    }
}
