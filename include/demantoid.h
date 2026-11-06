#ifndef DEMANTOID_H
#define DEMANTOID_H

#ifdef __cplusplus
extern "C" {
#endif

void test_print(void);

typedef struct DemantoidConn DemantoidConn;

DemantoidConn *demantoid_connect(const char *conninfo);
void demantoid_close(DemantoidConn *conn);

#ifdef __cplusplus
}
#endif

#endif /* DEMANTOID_H */
