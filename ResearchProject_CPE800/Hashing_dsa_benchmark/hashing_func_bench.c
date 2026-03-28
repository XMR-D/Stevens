#include <stdint.h>
#include <stdio.h>

#define MAX_REQ_CAP 2000000


uint64_t hash_murmur3(uint64_t key) {
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdULL;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ULL;
    key ^= key >> 33;
    return key % MAX_REQ_CAP;
}

#define TEST_SIZE 1000000

int main() {
    // On utilise un tableau pour compter le nombre d'entrées par case
    static uint32_t stats[MAX_REQ_CAP] = {0};
    uint32_t collisions = 0;
    uint32_t occupied = 0;

    for (uint64_t i = 1; i <= TEST_SIZE; i++) {
        uint32_t index = hash_murmur3(i);
        if (stats[index] > 0) {
            collisions++;
        } else {
            occupied++;
        }
        stats[index]++;
    }

    printf("Resultats pour %d entrees :\n", TEST_SIZE);
    printf("- Cases occupees : %u / %d\n", occupied, MAX_REQ_CAP);
    printf("- Nombre total de collisions : %u\n", collisions);
    
    return 0;
}
