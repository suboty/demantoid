#ifndef DE_H
#define DE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

bool run_de();
float fitness_function(int x1, int x2, int x3);
float calculate_fitness(int *population, float *pop_values);

#ifdef __cplusplus
}
#endif

#endif /* DE_H */
