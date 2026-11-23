
import psycopg


def get_database_connection() -> psycopg.Connection:
    """
    Establish a connection to the PostgreSQL database.

    Returns:
        psycopg.Connection: An open database connection.
    """
    return psycopg.connect()
