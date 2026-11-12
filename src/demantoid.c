#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <demantoid.h>
#include <libpq-fe.h>

struct DemantoidConn {
    PGconn *pg;
    char *tablename;
};

static bool exec_command(PGconn *pg, const char *sql) {
    PGresult *res = PQexec(pg, sql);
    if (!res) {
        fprintf(stderr, "[demantoid | ERROR] %s\n", PQerrorMessage(pg));
        return false;
    }
    bool ok = PQresultStatus(res) == PGRES_COMMAND_OK;
    if (!ok) {
        fprintf(stderr, "[demantoid | ERROR] SQL command failed: %s\n",
                PQresultErrorMessage(res));
    }
    PQclear(res);
    return ok;
}

DemantoidConn *demantoid_connect(const char *conninfo) {
    if (!conninfo) {
        fprintf(stderr, "[demantoid | ERROR] Connection info is NULL\n");
        return NULL;
    }

    PGconn *pg = PQconnectdb(conninfo);
    if (!pg) return NULL;
    if (PQstatus(pg) != CONNECTION_OK) {
        fprintf(stderr, "[demantoid | ERROR] Connection failed: %s", PQerrorMessage(pg));
        PQfinish(pg);
        return NULL;
    }

    DemantoidConn *conn = calloc(1, sizeof *conn);
    if (!conn) {
        PQfinish(pg);
        return NULL;
    }
    conn->pg = pg;
    return conn;
}

void demantoid_close(DemantoidConn *conn) {
    if (!conn) return;
    PQfinish(conn->pg);
    free(conn->tablename);
    free(conn);
}

bool create_experiment(DemantoidConn *conn, const char *experimentinfo) {
    if (!conn || !conn->pg || !experimentinfo) {
        fprintf(stderr, "[demantoid | ERROR] Invalid experiment arguments\n");
        return false;
    }

    bool success = false;
    bool in_transaction = false;
    char *table_name = NULL;
    char *identifier = NULL;
    char *regclass_literal = NULL;
    PGresult *res = NULL;

    const char *key = "tablename=";
    const char *start = strstr(experimentinfo, key);
    if (!start) {
        fprintf(stderr, "[demantoid | ERROR] tablename not found in: %s\n", experimentinfo);
        return false;
    }
    start += strlen(key);
    const char *end = start;
    while (*end && *end != ' ' && *end != '\t' && *end != '\n') ++end;
    size_t len = (size_t)(end - start);
    if (len == 0) {
        fprintf(stderr, "[demantoid | ERROR] Empty table name\n");
        return false;
    }

    table_name = malloc(len + 1);
    if (!table_name) goto cleanup;
    memcpy(table_name, start, len);
    table_name[len] = '\0';

    identifier = PQescapeIdentifier(conn->pg, table_name, len);
    if (!identifier) {
        fprintf(stderr, "[demantoid | ERROR] Invalid table identifier: %s\n",
                PQerrorMessage(conn->pg));
        goto cleanup;
    }

    regclass_literal = PQescapeLiteral(conn->pg, identifier, strlen(identifier));
    if (!regclass_literal) {
        fprintf(stderr, "[demantoid | ERROR] Cannot escape table reference: %s\n",
                PQerrorMessage(conn->pg));
        goto cleanup;
    }

    res = PQexec(conn->pg,
        "SELECT EXISTS (SELECT 1 FROM pg_extension WHERE extname = 'timescaledb')");
    if (!res) {
        fprintf(
            stderr,
            "[demantoid | ERROR] Extension check failed: %s\n",
            PQerrorMessage(conn->pg)
        );
        goto cleanup;
    }
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) != 1) {
        fprintf(stderr, "[demantoid | ERROR] Extension check failed: %s\n",
                PQresultErrorMessage(res));
        goto cleanup;
    }
    bool installed = strcmp(PQgetvalue(res, 0, 0), "t") == 0;
    PQclear(res);
    res = NULL;

    if (!installed) {
        if (!exec_command(conn->pg, "CREATE EXTENSION IF NOT EXISTS timescaledb"))
            goto cleanup;
    }

    if (!exec_command(conn->pg, "BEGIN")) goto cleanup;
    in_transaction = true;

    const char *create_fmt =
        "CREATE TABLE IF NOT EXISTS %s ("
        "time TIMESTAMPTZ NOT NULL, "
        "run_id INT NOT NULL, "
        "pop_id INT NOT NULL, "
        "metric_name TEXT NOT NULL, "
        "metric_value DOUBLE PRECISION NOT NULL)";

    int needed = snprintf(NULL, 0, create_fmt, identifier);
    if (needed < 0) goto cleanup;
    char *sql = malloc((size_t)needed + 1);
    if (!sql) goto cleanup;
    snprintf(sql, (size_t)needed + 1, create_fmt, identifier);
    bool ok = exec_command(conn->pg, sql);
    free(sql);
    if (!ok) goto cleanup;

    const char *hyper_fmt =
        "SELECT create_hypertable(%s::regclass, 'time', if_not_exists => TRUE)";

    needed = snprintf(NULL, 0, hyper_fmt, regclass_literal);
    if (needed < 0) goto cleanup;
    sql = malloc((size_t)needed + 1);
    if (!sql) goto cleanup;
    snprintf(sql, (size_t)needed + 1, hyper_fmt, regclass_literal);
    res = PQexec(conn->pg, sql);
    free(sql);

    if (!res) {
        fprintf(
            stderr,
            "[demantoid | ERROR] Hypertable creation failed: %s\n",
            PQerrorMessage(conn->pg)
        );
        goto cleanup;
    }

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(
            stderr,
            "[demantoid | ERROR] Hypertable creation failed: %s\n",
            PQresultErrorMessage(res)
        );
        goto cleanup;
    }
    PQclear(res);
    res = NULL;

    if (!exec_command(conn->pg, "COMMIT")) goto cleanup;
    in_transaction = false;

    free(conn->tablename);
    conn->tablename = table_name;
    table_name = NULL;
    success = true;
    printf("[demantoid | INFO] Table %s is ready\n", conn->tablename);

