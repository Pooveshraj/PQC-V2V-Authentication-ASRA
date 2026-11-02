#include "authentication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool generate_dsa_keys(Vehicle *vehicle)
{
    OQS_SIG *sig = OQS_SIG_new(DSA_ALGORITHM);
    if (sig == NULL) {
        printf("Failed to initialize %s\n", DSA_ALGORITHM);
        return false;
    }

    vehicle->dsa_public_key = malloc(sig->length_public_key);
    vehicle->dsa_secret_key = malloc(sig->length_secret_key);

    if (!vehicle->dsa_public_key || !vehicle->dsa_secret_key) {
        printf("Memory allocation failed for DSA keys\n");
        OQS_SIG_free(sig);
        return false;
    }

    OQS_STATUS rc = OQS_SIG_keypair(sig,
                                     vehicle->dsa_public_key,
                                     vehicle->dsa_secret_key);

    OQS_SIG_free(sig);

    if (rc != OQS_SUCCESS) {
        printf("DSA keypair generation failed for [%s]\n",
               vehicle->vehicle_id);
        return false;
    }

    printf("DSA keys generated for [%s]\n", vehicle->vehicle_id);
    return true;
}

bool sign_message(Vehicle *vehicle,
                   const uint8_t *message,
                   size_t message_len,
                   uint8_t **signature,
                   size_t *signature_len)
{
    OQS_SIG *sig = OQS_SIG_new(DSA_ALGORITHM);
    if (sig == NULL) return false;

    *signature = malloc(sig->length_signature);
    if (*signature == NULL) {
        OQS_SIG_free(sig);
        return false;
    }

    OQS_STATUS rc = OQS_SIG_sign(sig,
                                  *signature,
                                  signature_len,
                                  message,
                                  message_len,
                                  vehicle->dsa_secret_key);

    OQS_SIG_free(sig);

    if (rc != OQS_SUCCESS) {
        printf("Signing failed for [%s]\n", vehicle->vehicle_id);
        free(*signature);
        *signature = NULL;
        return false;
    }

    return true;
}

bool verify_signature(const uint8_t *public_key,
                       const uint8_t *message,
                       size_t message_len,
                       const uint8_t *signature,
                       size_t signature_len)
{
    OQS_SIG *sig = OQS_SIG_new(DSA_ALGORITHM);
    if (sig == NULL) return false;

    OQS_STATUS rc = OQS_SIG_verify(sig,
                                    message,
                                    message_len,
                                    signature,
                                    signature_len,
                                    public_key);

    OQS_SIG_free(sig);

    return (rc == OQS_SUCCESS);
}

bool authenticate_vehicle(Vehicle *sender, Vehicle *receiver)
{
    const char *challenge = "V2V_AUTH_CHALLENGE";
    uint8_t *signature = NULL;
    size_t signature_len = 0;

    if (!sign_message(sender,
                       (const uint8_t *)challenge,
                       strlen(challenge),
                       &signature,
                       &signature_len)) {
        return false;
    }

    bool valid = verify_signature(sender->dsa_public_key,
                                   (const uint8_t *)challenge,
                                   strlen(challenge),
                                   signature,
                                   signature_len);

    free(signature);

    if (valid) {
        sender->authenticated = true;
        printf("[%s] authenticated successfully by [%s]\n",
               sender->vehicle_id, receiver->vehicle_id);
    } else {
        printf("[%s] authentication FAILED\n", sender->vehicle_id);
    }

    return valid;
}
