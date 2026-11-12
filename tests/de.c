#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <demantoid.h>

const float MAX_BEST_VALUE = 1000000.0;
const int POP_SIZE = 10;
const int DIM = 3;
const int RUNS = 20;

float fitness_function(int x1, int x2, int x3) {
    return sinf((float)(x1 * x2) - (float)x3);
}

void calculate_fitness(float population[DIM][POP_SIZE], float pop_values[POP_SIZE]) {
    for (int i = 0; i < POP_SIZE; i++) {
        pop_values[i] = fitness_function(
            (int)population[0][i],
            (int)population[1][i],
            (int)population[2][i]
        );
    }
}

bool run_de(const char *experiment_info_str) {
    srand((unsigned)time(NULL));
    float best_values[RUNS];

    bool tests_ok = true;

    const char *connection_str = "host=localhost \
        port=16543 \
        dbname=demantoid \
        connect_timeout=10 \
        user=demo_user \
        password=demo_password";

    DemantoidConn *demantoid_conn = demantoid_connect(connection_str);

    tests_ok *= create_experiment(demantoid_conn, experiment_info_str);

    for (int i_run = 0; i_run < RUNS; i_run++) {
        float best_value = MAX_BEST_VALUE;
        float population[DIM][POP_SIZE];
        float pop_values[POP_SIZE];
        const float CR = 0.25f;
        const float F = 0.25f;

        for (int i_dim = 0; i_dim < DIM; i_dim++)
            for (int i_el = 0; i_el < POP_SIZE; i_el++)
                population[i_dim][i_el] = (float)(rand() % 100);

        calculate_fitness(population, pop_values);

        for (int i_pop = 0; i_pop < POP_SIZE; i_pop++) {
            int pool[POP_SIZE];
            int n = 0;
            for (int i = 0; i < POP_SIZE; i++)
                if (i != i_pop) pool[n++] = i;

            for (int i = n - 1; i > 0; i--) {
                int j = rand() % (i + 1);
                int tmp = pool[i]; pool[i] = pool[j]; pool[j] = tmp;
            }
            int r = pool[0], s = pool[1], t = pool[2];

            float mutation[DIM];
            for (int d = 0; d < DIM; d++)
                mutation[d] = population[d][t] + F * (population[d][r] - population[d][s]);

            float trial[DIM];
            for (int d = 0; d < DIM; d++) {
                float rand_pos = (float)rand() / (float)RAND_MAX;
                trial[d] = (rand_pos < CR) ? population[d][i_pop] : mutation[d];
            }

            float trial_value = fitness_function((int)trial[0], (int)trial[1], (int)trial[2]);
            if (trial_value <= pop_values[i_pop]) {
                for (int d = 0; d < DIM; d++)
                    population[d][i_pop] = trial[d];
                pop_values[i_pop] = trial_value;
            }
        }

        for (int i = 0; i < POP_SIZE; i++)
            if (pop_values[i] < best_value)
                best_value = pop_values[i];

        best_values[i_run] = best_value;

        tests_ok *= add_metric(
            demantoid_conn,
            i_run,
            -1,
            "test fitness best in population",
            best_values[i_run]
        );
    }

    return tests_ok;
}
