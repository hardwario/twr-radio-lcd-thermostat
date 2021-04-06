<a href="https://www.hardwario.com/"><img src="https://www.hardwario.com/ci/assets/hw-logo.svg" width="200" alt="HARDWARIO Logo" align="right"></a>

# Firmware for HARDWARIO Radio LCD Thermostat

[![Travis](https://travis-ci.org/hardwario/twr-radio-lcd-thermostat.svg?branch=master)](https://travis-ci.org/hardwario/twr-radio-lcd-thermostat)[![Release](https://img.shields.io/github/release/bigclownlabs/bcf-radio-lcd-thermostat.svg)](https://github.com/bigclownlabs/bcf-radio-lcd-thermostat/releases)
[![License](https://img.shields.io/github/license/bigclownlabs/bcf-radio-lcd-thermostat.svg)](https://github.com/bigclownlabs/bcf-radio-lcd-thermostat/blob/master/LICENSE)
[![Twitter](https://img.shields.io/twitter/follow/hardwario_en.svg?style=social&label=Follow)](https://twitter.com/hardwario_en)

See the project documentation on this link:

**https://developers.hardwario.com/projects/radio-lcd-thermostat/**

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
