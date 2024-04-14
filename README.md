# Spike Devices

For spike device plugins.

## Dependencies

- `riscv-toolchains`
- If you are using this within Chipyard, the dependencies will be already installed in the `$RISCV` directory.

## Quick Start

Make sure `$RISCV` is set before running the following commands.

```bash
make
make install
```

Example usage:

```bash
spike --extlib libspikedevices.so --device sifive_uart ./hello.riscv
```
