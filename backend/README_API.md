# 图像处理API使用说明

## 概述

这是一个基于Flask的图像处理API服务，集成了C++高性能图像处理功能。所有图像处理操作都通过pybind11封装的C++模块实现，支持OpenMP并行计算。

## 安装和运行

### 1. 安装依赖

```bash
pip install flask flask-cors requests
```

### 2. 确保C++模块可用

确保`image_processing.cpython-*.so`文件在backend目录下。

### 3. 启动服务

```bash
cd backend
python app.py
```

服务将在 `http://localhost:5000` 启动。

## API端点

### 1. 健康检查

**GET** `/api/health`

检查服务状态和图像处理模块可用性。

**响应示例：**
```json
{
  "status": "healthy",
  "message": "图像处理服务运行正常",
  "image_processing_available": true
}
```

### 2. 灰度转换

**POST** `/api/transform/grayscale`

将彩色图像转换为灰度图像。

**请求：**
- `image`: 上传的图像文件

**响应示例：**
```json
{
  "success": true,
  "message": "灰度转换完成",
  "cpp_time": 0.001234,
  "total_time": 0.001456,
  "output_file": "gray_uuid.bmp"
}
```

### 3. 二值化转换

**POST** `/api/transform/binary`

将图像转换为二值图像。

**请求：**
- `image`: 上传的图像文件
- `threshold`: 阈值（可选，默认128）

**响应示例：**
```json
{
  "success": true,
  "message": "二值化转换完成",
  "threshold": 128,
  "processing_time": 0.002345,
  "output_file": "binary_uuid.bmp"
}
```

### 4. 亮度调整

**POST** `/api/transform/brightness`

调整图像亮度。

**请求：**
- `image`: 上传的图像文件
- `adjustment`: 亮度调整值（可选，默认50）

**响应示例：**
```json
{
  "success": true,
  "message": "亮度调整完成",
  "adjustment": 50,
  "processing_time": 0.003456,
  "output_file": "bright_uuid.bmp"
}
```

### 5. 高斯模糊

**POST** `/api/transform/gaussian_blur`

对图像应用高斯模糊。

**请求：**
- `image`: 上传的图像文件
- `kernel_size`: 卷积核大小（可选，默认5）
- `sigma`: 高斯函数标准差（可选，默认1.0）

**响应示例：**
```json
{
  "success": true,
  "message": "高斯模糊完成",
  "kernel_size": 5,
  "sigma": 1.0,
  "processing_time": 0.004567,
  "output_file": "blur_uuid.bmp"
}
```

### 6. Sobel边缘检测

**POST** `/api/transform/sobel`

使用Sobel算子进行边缘检测。

**请求：**
- `image`: 上传的图像文件

**响应示例：**
```json
{
  "success": true,
  "message": "Sobel边缘检测完成",
  "processing_time": 0.005678,
  "output_file": "sobel_uuid.bmp"
}
```

### 7. 自定义卷积

**POST** `/api/transform/convolution`

应用自定义卷积核。

**请求：**
- `image`: 上传的图像文件
- `kernel`: 卷积核（JSON格式，可选，默认锐化卷积核）
- `scale`: 缩放因子（可选，默认1）

**卷积核示例：**
```json
[[0,-1,0],[-1,5,-1],[0,-1,0]]  // 锐化卷积核
```

**响应示例：**
```json
{
  "success": true,
  "message": "自定义卷积完成",
  "kernel": [[0,-1,0],[-1,5,-1],[0,-1,0]],
  "scale": 1,
  "processing_time": 0.006789,
  "output_file": "conv_uuid.bmp"
}
```

### 8. 图像拼接

**POST** `/api/transform/stitch`

拼接两张图像。

**请求：**
- `image1`: 第一张图像文件
- `image2`: 第二张图像文件

**响应示例：**
```json
{
  "success": true,
  "message": "图像拼接完成",
  "cpp_time": 0.007890,
  "total_time": 0.008123,
  "output_file": "stitched_uuid.bmp"
}
```

### 9. OpenMP线程管理

#### 获取线程数

**GET** `/api/omp/threads`

获取当前OpenMP最大线程数。

**响应示例：**
```json
{
  "success": true,
  "max_threads": 8
}
```

#### 设置线程数

**POST** `/api/omp/threads`

设置OpenMP线程数。

**请求：**
```json
{
  "threads": 4
}
```

**响应示例：**
```json
{
  "success": true,
  "message": "已设置线程数为 4"
}
```

### 10. 文件下载

**GET** `/api/files/<filename>`

下载处理后的图像文件。

## 使用示例

### Python客户端示例

```python
import requests

# 灰度转换
with open('test.jpg', 'rb') as f:
    files = {'image': f}
    response = requests.post('http://localhost:5000/api/transform/grayscale', files=files)
    result = response.json()
    print(f"处理完成: {result['output_file']}")

# 下载处理后的文件
if result['success']:
    file_response = requests.get(f"http://localhost:5000/api/files/{result['output_file']}")
    with open('result.bmp', 'wb') as f:
        f.write(file_response.content)
```

### cURL示例

```bash
# 灰度转换
curl -X POST -F "image=@test.jpg" http://localhost:5000/api/transform/grayscale

# 二值化转换
curl -X POST -F "image=@test.jpg" -F "threshold=128" http://localhost:5000/api/transform/binary

# 设置OpenMP线程数
curl -X POST -H "Content-Type: application/json" -d '{"threads": 4}' http://localhost:5000/api/omp/threads
```

## 错误处理

所有API都返回标准的HTTP状态码：

- `200`: 成功
- `400`: 请求参数错误
- `404`: 文件不存在
- `500`: 服务器内部错误

错误响应格式：
```json
{
  "error": "错误描述信息"
}
```

## 性能优化

1. **OpenMP并行计算**: 所有图像处理操作都支持OpenMP并行计算
2. **C++高性能实现**: 核心算法使用C++实现，通过pybind11封装
3. **文件缓存**: 处理后的文件保存在uploads目录，避免重复处理

## 注意事项

1. 支持的文件格式：PNG, JPG, JPEG, GIF, BMP, TIFF
2. 最大文件大小：16MB
3. 处理后的文件会保存在uploads目录
4. 建议在生产环境中配置适当的文件清理策略 