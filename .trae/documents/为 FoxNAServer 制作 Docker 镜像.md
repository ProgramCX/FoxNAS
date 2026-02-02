## 任务概览
为 FoxNAServer 创建生产级 Docker 镜像，并推送到 Docker Hub 供他人使用。

## 执行步骤

1. **创建 Dockerfile** - 使用多阶段构建，基于 Eclipse Temurin JDK 21 镜像
2. **创建 .dockerignore** - 排除不需要的文件，减小镜像体积
3. **创建 docker-compose.yml**（可选）- 方便用户一键启动所有依赖服务
4. **构建 Docker 镜像** - 本地构建并测试
5. **推送到 Docker Hub** - 上传镜像供他人拉取使用

## 镜像特点
- 使用多阶段构建，最终镜像体积小（仅包含 JRE 和 jar 包）
- 支持通过环境变量配置数据库连接
- 健康检查配置
- 非 root 用户运行，提高安全性

请确认此计划后，我将开始执行。