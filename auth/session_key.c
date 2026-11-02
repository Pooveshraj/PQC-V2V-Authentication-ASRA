#include "session_key.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool generate_kem_keys(Vehicle *vehicle)
{
    OQS_KEM *kem = OQS_KEM_new(KEM_ALGORITHM);
    if (kem == NULL) {
        printf("Failed to initialize %s\n", KEM_ALGORITHM);
        return false;
    }

    vehicle->kem_public_key = malloc(kem->length_public_key);
    vehicle->kem_secret_key = malloc(kem->length_secret_key);

    if (!vehicle->kem_public_key || !vehicle->kem_secret_key) {
        printf("Memory allocation failed for KEM keys\n");
        OQS_KEM_free(kem);
        return false;
    }

    OQS_STATUS rc = OQS_KEM_keypair(kem,
                                     vehicle->kem_public_key,
                                     vehicle->kem_secret_key);

    OQS_KEM_free(kem);

    if (rc != OQS_SUCCESS) {
        printf("KEM keypair generation failed for [%s]\n",
               vehicle->vehicle_id);
        return false;
    }

    printf("KEM keys generated for [%s]\n", vehicle->vehicle_id);
    return true;
}

/*
 * Initiator encapsulates against the responder's public KEM key.
 * Produces a ciphertext to send to the responder, and the
 * initiator's own copy of the shared secret (session key).
 */
bool encapsulate_session_key(Vehicle *initiator,
                              Vehicle *responder,
                              uint8_t **ciphertext,
                              size_t *ciphertext_len)
{
    OQS_KEM *kem = OQS_KEM_new(KEM_ALGORITHM);
    if (kem == NULL) return false;

    *ciphertext = malloc(kem->length_ciphertext);
    initiator->session_key = malloc(kem->length_shared_secret);

    if (!*ciphertext || !initiator->session_key) {
        printf("Memory allocation failed during encapsulation\n");
        OQS_KEM_free(kem);
        return false;
    }

    OQS_STATUS rc = OQS_KEM_encaps(kem,
                                    *ciphertext,
                                    initiator->session_key,
                                    responder->kem_public_key);

    *ciphertext_len = kem->length_ciphertext;
    initiator->session_key_length = kem->length_shared_secret;

    OQS_KEM_free(kem);

    if (rc != OQS_SUCCESS) {
        printf("Encapsulation failed for [%s]\n", initiator->vehicle_id);
        free(*ciphertext);
        free(initiator->session_key);
        initiator->session_key = NULL;
        *ciphertext = NULL;
        return false;
    }

    printf("[%s] encapsulated session key for [%s]\n",
           initiator->vehicle_id, responder->vehicle_id);

    return true;
}

/*
 * Responder decapsulates the ciphertext using its own KEM secret key
 * to recover the same shared secret the initiator derived.
 */
bool decapsulate_session_key(Vehicle *responder,
                              const uint8_t *ciphertext,
                              size_t ciphertext_len)
{
    OQS_KEM *kem = OQS_KEM_new(KEM_ALGORITHM);
    if (kem == NULL) return false;

    if (ciphertext_len != kem->length_ciphertext) {
        printf("Invalid ciphertext length for [%s]\n",
               responder->vehicle_id);
        OQS_KEM_free(kem);
        return false;
    }

    responder->session_key = malloc(kem->length_shared_secret);
    if (!responder->session_key) {
        printf("Memory allocation failed during decapsulation\n");
        OQS_KEM_free(kem);
        return false;
    }

    OQS_STATUS rc = OQS_KEM_decaps(kem,
                                    responder->session_key,
                                    ciphertext,
                                    responder->kem_secret_key);

    responder->session_key_length = kem->length_shared_secret;

    OQS_KEM_free(kem);

    if (rc != OQS_SUCCESS) {
        printf("Decapsulation failed for [%s]\n", responder->vehicle_id);
        free(responder->session_key);
        responder->session_key = NULL;
        return false;
    }

    printf("[%s] decapsulated session key successfully\n",
           responder->vehicle_id);

    return true;
}
