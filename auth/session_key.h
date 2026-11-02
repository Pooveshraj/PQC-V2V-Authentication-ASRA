#ifndef SESSION_KEY_H
#define SESSION_KEY_H

#include "vehicle.h"
#include <oqs/oqs.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
#define KEM_ALGORITHM OQS_KEM_alg_ml_kem_768

/* Key Management */
bool generate_kem_keys(Vehicle *vehicle);

/* Key Exchange */
bool encapsulate_session_key(Vehicle *initiator,
                              Vehicle *responder,
                              uint8_t **ciphertext,
                              size_t *ciphertext_len);

bool decapsulate_session_key(Vehicle *responder,
                              const uint8_t *ciphertext,
                              size_t ciphertext_len);

#ifdef __cplusplus
}
#endif

#endif
