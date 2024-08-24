#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

extern "C" {

int16_t index_from_number_key(uint16_t vkey);

uint32_t number_key_label_at(uint32_t index);

} // extern "C"
