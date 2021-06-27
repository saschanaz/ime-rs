$scriptDir = Split-Path -Path $MyInvocation.MyCommand.Definition -Parent
pushd $scriptDir/../rust
cargo build
cbindgen --crate composition_processor --output ../cpp/SampleIME/cbindgen/composition_processor.h
cbindgen --crate dictionary_parser --output ../cpp/SampleIME/cbindgen/dictionary_parser.h
cbindgen --crate globals --output ../cpp/SampleIME/cbindgen/globals.h
cbindgen --crate numberkey_windows --output ../cpp/SampleIME/cbindgen/numberkey_windows.h
cbindgen --crate ruststringrange --output ../cpp/SampleIME/cbindgen/ruststringrange.h
popd
