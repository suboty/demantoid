import de
import demantoid

if __name__ == '__main__':
    print('demantoid tests in Python have been launched...')

    wrong_connection_str = "123321"
    wrong_conn = demantoid.demantoid_connect(wrong_connection_str)
    if not wrong_conn:
        print("[OK] - wrong test connection")
    else:
        print("[ERROR] - wrong test connection")

    correct_connection_str = "host=localhost \
    port=16543 \
    dbname=demantoid \
    connect_timeout=10 \
    user=demo_user \
    password=demo_password"
    correct_conn = demantoid.demantoid_connect(correct_connection_str)
    if correct_conn:
        print("[OK] - corrected test connection")
        demantoid.demantoid_close(correct_conn)
    else:
        print("[ERROR] - corrected test connection")

    de_test_result = de.run_de()
    if de_test_result:
        print("[OK] - de 1/rand/bin example")
    else:
        print("[ERROR] - de 1/rand/bin example\n")
