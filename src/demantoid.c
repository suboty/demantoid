#include <stdio.h>
#include <stdlib.h>
#include <demantoid.h>
#include <libpq-fe.h>

void test_print() {
    printf("Demantoid test\n");
}

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
