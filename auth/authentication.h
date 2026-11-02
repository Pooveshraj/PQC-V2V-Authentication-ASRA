#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "vehicle.h"
#include <oqs/oqs.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
#define DSA_ALGORITHM OQS_SIG_alg_ml_dsa_65

/* Key Management */
bool generate_dsa_keys(Vehicle *vehicle);

/* Signing */
bool sign_message(Vehicle *vehicle,
                   const uint8_t *message,
                   size_t message_len,
                   uint8_t **signature,
                   size_t *signature_len);

/* Verification */
bool verify_signature(const uint8_t *public_key,
                       const uint8_t *message,
                       size_t message_len,
                       const uint8_t *signature,
                       size_t signature_len);

/* High-level Authentication */
bool authenticate_vehicle(Vehicle *sender,
                           Vehicle *receiver);

#ifdef __cplusplus
}
#endif

#endif
