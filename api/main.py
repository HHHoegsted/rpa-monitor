
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from .database import get_database_connection


app = FastAPI(title="RPA Monitor API")


app.add_middleware(
    CORSMiddleware,
    allow_origins=[
        "http://127.0.0.1:5500",
        "http://localhost:5500",
    ],
    allow_methods=["GET"],
    allow_headers=["*"],
)



@app.get("/health")
def health() -> dict[str, str]:
    """
    Report whether the API application is running.

    Returns:
        dict[str, str]: The application's health status.
    """
    return {"status": "ok"}

@app.get("/machines")
def get_machines() -> list[dict[str, int | str]]:
    """
    Retrieve all registered monitoring machines.

    Returns:
        list[dict[str, int | str]]: Machine IDs and hostnames,
            ordered alphabetically by hostname.
    """
    with get_database_connection() as connection:
        rows = connection.execute(
            "SELECT id, hostname FROM machines ORDER BY hostname"
        ).fetchall()

    return [
        {"id": row[0], "hostname": row[1]}
        for row in rows
    ]

@app.get("/machines/{machine_id}/measurements")
def get_machine_measurements(
    machine_id: int,
) -> list[dict[str, int | float | bool | str]]:
    """
    Retrieve the 100 most recent measurements for a machine.

    Args:
        machine_id: Database ID of the machine.

    Returns:
        list[dict[str, int | float | bool | str]]:
            Measurements ordered from newest to oldest.
            Returns an empty list when no measurements exist.
    """
    with get_database_connection() as connection:
        rows = connection.execute(
            """
            SELECT
                r.id,
                r.recorded_at,
                r.cpu_percent,
                r.ram_percent,
                r.disk_free_percent,
                r.robot_running,
                r.uptime_seconds
            FROM measurements AS r
            JOIN machines AS m ON m.id = r.machine_id
            WHERE m.id = %s
            ORDER BY r.recorded_at DESC, r.id DESC
            LIMIT 100
            """,
            (machine_id,),
        ).fetchall()

    return [
        {
            "id": row[0],
            "recorded_at": row[1].isoformat(),
            "cpu_percent": row[2],
            "ram_percent": row[3],
            "disk_free_percent": row[4],
            "robot_running": row[5],
            "uptime_seconds": row[6],
        }
        for row in rows
    ]

@app.get("/machines/latest")
def get_latest_machine_statuses() -> list[dict[str, int | float | bool | str | None]]:
    """
    Retrieve the latest recorded measurements for all registered machines.

    Includes machines that have never submitted a measurement.

    Returns:
        list[dict[str, int | float | bool | str | None]]:
            Machine information and the latest measurements,
            ordered alphabetically by hostname. Measurement
            fields are null when no measurements exist.
    """
    with get_database_connection() as connection:
        rows = connection.execute(
            """
            SELECT
                m.id,
                m.hostname,
                r.recorded_at,
                r.cpu_percent,
                r.ram_percent,
                r.disk_free_percent,
                r.robot_running,
                r.uptime_seconds
            FROM machines AS m
            LEFT JOIN LATERAL (
                SELECT
                    recorded_at,
                    cpu_percent,
                    ram_percent,
                    disk_free_percent,
                    robot_running,
                    uptime_seconds
                FROM measurements
                WHERE machine_id = m.id
                ORDER BY recorded_at DESC, id DESC
                LIMIT 1
            ) AS r ON TRUE
            ORDER BY m.hostname
            """
        ).fetchall()

    return [
        {
            "id": row[0],
            "hostname": row[1],
            "recorded_at": row[2].isoformat() if row[2] is not None else None,
            "cpu_percent": row[3],
            "ram_percent": row[4],
            "disk_free_percent": row[5],
            "robot_running": row[6],
            "uptime_seconds": row[7],
        }
        for row in rows
    ]
