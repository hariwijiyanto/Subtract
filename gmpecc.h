#ifndef GMPECC_H
#define GMPECC_H

#include <gmp.h>

typedef struct {
    mpz_t x, y;
    int infinity;
} gmpecc_point_t;

void gmpecc_point_init(gmpecc_point_t *P);
void gmpecc_point_clear(gmpecc_point_t *P);
void gmpecc_point_copy(gmpecc_point_t *R, const gmpecc_point_t *P);
int gmpecc_point_equal(const gmpecc_point_t *P, const gmpecc_point_t *Q);
void gmpecc_point_mulsub_g(gmpecc_point_t *R, const gmpecc_point_t *P, const mpz_t k);

#endif
