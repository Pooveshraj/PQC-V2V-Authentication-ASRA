#include "vehicle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initialize_vehicle(Vehicle *vehicle, const char *id)
{
    memset(vehicle, 0, sizeof(Vehicle));

    strncpy(vehicle->vehicle_id,
            id,
            VEHICLE_ID_LENGTH - 1);

    vehicle->trust_score = 1.0f;
    vehicle->risk_score = 0.0f;
    vehicle->authenticated = false;

    printf("Vehicle [%s] initialized.\n",
           vehicle->vehicle_id);
}

void free_vehicle(Vehicle *vehicle)
{
    free(vehicle->kem_public_key); vehicle->kem_public_key = NULL;
    free(vehicle->kem_secret_key); vehicle->kem_secret_key = NULL;
    free(vehicle->dsa_public_key); vehicle->dsa_public_key = NULL;
    free(vehicle->dsa_secret_key); vehicle->dsa_secret_key = NULL;
    free(vehicle->session_key);     vehicle->session_key = NULL;

}
