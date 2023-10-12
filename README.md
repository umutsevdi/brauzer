<p align="center">
  <a href="https://github.com/umutsevdi/brauzer">
  <h3 align="center">Brauzer | Браузер</h3>
  </a>

<p align="center">  
A graphical web browser from scratch in C programming language that implements
various web protocols.
  <br/><i>Being actively developed by <a href="https://github.com/umutsevdi">
    Umut Sevdi</a></i> 
<!--<p align="center"><a href="docs.md"><strong>Explore the docs »
</strong></a></p>-->

## Definition

### Status: WORK IN PROGRESS
#### Back-end:

- [x] TCP Session Management
- [x] TLS/SSL System
- [x] Protocol Spesific Calls
- [x] User Agent
- [x] URI Parser
- [ ] CLI Interface
- [ ] **Supported Protocols**:
    - [ ] HTTP
        - [x] HTTP 1.1 Polling
        - [x] Success
        - [x] Header Parser
        - [ ] Post/Put (Partially)
        - [ ] Redirect
        - [ ] HTML Parser
        - [ ] Link Analysis and Installation
    - [ ] Gemini:
        - [x] Success
        - [x] Polling
        - [ ] Input
        - [ ] Redirect
        - [ ] Gemtext Parser
    - [ ] Gopher:
    - [ ] FTP/SFTP:
#### User Interface:
**STATUS**: Not started


<p id="installation">

### Installation

Requirements: 
* [OpenSSL](https://packages.debian.org/bookworm/openssl)
* [libglib2.0-0](https://packages.debian.org/bookworm/libglib2.0-0)

 Current version does not require the following packages, But the future versions will require:

* [libgtk-4](https://packages.debian.org/bookworm/libgtk-4-1)
* [libadwaita](https://packages.debian.org/bookworm/libs/libadwaita-1-0)

1. Clone the repository.

```sh
   git clone https://github.com/umutsevdi/brauzer.git
```

2. Compile the program.
```sh
    make
```

## 5. License

Distributed under the MIT License. See `LICENSE` for more information.

<p id="contact">

## 6. Contact

Feel free to contact me for any further information or suggestion.

Project: [umutsevdi/brauzer](https://github.com/umutsevdi/brauzer)

<i>Developed by <a href="https://github.com/umutsevdi">Umut Sevdi</a>