cleanup:
    if (in_transaction) {
        PGresult *rollback = PQexec(conn->pg, "ROLLBACK");
        if (rollback) PQclear(rollback);
    }
    if (res) PQclear(res);
    if (regclass_literal) PQfreemem(regclass_literal);
    if (identifier) PQfreemem(identifier);
    free(table_name);
    return success;
}

bool add_metric(
    DemantoidConn *conn,
    int run_number,
    int pop_number,
    const char *metric_name,
    float metric_value
) {
    if (!conn || !conn->pg || !conn->tablename || !metric_name) {
        fprintf(stderr, "[demantoid | ERROR] Invalid arguments for adding metric\n");
        return false;
    }

    char *identifier = PQescapeIdentifier(
        conn->pg, conn->tablename, strlen(conn->tablename));
    if (!identifier) {
        fprintf(
            stderr,
            "[demantoid | ERROR] Cannot escape table name: %s\n",
            PQerrorMessage(conn->pg)
        );
        return false;
    }

    const char *fmt =
        "INSERT INTO %s (time, run_id, pop_id, metric_name, metric_value) "
        "VALUES (NOW(), $1, $2, $3, $4)";
    int needed = snprintf(NULL, 0, fmt, identifier);
    if (needed < 0) {
        PQfreemem(identifier);
        return false;
    }
    char *sql = malloc((size_t)needed + 1);
    if (!sql) {
        PQfreemem(identifier);
        return false;
    }
    snprintf(sql, (size_t)needed + 1, fmt, identifier);
    PQfreemem(identifier);

    char run_str[32], pop_str[32], value_str[64];
    snprintf(run_str, sizeof run_str, "%d", run_number);
    snprintf(pop_str, sizeof pop_str, "%d", pop_number);
    snprintf(value_str, sizeof value_str, "%.9g", metric_value);
    const char *params[] = {run_str, pop_str, metric_name, value_str};

    PGresult *res = PQexecParams(conn->pg, sql, 4, NULL, params, NULL, NULL, 0);
    free(sql);
    if (!res) {
        fprintf(
            stderr,
            "[demantoid | ERROR] Metric insertion failed: %s\n",
            PQerrorMessage(conn->pg)
        );
        return false;
    }
    bool ok = PQresultStatus(res) == PGRES_COMMAND_OK;
    if (!ok) {
        fprintf(
            stderr,
            "[demantoid | ERROR] Metric insertion failed: %s\n",
            PQresultErrorMessage(res)
        );
    }
    PQclear(res);
    if (ok) {
        fprintf(
            stderr,
            "[demantoid | INFO] Metric %s added to %s\n",
            metric_name, conn->tablename
        );
    }
    return ok;
}
