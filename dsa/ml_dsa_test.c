#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oqs/oqs.h>

int main(void) {

    OQS_SIG *sig = OQS_SIG_new(OQS_SIG_alg_ml_dsa_65);

    if (sig == NULL) {
        printf("Failed to initialize ML-DSA-65\n");
        return 1;
    }

    uint8_t *public_key = malloc(sig->length_public_key);
    uint8_t *secret_key = malloc(sig->length_secret_key);

    if (OQS_SIG_keypair(sig, public_key, secret_key) != OQS_SUCCESS) {
        printf("Key generation failed\n");
        return 1;
    }

    const char *message = "Vehicle Authentication";
    size_t message_len = strlen(message);

    uint8_t *signature = malloc(sig->length_signature);

    size_t signature_len;

    if (OQS_SIG_sign(sig,
                     signature,
                     &signature_len,
                     (const uint8_t *)message,
                     message_len,
                     secret_key) != OQS_SUCCESS) {

        printf("Signing failed\n");
        return 1;
    }

    if (OQS_SIG_verify(sig,
                       (const uint8_t *)message,
                       message_len,
                       signature,
                       signature_len,
                       public_key) == OQS_SUCCESS) {

        printf("\n=====================================\n");
        printf("ML-DSA-65 TEST PASSED\n");
        printf("Signature Verified Successfully\n");
        printf("=====================================\n");

    } else {

        printf("Signature Verification FAILED\n");
    }

    free(public_key);
    free(secret_key);
    free(signature);

    OQS_SIG_free(sig);

    return 0;
}
