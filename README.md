<p align="center"><img src="./favicon.ico" width="100" height="100"></p>
<h1 align="center">FoxNAS Management Client</h1>

<div align="center">

[![star](https://img.shields.io/github/stars/ProgramCX/FoxNAS?logo=github&style=round-square)](https://github.com/ProgramCX/flow_im_app/stargazers)
[![license](https://img.shields.io/github/license/ProgramCX/FoxNAS?style=round-square&logo=github)](https://github.com/ProgramCX/flow_im_app/blob/main/LICENSE)
[![Activity](https://img.shields.io/github/last-commit/ProgramCX/FoxNAS?style=round-square&logo=github)](#)
[![Frame](https://img.shields.io/badge/Frame-Qt6.8-orange.svg?style=round-square&logo=github)](#)

</div>

>

[English](README.md) | 中文

FoxNAS Management Client is a cross-platform NAS administration tool developed with **Qt 6.8**, supporting core features such as **UDP-based device discovery**, **SSH remote access**, **real-time media playback**, and **file management**. It also provides **automatic DDNS updates** and **email notifications** upon IP changes. Designed for embedded systems, home NAS setups, and private server gateways, this client works in tandem with the [FoxNAS Server](https://github.com/ProgramCX/FoxNAServer) to help users manage and utilize their NAS devices more efficiently.

## Features

- **Cross-Platform Support**  
  Compatible with Windows, macOS, and Linux, offering a consistent user experience across systems.

- **Automatic LAN Device Discovery**  
  Supports discovering devices within the local network using UDP broadcast, eliminating the need to manually input IP addresses and simplifying the connection process.

- **File Management**  
  Includes a built-in file browser to remotely access, view, and manage server files with support for upload, download, rename, and delete operations.

- **Real-Time Media Playback**  
  Allows streaming of music and video files (such as MP3, MP4, MKV, etc.) stored on the NAS without the need to download. Supports a wide range of common formats, with ongoing expansion for more.

- **Dynamic DNS (DDNS) Update**  
  Automatically updates domain records via specified APIs (e.g., Tencent Cloud, Alibaba Cloud) when the public IP changes, ensuring consistent remote access.

- **Email Notification Alerts**  
  Sends email alerts when IP address changes are detected, helping users stay informed about the network status of their devices and avoid access issues.

- **Multi-User Configuration**  
  Supports saving multiple user profiles for different devices, with account and permission management for multi-user or multi-device scenarios.

## Development & Compilation

This project is built with **Qt 6.8.1** and supports the following compilers:
- Qt 6.8.1 MSVC 2022 64-bit
- Qt 6.8.1 MinGW 64-bit
- GCC / G++

## Contributing

We welcome all forms of contributions, including code, documentation, UI design, bug reports, and suggestions. Please participate via issues or pull requests.

## License

FoxNAS is licensed under the [GPL 3.0 License](LICENSE).

---

> This project is currently under active development. The first stable release is expected within two months. Stay tuned and feel free to share your feedback and suggestions!
