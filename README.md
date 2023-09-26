# Palladium ACPI module Test Image

This is a Limine kernel containing the bare minimum to execute the Palladium ACPI module.

Currently, this is used for testing in UEFI mode (as bootmgr.exe cannot boot in UEFI mode).

As with the ACPI module in the Palladium source tree, kernel/acpi/amd64 and kernel/acpi/os.c are OS/arch specific; None of the other files were modified compared to Palladium.

kernel/crt contains the required CRT functions (taken from the Palladium CRT) to make the ACPI module work.
