## 问题原因

Qt 端 `ApiRequest::sendRequest()` 中虽然检测了 443 端口并尝试切换到 HTTPS，但存在 Bug：

1. URL 转换后只更新了局部变量 `urlApi`，**没有同步更新 `request` 对象的 URL**
2. 实际网络请求使用的是 `request` 对象，导致仍然发送 HTTP 请求到 HTTPS 端口

## 修复步骤

### 1. 修复 ApiRequest.cpp

在 `sendRequest()` 方法中，将 HTTPS 转换逻辑提前，并确保 `request` 对象的 URL 也被更新：

```cpp
void ApiRequest::sendRequest()
{
    if (api.isEmpty()) {
        qDebug() << "API 地址为空";
        return;
    }

    QString token = getToken();

    if (!urlApi.toString().contains("/api/status/status")) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", QString("Bearer " + token).toUtf8());
    }

    // 修复：在设置 request URL 之前进行协议转换
    if (urlApi.toString().contains(":443")) {
        urlApi = QUrl("https" + urlApi.toString().mid(4));
        request.setUrl(urlApi);  // 关键：同步更新 request 的 URL
    }
    
    qDebug() << "正在发送API请求..\n"
             << "请求token:" << token << "\n请求Url：" << urlApi.toString() << "\n请求体："
             << body.toJson() << "\n";

    // ... 后续代码不变
}
```

### 2. 同样修复 refreshToken() 方法

该方法也有相同的 Bug：

```cpp
bool ApiRequest::refreshToken()
{
    // ... 前置代码不变

    QUrl url(getFullApiPath(FULLHOST, NASREFRESHTOKEN));
    if (url.toString().contains(":443")) {
        url = QUrl("https" + url.toString().mid(4));
        // 需要确保后续 request 使用更新后的 url
    }
    QNetworkRequest request;
    request.setUrl(url);  // 这里已经正确设置了
    
    // ... 后续代码不变
}
```

### 3. 可选：改进 getFullApiPath 函数

在 `ApiUrl.cpp` 中，根据端口自动选择协议：

```cpp
QString getFullApiPath(QString host, QString api)
{
    // 如果端口是 443，使用 HTTPS
    if (host.contains(":443")) {
        return "https://" + host + api;
    }
    return "http://" + host + api;
}
```

## 验证步骤

1. 修改代码后重新编译 Qt 客户端
2. 测试通过 443 端口连接服务器
3. 确认 `/api/status/status` 请求返回 200 而不是 502