# 图像处理后端服务

这是一个基于Flask的图像处理API服务，提供图像变换功能。

## 功能特性

- 图像上传和验证
- 支持多种图像格式（PNG, JPG, JPEG, GIF, BMP, TIFF）
- 图像变换API（灰度化、二值化、亮度调整）
- 跨域请求支持
- 文件大小限制（16MB）

## 安装依赖

```bash
pip install -r requirements.txt
```

## 运行服务

```bash
python app.py
```

服务将在 `http://localhost:5000` 启动

## API接口

### 1. 健康检查
- **GET** `/api/health`
- 返回服务状态

### 2. 图像变换
- **POST** `/api/transform/<transform_type>`
- 参数：
  - `transform_type`: 变换类型（grayscale, binary, brightness）
  - `image`: 上传的图像文件
- 返回：处理后的图像文件

### 3. 根路径
- **GET** `/`
- 返回API服务信息

## 支持的变换类型

- `grayscale`: 灰度化
- `binary`: 二值化
- `brightness`: 亮度调整

## 文件结构

```
backend/
├── app.py              # 主应用文件
├── requirements.txt    # Python依赖
├── README.md          # 说明文档
└── uploads/           # 上传文件存储目录（自动创建）
``` 