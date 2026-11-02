#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oqs/oqs.h>

int main(void) {

    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_768);

    if (kem == NULL) {
        printf("Failed to initialize ML-KEM-768\n");
        return 1;
    }

    uint8_t *public_key = malloc(kem->length_public_key);
    uint8_t *secret_key = malloc(kem->length_secret_key);
    uint8_t *ciphertext = malloc(kem->length_ciphertext);
    uint8_t *shared_secret_enc = malloc(kem->length_shared_secret);
    uint8_t *shared_secret_dec = malloc(kem->length_shared_secret);

    if (OQS_KEM_keypair(kem, public_key, secret_key) != OQS_SUCCESS) {
        printf("Key generation failed\n");
        return 1;
    }

    if (OQS_KEM_encaps(kem, ciphertext, shared_secret_enc, public_key) != OQS_SUCCESS) {
        printf("Encapsulation failed\n");
        return 1;
    }

    if (OQS_KEM_decaps(kem, shared_secret_dec, ciphertext, secret_key) != OQS_SUCCESS) {
        printf("Decapsulation failed\n");
        return 1;
    }

    if (memcmp(shared_secret_enc,
               shared_secret_dec,
               kem->length_shared_secret) == 0) {

        printf("\n=====================================\n");
        printf("ML-KEM-768 TEST PASSED\n");
        printf("Shared secrets match.\n");
        printf("=====================================\n");
    } else {
        printf("Shared secrets do NOT match.\n");
    }

    free(public_key);
    free(secret_key);
    free(ciphertext);
    free(shared_secret_enc);
    free(shared_secret_dec);

    OQS_KEM_free(kem);

    return 0;
}

