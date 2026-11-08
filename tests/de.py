import math
import random

import demantoid

MAX_BEST_VALUE = 1000000.0
POP_SIZE = 10
DIM = 3
RUNS = 20


def fitness_function(x1: int, x2: int, x3: int) -> float:
    return math.sin((x1 * x2) - x3)


def calculate_fitness(population: list[list], pop_values: list[float]) -> None:
    for i, individual in enumerate(population):
        pop_values[i] = fitness_function(*individual)


def run_de(experiment_info_str: str) -> bool:
    tests_ok = True

    correct_connection_str = (
        "host=localhost "
        "port=16543 "
        "dbname=demantoid "
        "connect_timeout=10 "
        "user=demo_user "
        "password=demo_password"
    )
    demantoid_conn = demantoid.demantoid_connect(correct_connection_str)

    tests_ok *= demantoid.create_experiment(demantoid_conn, experiment_info_str)

    best_values = [MAX_BEST_VALUE] * RUNS

    for i_run in range(RUNS):
        best_value = MAX_BEST_VALUE
        CR = 0.25
        F = 0.25

        population = [[random.randint(0, 100) for _ in range(DIM)] for _ in range(POP_SIZE)]
        pop_values = [0.0] * POP_SIZE

        calculate_fitness(population, pop_values)

        for i_pop in range(POP_SIZE):
            pool = [i for i in range(POP_SIZE) if i != i_pop]
            random.shuffle(pool)
            r, s, t = pool[0], pool[1], pool[2]

            mutation = [0.0] * DIM
            for d in range(DIM):
                mutation[d] = population[t][d] + F * (population[r][d] - population[s][d])

            trial = [0.0] * DIM
            for d in range(DIM):
                rand_pos = random.random()
                trial[d] = population[i_pop][d] if rand_pos < CR else mutation[d]

            trial_value = fitness_function(int(trial[0]), int(trial[1]), int(trial[2]))
            if trial_value <= pop_values[i_pop]:
                population[i_pop] = trial.copy()
                pop_values[i_pop] = trial_value

        for i in range(POP_SIZE):
            best_value = min(best_value, pop_values[i])

        best_values[i_run] = best_value

        tests_ok *= demantoid.add_metric(
            demantoid_conn,
            i_run,
            "test fitness best in population",
            best_values[i_run]
        )

    return tests_ok
