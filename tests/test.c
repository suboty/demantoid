#include <stdio.h>
#include <stdlib.h>
#include <demantoid.h>

int main() {
    printf("demantoid tests in C have been launched...\n");
    test_print();

    const char *wrong_connection_str = "123321";
    DemantoidConn *wrong_conn = demantoid_connect(wrong_connection_str);
    if (!wrong_conn) {
        printf("[OK] - wrong test connection\n");
    } else {
        printf("[ERROR] - wrong test connection\n");
    }

    const char *correct_connection_str = "host=localhost \
        port=16543 \
        dbname=demantoid \
        connect_timeout=10 \
        user=demo_user \
        password=demo_password";
    DemantoidConn *correct_conn = demantoid_connect(correct_connection_str);
    if (correct_conn) {
        printf("[OK] - corrected test connection\n");
        demantoid_close(correct_conn);
    } else {
        printf("[ERROR] - corrected test connection\n");
    }
}
