#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
        printf("Connection failed: %s", PQerrorMessage(conn));
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
    return true;
}

bool add_metric(DemantoidConn *conn, int run_number, char *metric_name, float metric_value) {
    return true;
}
