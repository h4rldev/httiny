# HTTiny

A tiny little HTTP server for learning purposes and how C translates to amd64 assembly.

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [Features](#features)
- [Dependencies](#dependencies)
- [License](#license)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Features

HTTiny uses only the standard C library, and is written using gnu11.
(httiny_assert requires GNU extensions for now, but i'll get to it.).

- [x] HTTP/1.1 support
- [ ] HTTP/1.0 support (non chunked responses)
- [x] Handlers and Handler registration
- [x] "Secure" File serving
- [ ] "Secure" Directory serving
- [ ] Custom 404 file serving

- [ ] Make into library

## Dependencies

- libmagic (for mime type detection)

## License

This project is licensed under the BSD 3-Clause License - see the [LICENSE](LICENSE) file for details.
