#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <minwindef.h>

enum class CandidateMode {
  None,
  Original,
  Incremental,
  WithNextComposition,
};

enum class KeystrokeCategory {
  None,
  Composing,
  Candidate,
  InvokeCompositionEditSession,
};

enum class KeystrokeFunction {
  None,
  Input,
  Cancel,
  FinalizeTextstore,
  FinalizeTextstoreAndInput,
  FinalizeCandidatelist,
  FinalizeCandidatelistAndInput,
  Convert,
  ConvertWildcard,
  SelectByNumber,
  Backspace,
  MoveLeft,
  MoveRight,
  MoveUp,
  MoveDown,
  MovePageUp,
  MovePageDown,
  MovePageTop,
  MovePageBottom,
  DoubleSingleByte,
  Punctuation,
};

extern "C" {

bool compositionprocessorengine_add_virtual_key(void *engine, uint16_t wch);

void compositionprocessorengine_free(void *engine);

void *compositionprocessorengine_get_reading_string(void *engine);

const void *compositionprocessorengine_get_table_dictionary_engine(const void *engine);

bool compositionprocessorengine_has_virtual_key(void *engine);

bool compositionprocessorengine_keystroke_buffer_includes_wildcard(void *engine);

bool compositionprocessorengine_modifiers_is_alt_key_down_only(void *engine);

bool compositionprocessorengine_modifiers_is_control_key_down_only(void *engine);

bool compositionprocessorengine_modifiers_is_shift_key_down_only(void *engine);

void compositionprocessorengine_modifiers_update(void *engine, WPARAM w, LPARAM l);

void *compositionprocessorengine_new();

void compositionprocessorengine_pop_virtual_key(void *engine);

uint16_t compositionprocessorengine_punctuations_get_alternative_punctuation_counted(void *engine,
                                                                                     uint16_t wch);

bool compositionprocessorengine_punctuations_has_alternative_punctuation(void *engine,
                                                                         uint16_t wch);

void compositionprocessorengine_purge_virtual_key(void *engine);

void compositionprocessorengine_setup_dictionary_file(void *engine,
                                                      HINSTANCE dll_instance_handle,
                                                      void *dictionary_file_name);

void compositionprocessorengine_test_virtual_key(void *engine,
                                                 uint16_t code,
                                                 uint16_t ch,
                                                 bool composing,
                                                 CandidateMode candidate_mode,
                                                 bool *key_eaten,
                                                 KeystrokeCategory *keystroke_category,
                                                 KeystrokeFunction *keystroke_function);

} // extern "C"
