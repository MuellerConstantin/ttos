#include <util/shortid.h>
#include <util/random.h>

void generate_short_id(char* buffer, bool (*exists)(const char* id)) {
    const char* hex_digits = "0123456789abcdef";

    // Generate a random hex id, retrying until the caller's predicate reports
    // it as free. This guarantees uniqueness by construction instead of relying
    // on the size of the id space.
    do {
        for(size_t i = 0; i < SHORT_ID_LENGTH; i++) {
            buffer[i] = hex_digits[random_next() & 0x0F];
        }

        buffer[SHORT_ID_LENGTH] = '\0';
    } while(exists != NULL && exists(buffer));
}
