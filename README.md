# CHASE backends

This repository contains the standard backends of CHASE (Contract-based
Heterogeneous Analysis and Systems Exploration). The backends encode CHASE
problems into the input languages of different external tools:

- **GR1CPrinter**: specifications for [gr1c][GR1C] (`.spc` files); a
  relation between two variables, or with arithmetic, is not written;
- **SlugsPrinter**: structured specifications for [slugs][SLUGS]
  (`.structuredSlugs` files);
- **NuSMVPrinter**: models for [NuSMV][NUSMV] (`.smv` files), used to verify
  refinement, consistency and compatibility of contracts;
- **PySTLPrinter**: scripts for PySTL, written for an early version of
  PyCASSE; they do not run with the current one.

The backends are compiled both as a C++ library (`libchase-backends`) and as a
Python module (`pychase_backends`). The module requires the
[core-library][CORE] to be installed first.

For the installation, please refer to the [CHASE website][WEB].

## License

This project is released under the 3-Clause BSD License.

[GR1C]: https://github.com/tulip-control/gr1c
[SLUGS]: https://github.com/VerifiableRobotics/slugs
[NUSMV]: https://nusmv.fbk.eu/
[CORE]: https://github.com/chase-cps/core-library
[WEB]: https://chase-cps.github.io/doc/md/install.html
