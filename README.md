# Zephyr Programs

This repo contains a list of programs written using Zephyr RTOS for the
following boards

1. [STM32 Nucleo L433RC-P](./nucleo-l433rc-p/README.md)
2. [Nordic nrf52840dk](./nrf52840dk/README.md)

## Steps for reproduction of environment

Install the Zephyr SDK by following the instructions on their
[official documentation](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html#toolchain-zephyr-sdk-install)

Create a `$HOME/zephyrproject/` directory to store the source code of Zephyr
RTOS

After running the commands given in this part of the
[documentation](https://docs.zephyrproject.org/latest/develop/getting_started/index.html#getting-started-guide)
which is *Getting Started* and activating the Virtual Environment, make sure to
assign the environment variable `ZEPHYR_BASE` to the path where you have stored
`zephyrproject/` folder i.e if it is in `$HOME` add the following line to your
`~/.bashrc` or `~/.zshrc`

```bash
export ZEPHYR_BASE="$HOME/zephyrproject/zephyr/"
```

This allows you to create zephyr apps anywhere in your system.

## Scripts

I have created 2 scripts `batch_build.d` and `batch_clean.d` which allows you to
build all projects and remove build artifacts from all projects respectively.

Just run:

```bash 
./scripts/batch_build.d          # To build 
./scripts/batch_clean.d          # To clean
```
