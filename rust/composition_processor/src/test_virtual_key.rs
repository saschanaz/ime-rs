use numberkey_windows::is_number_key;

use crate::{
    bindings::Windows::Win32::UI::WindowsAndMessaging::{
        VK_BACK, VK_DOWN, VK_END, VK_ESCAPE, VK_HOME, VK_LEFT, VK_NEXT, VK_PRIOR, VK_RETURN,
        VK_RIGHT, VK_SPACE, VK_UP,
    },
    engine::CompositionProcessorEngine,
};

#[repr(C)]
#[derive(PartialEq)]
pub enum KeystrokeCategory {
    None,
    Composing,
    Candidate,
    InvokeCompositionEditSession,
}

#[repr(C)]
#[derive(PartialEq)]
pub enum KeystrokeFunction {
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

    // Function Double/Single byte
    DoubleSingleByte,

    // Function Punctuation
    Punctuation,
}

#[repr(C)]
#[derive(PartialEq)]
pub enum CandidateMode {
    None,
    Original,
    Incremental,
    WithNextComposition,
}

fn map_invariable_keystroke_function(keystroke: u32) -> Option<KeystrokeFunction> {
    match keystroke {
        VK_SPACE => Some(KeystrokeFunction::Convert),
        VK_RETURN => Some(KeystrokeFunction::FinalizeCandidatelist),

        VK_UP => Some(KeystrokeFunction::MoveUp),
        VK_DOWN => Some(KeystrokeFunction::MoveDown),
        VK_PRIOR => Some(KeystrokeFunction::MovePageUp),
        VK_NEXT => Some(KeystrokeFunction::MovePageDown),
        VK_HOME => Some(KeystrokeFunction::MovePageTop),
        VK_END => Some(KeystrokeFunction::MovePageBottom),
        _ => None,
    }
}

fn is_virtual_key_keystroke_composition(code: u32, modifiers: u32) -> bool {
    code >= 'A' as u32 && code <= 'Z' as u32 && modifiers == 0
}

fn is_keystroke_range(code: u32, modifiers: u32, candidate_mode: CandidateMode) -> bool {
    if !is_number_key(code) {
        false
    } else if candidate_mode == CandidateMode::WithNextComposition {
        // Candidate phrase could specify modifier
        modifiers == 0
        // else next composition
    } else {
        candidate_mode != CandidateMode::None
    }
}

pub fn test_virtual_key(
    engine: &CompositionProcessorEngine,
    code: u32,
    ch: char,
    mut composing: bool,
    candidate_mode: CandidateMode,
) -> (bool, KeystrokeCategory, KeystrokeFunction) {
    if candidate_mode == CandidateMode::Original
        || candidate_mode == CandidateMode::WithNextComposition
    {
        composing = false;
    }

    if composing
        || candidate_mode == CandidateMode::Incremental
        || candidate_mode == CandidateMode::None
    {
        if (ch == '*' || ch == '?') && engine.has_virtual_key() {
            return (true, KeystrokeCategory::Composing, KeystrokeFunction::Input);
        } else if engine.keystroke_buffer_includes_wildcard() && code == VK_SPACE {
            return (
                true,
                KeystrokeCategory::Composing,
                KeystrokeFunction::ConvertWildcard,
            );
        }
    }

    let modifiers = engine.modifiers().get();

    // Candidate list could not handle key. We can try to restart the composition.
    if is_virtual_key_keystroke_composition(code, modifiers) {
        return if candidate_mode == CandidateMode::Original {
            (
                true,
                KeystrokeCategory::Candidate,
                KeystrokeFunction::FinalizeCandidatelistAndInput,
            )
        } else {
            (true, KeystrokeCategory::Composing, KeystrokeFunction::Input)
        };
    }

    let mapped_function = map_invariable_keystroke_function(code);
    // System pre-defined keystroke
    if composing {
        if let Some(mapped_function) = mapped_function {
            let category = if candidate_mode == CandidateMode::Incremental {
                KeystrokeCategory::Candidate
            } else {
                KeystrokeCategory::Composing
            };
            return (true, category, mapped_function);
        }
        if candidate_mode != CandidateMode::Incremental {
            match code {
                VK_LEFT => {
                    return (
                        true,
                        KeystrokeCategory::Composing,
                        KeystrokeFunction::MoveLeft,
                    )
                }
                VK_RIGHT => {
                    return (
                        true,
                        KeystrokeCategory::Composing,
                        KeystrokeFunction::MoveRight,
                    )
                }
                VK_ESCAPE => {
                    return (
                        true,
                        KeystrokeCategory::Composing,
                        KeystrokeFunction::Cancel,
                    )
                }
                VK_BACK => {
                    return (
                        true,
                        KeystrokeCategory::Composing,
                        KeystrokeFunction::Backspace,
                    )
                }
                _ => (),
            }
        } else {
            match code {
                // VK_LEFT, VK_RIGHT - set *pIsEaten = false for application could move caret left or right.
                // and for CUAS, invoke _HandleCompositionCancel() edit session due to ignore CUAS default key handler for send out terminate composition
                VK_LEFT | VK_RIGHT => {
                    return (
                        false,
                        KeystrokeCategory::InvokeCompositionEditSession,
                        KeystrokeFunction::Cancel,
                    )
                }
                VK_ESCAPE => {
                    return (
                        true,
                        KeystrokeCategory::Candidate,
                        KeystrokeFunction::Cancel,
                    )
                }
                // VK_BACK - remove one char from reading string.
                VK_BACK => {
                    return (
                        true,
                        KeystrokeCategory::Composing,
                        KeystrokeFunction::Backspace,
                    )
                }
                _ => (),
            }
        }
    }

    if (candidate_mode == CandidateMode::Original)
        || (candidate_mode == CandidateMode::WithNextComposition)
    {
        if let Some(mapped_function) = mapped_function {
            return (true, KeystrokeCategory::Candidate, mapped_function);
        }
        match code {
            VK_BACK => {
                return (
                    true,
                    KeystrokeCategory::Candidate,
                    KeystrokeFunction::Cancel,
                )
            }
            VK_ESCAPE => {
                return if candidate_mode == CandidateMode::WithNextComposition {
                    (
                        true,
                        KeystrokeCategory::InvokeCompositionEditSession,
                        KeystrokeFunction::FinalizeTextstore,
                    )
                } else {
                    (
                        true,
                        KeystrokeCategory::Candidate,
                        KeystrokeFunction::Cancel,
                    )
                }
            }
            _ => (),
        }
    }

    if is_keystroke_range(code, modifiers, candidate_mode) {
        return (
            true,
            KeystrokeCategory::Candidate,
            KeystrokeFunction::SelectByNumber,
        );
    }

    if ch != '\0' {
        return if is_virtual_key_keystroke_composition(code, modifiers) {
            (
                false,
                KeystrokeCategory::Composing,
                KeystrokeFunction::Input,
            )
        } else {
            (
                false,
                KeystrokeCategory::InvokeCompositionEditSession,
                KeystrokeFunction::FinalizeTextstore,
            )
        };
    }

    (false, KeystrokeCategory::None, KeystrokeFunction::None)
}
