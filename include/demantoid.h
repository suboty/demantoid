#ifndef DEMANTOID_H
#define DEMANTOID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct DemantoidConn DemantoidConn;

DemantoidConn *demantoid_connect(const char *conninfo);
bool create_experiment(DemantoidConn *conn, const char *experimentinfo);
bool add_metric(DemantoidConn *conn, int run_number, char *metric_name, float metric_value);
void demantoid_close(DemantoidConn *conn);

#ifdef __cplusplus
}
#endif

#endif /* DEMANTOID_H */
