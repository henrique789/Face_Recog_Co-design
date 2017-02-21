set_global_assignment -entity "FP_test" -library "lib_FP_test" -name IP_TOOL_NAME "altera_fp_functions"
set_global_assignment -entity "FP_test" -library "lib_FP_test" -name IP_TOOL_VERSION "15.0"
set_global_assignment -entity "FP_test" -library "lib_FP_test" -name IP_TOOL_ENV "mwpim"
set_global_assignment -library "lib_FP_test" -name SPD_FILE [file join $::quartus(sip_path) "FP_test.spd"]

set_global_assignment -library "lib_FP_test" -name MISC_FILE [file join $::quartus(sip_path) "FP_test_sim/dspba_library_package.vhd"]
set_global_assignment -library "lib_FP_test" -name MISC_FILE [file join $::quartus(sip_path) "FP_test_sim/dspba_library.vhd"]
set_global_assignment -library "lib_FP_test" -name MISC_FILE [file join $::quartus(sip_path) "FP_test_sim/FP_test.vhd"]
