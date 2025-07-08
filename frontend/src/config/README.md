# API配置说明

## 概述

本项目将API端点配置集中管理，便于维护和修改。所有API相关的配置都在 `api.ts` 文件中定义。

## 文件结构

```
src/
├── config/
│   ├── api.ts          # API配置主文件
│   └── README.md       # 本文件
└── services/
    └── apiService.ts   # API服务封装
```

## 配置内容

### 1. API基础URL
```typescript
export const API_BASE_URL = 'http://localhost:5000'
```

### 2. API端点配置
```typescript
export const API_ENDPOINTS = {
  // 健康检查
  health: '/api/health',
  
  // OpenMP线程数管理
  omp: {
    threads: '/api/omp/threads'
  },
  
  // 图像变换API
  transform: {
    // 基础变换
    grayscale: '/api/transform/grayscale',
    binary: '/api/transform/binary',
    brightness: '/api/transform/brightness',
    
    // 卷积操作
    gaussianBlur: '/api/transform/gaussian_blur',
    sobel: '/api/transform/sobel',
    convolution: '/api/transform/convolution',
    
    // 图像拼接
    stitch: '/api/transform/stitch'
  },
  
  // 文件下载
  files: '/api/files'
}
```

## 使用方法

### 1. 直接使用配置

```typescript
import { buildApiUrl, buildFileUrl, API_ENDPOINTS } from '../config/api'

// 构建API URL
const url = buildApiUrl(API_ENDPOINTS.transform.grayscale)

// 构建文件下载URL
const fileUrl = buildFileUrl('output.jpg')
```

### 2. 使用API服务（推荐）

```typescript
import { ImageProcessingService } from '../services/apiService'

// 处理图像并下载结果
const formData = new FormData()
formData.append('image', blob)
const resultBlob = await ImageProcessingService.processAndDownload('grayscale', formData)
```

## 服务类

### ImageProcessingService
- `processImage(endpoint, formData)`: 处理图像变换
- `downloadFile(filename)`: 下载处理后的文件
- `processAndDownload(endpoint, formData)`: 处理图像并下载结果

### HealthService
- `checkHealth()`: 检查API健康状态

### OmpService
- `getThreads()`: 获取当前线程数
- `setThreads(threads)`: 设置线程数

## 类型定义

```typescript
// API响应类型
export interface ApiResponse {
  success: boolean
  message?: string
  output_file?: string
  error?: string
}

// 图像处理参数类型
export interface ImageTransformParams {
  threshold?: number
  adjustment?: number
  kernel_size?: number
  sigma?: number
  kernel?: string
  scale?: number
}
```

## 优势

1. **集中管理**: 所有API端点在一个地方配置
2. **类型安全**: 使用TypeScript提供类型检查
3. **易于维护**: 修改API地址只需要改一个地方
4. **代码复用**: 通过服务类封装常用操作
5. **错误处理**: 统一的错误处理机制

## 注意事项

1. 修改API地址时，只需要修改 `API_BASE_URL` 常量
2. 添加新的API端点时，在 `API_ENDPOINTS` 中添加相应的配置
3. 建议使用服务类而不是直接调用fetch，以获得更好的错误处理和代码复用 