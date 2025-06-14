#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include "gmpecc.h"
#include "util.h"

int main(int argc, char **argv) {
    char *pubkey_hex = NULL;
    char *filename = NULL;
    unsigned long max_offset = 0;
    unsigned long step = 1024;

    // Argument parsing
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-p") && i + 1 < argc) {
            pubkey_hex = argv[++i];
        } else if (!strcmp(argv[i], "-file") && i + 1 < argc) {
            filename = argv[++i];
        } else if (!strcmp(argv[i], "-r") && i + 1 < argc) {
            max_offset = strtoul(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "-s") && i + 1 < argc) {
            step = strtoul(argv[++i], NULL, 10);
        }
    }

    if (!pubkey_hex || !filename || max_offset == 0) {
        fprintf(stderr, "Usage: %s -p <pubkey> -file <target_file> -r <range> [-s <step>]\n", argv[0]);
        return 1;
    }

    // Load target pubkeys from file
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open target file");
        return 1;
    }

    char line[256];
    size_t target_count = 0;
    size_t max_targets = 1000000;
    gmpecc_point_t *targets = malloc(sizeof(gmpecc_point_t) * max_targets);

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;
        if (target_count >= max_targets) {
            fprintf(stderr, "Too many targets (max %zu)\n", max_targets);
            break;
        }
        gmpecc_point_init(&targets[target_count]);
        if (!parse_public_key_hex(&targets[target_count], line)) {
            fprintf(stderr, "Failed to parse target pubkey: %s\n", line);
            gmpecc_point_clear(&targets[target_count]);
            continue;
        }
        target_count++;
    }
    fclose(fp);

    // Parse initial pubkey
    gmpecc_point_t base;
    gmpecc_point_init(&base);
    if (!parse_public_key_hex(&base, pubkey_hex)) {
        fprintf(stderr, "Failed to parse input pubkey\n");
        return 1;
    }

    // Prepare variables
    gmpecc_point_t result;
    gmpecc_point_init(&result);
    mpz_t offset_scalar, total_scalar;
    mpz_init(offset_scalar);
    mpz_init(total_scalar);

    for (unsigned long i = 1; i <= max_offset; i++) {
        mpz_set_ui(offset_scalar, i);
        mpz_mul_ui(total_scalar, offset_scalar, step); // total_scalar = i * step

        // result = base - (total_scalar * G)
        gmpecc_point_mulsub_g(&result, &base, total_scalar);

        // Check against all target pubkeys
        for (size_t j = 0; j < target_count; j++) {
            if (gmpecc_point_equal(&result, &targets[j])) {
                char *hex = format_public_key_hex(&result);
                printf("Match found:\n");
                printf("Public Key: %s\n", hex);
                gmp_printf("Offset: %Zd\n", offset_scalar);
                gmp_printf("Total Scalar: %Zd\n", total_scalar);
                free(hex);
                goto cleanup;
            }
        }
    }

    printf("No match found in %lu steps.\n", max_offset);

cleanup:
    for (size_t i = 0; i < target_count; i++) {
        gmpecc_point_clear(&targets[i]);
    }
    free(targets);
    gmpecc_point_clear(&base);
    gmpecc_point_clear(&result);
    mpz_clear(offset_scalar);
    mpz_clear(total_scalar);
    return 0;
}
