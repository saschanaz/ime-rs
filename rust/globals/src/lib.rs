use windows::core::GUID;

//---------------------------------------------------------------------
// SampleIME CLSID
//---------------------------------------------------------------------
// {89adf27c-0863-4ad3-a463-f7ecfc75d01d}
#[no_mangle]
pub static SAMPLEIME_CLSID: GUID = GUID::from_u128(0x89adf27c_0863_4ad3_a463_f7ecfc75d01d);

//---------------------------------------------------------------------
// Profile GUID
//---------------------------------------------------------------------
// {2b574d86-3844-4136-8dd9-41e7829ac9f8}
#[no_mangle]
pub static SAMPLEIME_GUID_PROFILE: GUID = GUID::from_u128(0x2b574d86_3844_4136_8dd9_41e7829ac9f8);

//---------------------------------------------------------------------
// PreserveKey GUID
//---------------------------------------------------------------------
// {27579631-4620-4640-b3e0-ae24541c216c}
#[no_mangle]
pub static SAMPLEIME_GUID_IME_MODE_PRESERVE_KEY: GUID =
    GUID::from_u128(0x27579631_4620_4640_b3e0_ae24541c216c);

// {8b5255fb-0ca8-431b-a54b-b06a060c73f4}
#[no_mangle]
pub static SAMPLEIME_GUID_DOUBLE_SINGLE_BYTE_PRESERVE_KEY: GUID =
    GUID::from_u128(0x8b5255fb_0ca8_431b_a54b_b06a060c73f4);

// {04250a07-9b7b-4b9d-b7a9-8dc3a62c71d1}
#[no_mangle]
pub static SAMPLEIME_GUID_PUNCTUATION_PRESERVE_KEY: GUID =
    GUID::from_u128(0x04250a07_9b7b_4b9d_b7a9_8dc3a62c71d1);

//---------------------------------------------------------------------
// Compartments
//---------------------------------------------------------------------
// {88677281-8ae0-4b88-bb6b-a36101cc0eea}
#[no_mangle]
pub static SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE: GUID =
    GUID::from_u128(0x88677281_8ae0_4b88_bb6b_a36101cc0eea);

// {0a83989c-1b64-4374-ae3d-e60b7208b36f}
#[no_mangle]
pub static SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION: GUID =
    GUID::from_u128(0x0a83989c_1b64_4374_ae3d_e60b7208b36f);

//---------------------------------------------------------------------
// LanguageBars
//---------------------------------------------------------------------

// {9826a3f7-a822-43f7-a51b-fbe7b55995d3}
#[no_mangle]
pub static SAMPLEIME_GUID_DISPLAY_ATTRIBUTE_INPUT: GUID =
    GUID::from_u128(0x9826a3f7_a822_43f7_a51b_fbe7b55995d3);

// {36a97adf-e994-4164-b271-5f169689093e}
#[no_mangle]
pub static SAMPLEIME_GUID_DISPLAY_ATTRIBUTE_CONVERTED: GUID =
    GUID::from_u128(0x36a97adf_e994_4164_b271_5f169689093e);

//---------------------------------------------------------------------
// UI element
//---------------------------------------------------------------------

// {c2bc76f1-9b5b-4a88-9620-8ce35d368457}
#[no_mangle]
pub static SAMPLEIME_GUID_CAND_UIELEMENT: GUID =
    GUID::from_u128(0xc2bc76f1_9b5b_4a88_9620_8ce35d368457);
