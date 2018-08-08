<a href="https://www.bigclown.com/"><img src="https://bigclown.sirv.com/logo.png" width="200" alt="BigClown Logo" align="right"></a>

# Firmware for BigClown Radio LCD Thermostat

[![Travis](https://img.shields.io/travis/bigclownlabs/bcf-radio-lcd-thermostat/master.svg)](https://travis-ci.org/bigclownlabs/bcf-radio-lcd-thermostat)
[![Release](https://img.shields.io/github/release/bigclownlabs/bcf-radio-lcd-thermostat.svg)](https://github.com/bigclownlabs/bcf-radio-lcd-thermostat/releases)
[![License](https://img.shields.io/github/license/bigclownlabs/bcf-radio-lcd-thermostat.svg)](https://github.com/bigclownlabs/bcf-radio-lcd-thermostat/blob/master/LICENSE)
[![Twitter](https://img.shields.io/twitter/follow/BigClownLabs.svg?style=social&label=Follow)](https://twitter.com/BigClownLabs)

See the project documentation on this link:

**https://www.bigclown.com/doc/projects/radio-lcd-thermostat/**

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
