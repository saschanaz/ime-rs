use crate::engine::CompositionProcessorEngine;
use windows::Win32::UI::Input::KeyboardAndMouse::{
    VK_BACK, VK_DOWN, VK_END, VK_ESCAPE, VK_HOME, VK_LEFT, VK_NEXT, VK_PRIOR, VK_RETURN, VK_RIGHT,
    VK_SPACE, VK_UP,
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
#[derive(PartialEq, Clone, Copy)]
pub enum CandidateMode {
    None,
    Original,
    Incremental,
    WithNextComposition,
}

fn map_invariable_keystroke_function(keystroke: u16) -> Option<KeystrokeFunction> {
    match keystroke {
        k if k == VK_SPACE.0 => Some(KeystrokeFunction::Convert),
        k if k == VK_RETURN.0 => Some(KeystrokeFunction::FinalizeCandidatelist),

        k if k == VK_UP.0 => Some(KeystrokeFunction::MoveUp),
        k if k == VK_DOWN.0 => Some(KeystrokeFunction::MoveDown),
        k if k == VK_PRIOR.0 => Some(KeystrokeFunction::MovePageUp),
        k if k == VK_NEXT.0 => Some(KeystrokeFunction::MovePageDown),
        k if k == VK_HOME.0 => Some(KeystrokeFunction::MovePageTop),
        k if k == VK_END.0 => Some(KeystrokeFunction::MovePageBottom),
        _ => None,
    }
}

fn character_affects_keystroke_composition(ch: char, modifiers: u32) -> bool {
    // Normally [a-z] are only relevant since composition does not happen with Shift,
    // but KEYEVENTF_UNICODE can dispatch A-Z without Shift and that's allowed here
    // to maintain the original behavior.
    ('A'..='z').contains(&ch) && modifiers == 0
}

fn is_keystroke_range(ch: char, modifiers: u32, candidate_mode: CandidateMode) -> bool {
    if !ch.is_numeric() {
        false
    } else if candidate_mode == CandidateMode::WithNextComposition {
        // Candidate phrase could specify modifier
        modifiers == 0
        // else next composition
    } else {
        candidate_mode != CandidateMode::None
    }
}

/// Test virtual key code need to the Composition Processor Engine.
/// If engine need this virtual key code, returns true. Otherwise returns false.
/// Returns function regarding virtual key.
pub fn test_virtual_key(
    engine: &CompositionProcessorEngine,
    code: u16,
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
        if (ch == '*' || ch == '?') && engine.keystroke_buffer().has_virtual_key() {
            return (true, KeystrokeCategory::Composing, KeystrokeFunction::Input);
        } else if engine.keystroke_buffer().includes_wildcard() && code == VK_SPACE.0 {
            return (
                true,
                KeystrokeCategory::Composing,
                KeystrokeFunction::ConvertWildcard,
            );
        }
    }

    let modifiers = engine.modifiers().get();

    // Candidate list could not handle key. We can try to restart the composition.
    if character_affects_keystroke_composition(ch, modifiers) {
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
                c if c == VK_LEFT.0 => {
                    return (
                        true,
                        KeystrokeCategory::Composing,
                        KeystrokeFunction::MoveLeft,
                    )
                }
                c if c == VK_RIGHT.0 => {
                    return (
                        true,
                        KeystrokeCategory::Composing,
                        KeystrokeFunction::MoveRight,
                    )
                }
                c if c == VK_ESCAPE.0 => {
                    return (
                        true,
                        KeystrokeCategory::Composing,
                        KeystrokeFunction::Cancel,
                    )
                }
                c if c == VK_BACK.0 => {
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
                c if c == VK_LEFT.0 || c == VK_RIGHT.0 => {
                    return (
                        false,
                        KeystrokeCategory::InvokeCompositionEditSession,
                        KeystrokeFunction::Cancel,
                    )
                }
                c if c == VK_ESCAPE.0 => {
                    return (
                        true,
                        KeystrokeCategory::Candidate,
                        KeystrokeFunction::Cancel,
                    )
                }
                // VK_BACK - remove one char from reading string.
                c if c == VK_BACK.0 => {
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
            c if c == VK_BACK.0 => {
                return (
                    true,
                    KeystrokeCategory::Candidate,
                    KeystrokeFunction::Cancel,
                )
            }
            c if c == VK_ESCAPE.0 => {
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

    if is_keystroke_range(ch, modifiers, candidate_mode) {
        return (
            true,
            KeystrokeCategory::Candidate,
            KeystrokeFunction::SelectByNumber,
        );
    }

    if ch != '\0' {
        return if character_affects_keystroke_composition(ch, modifiers) {
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
