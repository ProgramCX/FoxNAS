<p align="center"><img src="./favicon.ico" width="100" height="100"></p>
<h1 align="center">FoxNAS 管理客户端</h1>

<div align="center">

[![star](https://img.shields.io/github/stars/ProgramCX/FoxNAS?logo=github&style=round-square)](https://github.com/ProgramCX/flow_im_app/stargazers)
[![license](https://img.shields.io/github/license/ProgramCX/FoxNAS?style=round-square&logo=github)](https://github.com/ProgramCX/flow_im_app/blob/main/LICENSE)
[![Activity](https://img.shields.io/github/last-commit/ProgramCX/FoxNAS?style=round-square&logo=github)](#)
[![Frame](https://img.shields.io/badge/Frame-Qt6.8-orange.svg?style=round-square&logo=github)](#)

</div>

> 

[English](README.md) | 中文

FoxNAS 管理客户端是一个基于 Qt 6.8 开发的 NAS 管理客户端工具，支持 UDP 广播发现设备、SSH 远程连接、媒体实时播放、文件管理 等核心功能，并可在 IP 变动时自动更新 DDNS 并发送邮件通知，适用于嵌入式设备、家庭 NAS、私有服务器网关等场景。该管理工具搭配 [FoxNAS 服务器端](https://github.com/ProgramCX/FoxNAServer)使用，帮助用户更高效地管理和使用 NAS 设备。

## 功能
- **跨平台支持**：支持 Windows、macOS 和 Linux 等多个操作系统，确保用户在不同设备上都能享受一致的使用体验。

- **局域网自动发现**：支持通过 UDP 广播自动发现局域网内的服务器设备，无需手动输入 IP 地址，提升连接效率与使用便捷性。

- **文件管理**：集成文件浏览器，用户可远程访问、查看和管理服务器中的文件，实现上传、下载、重命名、删除等常用操作。

- **实时媒体播放**：支持在线播放 NAS 中存储的音乐与视频文件（如 MP3、MP4、MKV 等），无需下载即可直接播放，兼容多种常见格式，后续将持续扩展对更多媒体格式的支持。

- **动态域名更新（DDNS）**：设备公网 IP 变动时自动调用指定 API 更新域名解析（支持腾讯云、阿里云等 DDNS 服务），确保远程访问地址始终可用。

- **邮件通知提醒**：当检测到 IP 变更时自动发送邮件通知用户，实时掌握设备网络状态，避免远程连接失败。

- **多用户配置支持**：可为不同设备配置多个用户信息，支持账户管理与权限区分，适用于多设备/多人使用场景。

## 开发与编译
本项目基于 **Qt 6.8.1** 版本，支持的编译器有：
- Qt 6.8.1 MSVC 2022 64bit
- Qt 6.8.1 MinGW 64bit
- GCC / G++

## 贡献

欢迎任何形式的贡献，包括但不限于代码、文档、UI设计、问题反馈等。请通过 Issue 或 PR 参与社区共建。

## 协议

FoxNAS 使用 [GPL 3.0 License](LICENSE) 协议.

---

> 当前项目正在积极开发中，预计在两个月内发布首个稳定版 Release。欢迎关注项目进展并提出宝贵建议！