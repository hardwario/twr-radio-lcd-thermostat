<a href="https://www.bigclown.com/"><img src="https://bigclown.sirv.com/logo.png" width="200" alt="BigClown Logo" align="right"></a>

# Firmware for BigClown Thermostat Kit

[![Travis](https://img.shields.io/travis/bigclownlabs/bcf-kit-wireless-lcd-thermostat/master.svg)](https://travis-ci.org/bigclownlabs/bcf-kit-wireless-lcd-thermostat)
[![Release](https://img.shields.io/github/release/bigclownlabs/bcf-kit-wireless-lcd-thermostat.svg)](https://github.com/bigclownlabs/bcf-kit-wireless-lcd-thermostat/releases)
[![License](https://img.shields.io/github/license/bigclownlabs/bcf-kit-wireless-lcd-thermostat.svg)](https://github.com/bigclownlabs/bcf-kit-wireless-lcd-thermostat/blob/master/LICENSE)
[![Twitter](https://img.shields.io/twitter/follow/BigClownLabs.svg?style=social&label=Follow)](https://twitter.com/BigClownLabs)

This repository contains firmware for Thermostat Kit

## Build

For build use this commands

        make

For build with LCD rotation support use this commands for core R2.x

        make clean
        make -j4 ROTATE_SUPPORT=1 CORE_R=2

For build with LCD rotation support use this commands for core R1.3

        make clean
        make -j4 ROTATE_SUPPORT=1


## License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT/) - see the [LICENSE](LICENSE) file for details.

---

Made with &#x2764;&nbsp; by [**HARDWARIO s.r.o.**](https://www.hardwario.com/) in the heart of Europe.
