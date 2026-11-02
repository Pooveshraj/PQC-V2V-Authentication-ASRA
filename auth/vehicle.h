#ifndef VEHICLE_H
#define VEHICLE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <oqs/oqs.h>

#ifdef __cplusplus
extern "C" {
#endif
#define VEHICLE_ID_LENGTH 32

typedef struct {

    /* Vehicle Identity */
    char vehicle_id[VEHICLE_ID_LENGTH];

    /* ML-KEM Keys */
    uint8_t *kem_public_key;
    uint8_t *kem_secret_key;

    /* ML-DSA Keys */
    uint8_t *dsa_public_key;
    uint8_t *dsa_secret_key;

    /* Session Key */
    uint8_t *session_key;

    size_t session_key_length;

    /* Trust Parameters */
    float trust_score;
    float risk_score;

    /* Authentication Status */
    bool authenticated;

} Vehicle;

/* Vehicle Management */
void initialize_vehicle(Vehicle *vehicle, const char *id);
void free_vehicle(Vehicle *vehicle);

#ifdef __cplusplus
}
#endif

#endif


