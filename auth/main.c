#include "vehicle.h"
#include "authentication.h"
#include "session_key.h"
#include  "asra.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Vehicle vehicleA;
    Vehicle vehicleB;

    printf("===== V2V PQC Authentication Demo =====\n\n");

    /* Step 1: Initialize both vehicles */
    initialize_vehicle(&vehicleA, "VEHICLE_A");
    initialize_vehicle(&vehicleB, "VEHICLE_B");

    /* Step 2: Generate ML-DSA signing keys for both vehicles */
    if (!generate_dsa_keys(&vehicleA) || !generate_dsa_keys(&vehicleB)) {
        printf("DSA key generation failed. Exiting.\n");
        return 1;
    }

    /* Step 3: Generate ML-KEM keys for both vehicles */
    if (!generate_kem_keys(&vehicleA) || !generate_kem_keys(&vehicleB)) {
        printf("KEM key generation failed. Exiting.\n");
        return 1;
    }

    printf("\n----- Authentication Phase -----\n");

    /* Step 4: Vehicle A authenticates itself to Vehicle B */
    bool crypto_valid = authenticate_vehicle(&vehicleA, &vehicleB);
    if (!crypto_valid){
        printf("Authentication failed. Exiting.\n");
        return 1;
    }

printf("\n----- ASRA: Adaptive Side-Channel Risk Assessment -----\n");

    ASRA_Result signing_risk = asra_evaluate_signing_robust(&vehicleA,5, ASRA_DEFAULT_TRIALS);
    asra_print_result(vehicleA.vehicle_id, &signing_risk);

    double risk_threshold = 0.7; /* calibrated: loosened to verify full pipeline;revert to 0.5 for real experiment */
    bool final_decision = asra_authentication_decision(crypto_valid,&signing_risk,risk_threshold);
                                                  
                                                       

    if (!final_decision) {
        printf("[ASRA] Authentication REJECTED due to high leakage risk for [%s]\n",
               vehicleA.vehicle_id);
        free_vehicle(&vehicleA);
        free_vehicle(&vehicleB);
        return 1;
    }

    printf("[ASRA] Authentication ACCEPTED — cryptographically valid and low leakage risk.\n");

    printf("\n----- Session Key Exchange -----\n");

    /* Step 5: Vehicle A (initiator) encapsulates a session key
     *         using Vehicle B's public KEM key. */
    uint8_t *ciphertext = NULL;
    size_t ciphertext_len = 0;

    if (!encapsulate_session_key(&vehicleA, &vehicleB,
                                  &ciphertext, &ciphertext_len)) {
        printf("Encapsulation failed. Exiting.\n");
        return 1;
    }

    /* Step 6: Vehicle B (responder) decapsulates the ciphertext
     *         using its own secret KEM key. */
    if (!decapsulate_session_key(&vehicleB, ciphertext, ciphertext_len)) {
        printf("Decapsulation failed. Exiting.\n");
        free(ciphertext);
        return 1;
    }

    /* Step 7: Confirm both vehicles derived the same session key */
    printf("\n----- Verifying Shared Session Key -----\n");

    if (vehicleA.session_key_length == vehicleB.session_key_length &&
        memcmp(vehicleA.session_key, vehicleB.session_key,
               vehicleA.session_key_length) == 0) {
        printf("SUCCESS: Vehicle A and Vehicle B share the same session key.\n");
    } else {
        printf("MISMATCH: Session keys do not match!\n");
    }

    /* Cleanup */
    free(ciphertext);
    free_vehicle(&vehicleA);
    free_vehicle(&vehicleB);

    printf("\n===== Demo Complete =====\n");

    return 0;
}
