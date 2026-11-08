#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <demantoid.h>
#include <libpq-fe.h>

struct DemantoidConn {
    PGconn *pg;
};

DemantoidConn *demantoid_connect(const char *conninfo) {
    PGconn *conn = PQconnectdb(conninfo);
    if (!conn) {
        return NULL;
    }

    if (PQstatus(conn) != CONNECTION_OK) {
        printf(
            "[demantoid | ERROR] Connection failed: %s",
            PQerrorMessage(conn)
        );
        PQfinish(conn);
        return NULL;
    }

    DemantoidConn *out = malloc(sizeof *out);
    if (!out) {
        PQfinish(conn);
        return NULL;
    }
    out -> pg = conn;
    return out;
}

void demantoid_close(DemantoidConn *conn) {
    if (!conn) {
        return;
    }
    PQfinish(conn -> pg);
    free(conn);
    return;
}

bool create_experiment(DemantoidConn *conn, const char *experimentinfo) {
    const char *key = "tablename=";
    const char *start = strstr(experimentinfo, key);
    if (!start) {
        printf(
            "[demantoid | ERROR] tablename not found in: %s\n",
            experimentinfo
        );
        return false;
    }
    start += strlen(key);

    const char *end = start;
    while (*end && *end != ' ' && *end != '\0') end++;

    size_t len = (size_t)(end - start);
    char *table_name = malloc(len + 1);
    if (!table_name) return false;

    memcpy(table_name, start, len);
    table_name[len] = '\0';

    printf("[demantoid | INFO] Detected table name: %s\n", table_name);

    char *escaped_table_name = PQescapeIdentifier(conn->pg, table_name, strlen(table_name));
    if (!escaped_table_name) {
        printf("[demantoid | ERROR] Failed to escape table name: %s\n", PQerrorMessage(conn->pg));
        return false;
    }

    char *escaped_table_name_as_literal = PQescapeLiteral(conn->pg, table_name, strlen(table_name));
    if (!escaped_table_name_as_literal) {
        printf("[demantoid | ERROR] Failed to escape table name: %s\n", PQerrorMessage(conn->pg));
        return false;
    }

    char create_table_query[1024];
    int n = snprintf(create_table_query, sizeof(create_table_query),
        "CREATE TABLE %s ("
        "time TIMESTAMPTZ NOT NULL, "
        "run_id INT NOT NULL, "
        "metric_name FLOAT NOT NULL, "
        "metric_value FLOAT NOT NULL);",
        escaped_table_name);

    if (n < 0 || (size_t)n >= sizeof(create_table_query)) {
        printf("[demantoid | ERROR] Query for table creating string is too long (truncated)\n");
        return false;
    }

    PGresult *create_table_res = PQexec(conn->pg, create_table_query);

    if (!create_table_res) {
        printf(
            "[demantoid | ERROR] Query for table creating failed: %s\n",
            PQerrorMessage(conn->pg)
        );
        return false;
    }

    if (PQresultStatus(create_table_res) != PGRES_COMMAND_OK) {
        printf(
            "[demantoid | ERROR] Creating table for metrics process failed: %s\n",
            PQerrorMessage(conn->pg)
        );
        PQclear(create_table_res);
        return false;
    }

    PQclear(create_table_res);
    PQfreemem(escaped_table_name);

    char create_hypertable_query[1024];
    int f = snprintf(create_hypertable_query, sizeof(create_hypertable_query),
        "SELECT create_hypertable(%s, 'time');",
        escaped_table_name_as_literal
    );

    if (f < 0 || (size_t)f >= sizeof(create_hypertable_query)) {
        printf("[demantoid | ERROR] Query for hypertable creating string is too long (truncated)\n");
        return false;
    }

    PGresult *create_hypertable_res = PQexec(conn->pg, create_hypertable_query);

    if (!create_hypertable_res) {
        printf(
            "[demantoid | ERROR] Query for hypertable creating failed: %s\n",
            PQerrorMessage(conn->pg)
        );
        return false;
    }

    if (PQresultStatus(create_hypertable_res) != PGRES_TUPLES_OK) {
        char *error = PQerrorMessage(conn->pg);
        char *result = strstr(error, "No function matches");
        if (result) {
            PGresult *extension_res = PQexec(
                conn->pg,
                "CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;"
            );

            if (!extension_res) {
                printf(
                    "[demantoid | ERROR] Query for extension activating failed: %s\n",
                    PQerrorMessage(conn->pg)
                );
                return false;
            }

            if (PQresultStatus(extension_res) != PGRES_COMMAND_OK) {
                printf(
                    "[demantoid | ERROR] Activating extension for timescaleDB process failed: %s\n",
                    PQerrorMessage(conn->pg)
                );
                PQclear(extension_res);
                return false;
            }
        }
        else {
            printf(
                "[demantoid | ERROR] Creating hypertable for metrics process failed: %s\n",
                PQerrorMessage(conn->pg)
            );
            PQclear(create_hypertable_res);
            return false;
        }
    }

    PQclear(create_hypertable_res);

    printf("[demantoid | INFO] Table %s is successfully created\n", table_name);
    free(table_name);
    return true;
}

bool add_metric(DemantoidConn *conn, int run_number, char *metric_name, float metric_value) {
    return true;
}
