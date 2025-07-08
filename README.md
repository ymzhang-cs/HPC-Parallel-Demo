# HPC 并行计算图像处理演示项目

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python](https://img.shields.io/badge/Python-3.8+-blue.svg)](https://www.python.org/)
[![Vue.js](https://img.shields.io/badge/Vue.js-3.5+-green.svg)](https://vuejs.org/)
[![OpenMP](https://img.shields.io/badge/OpenMP-并行计算-orange.svg)](https://www.openmp.org/)

一个基于高性能并行计算的图像处理Web应用，结合了C++并行算法、Python后端API和Vue.js前端界面，展示了现代HPC技术在图像处理领域的应用。

## 🚀 项目特性

### 核心功能
- **并行图像处理**: 使用OpenMP实现的高性能C++并行算法
- **多种图像变换**: 灰度化、二值化、亮度调整、高斯模糊、边缘检测
- **图像拼接**: 基于SURF特征点的图像拼接功能
- **性能对比**: 并行版本与串行版本的性能对比
- **Web界面**: 现代化的Vue.js前端界面

### 技术亮点
- **OpenMP并行化**: 利用多核CPU进行图像处理的并行计算
- **C++/Python混合**: 高性能C++核心算法 + Python Web API
- **现代前端**: Vue 3 + TypeScript + Element Plus UI框架
- **跨平台**: 支持Linux、Windows、macOS

## 🏗️ 项目架构

```
HPC-Parallel-Demo/
├── backend/                 # Python Flask后端API
│   ├── app.py              # 主应用文件
│   ├── image_processing.cpython-*.so  # C++编译的Python扩展
│   ├── requirements.txt    # Python依赖
│   └── uploads/           # 上传文件存储
├── frontend/               # Vue.js前端应用
│   ├── src/               # 源代码
│   ├── package.json       # 前端依赖
│   └── vite.config.ts     # Vite配置
├── cpp/                   # C++并行算法实现
│   ├── image_processing.cpp  # 核心算法实现
│   └── CMakeLists.txt     # CMake构建配置
└── README.md              # 项目说明文档
```

## 🛠️ 技术栈

### 后端
- **Python 3.8+**: 主要开发语言
- **Flask**: Web框架
- **OpenCV**: 图像处理库
- **pybind11**: C++/Python绑定
- **OpenMP**: 并行计算框架

### 前端
- **Vue 3**: 前端框架
- **TypeScript**: 类型安全
- **Element Plus**: UI组件库
- **Vite**: 构建工具
- **Pinia**: 状态管理

### 并行计算
- **OpenMP**: 多线程并行编程
- **C++17**: 高性能算法实现
- **CMake**: 跨平台构建系统

## 📦 安装与运行

### 系统要求
- Python 3.8+
- Node.js 16+
- C++编译器 (GCC/Clang/MSVC)
- CMake 3.10+
- OpenMP支持

### 1. 克隆项目
```bash
git clone https://github.com/ymzhang-cs/HPC-Parallel-Demo.git
cd HPC-Parallel-Demo
```

### 2. 编译C++扩展
```bash
cd cpp
mkdir build && cd build
cmake ..
make
# 将生成的.so文件复制到backend目录
cp image_processing.cpython-*.so ../backend/
cd ../..
```

### 3. 安装后端依赖
```bash
cd backend
pip install -r requirements.txt
```

### 4. 安装前端依赖
```bash
cd frontend
pnpm install
```

### 5. 启动服务

#### 启动后端API
```bash
cd backend
python app.py
```
后端服务将在 `http://localhost:5000` 启动

#### 启动前端开发服务器
```bash
cd frontend
pnpm dev
```
前端应用将在 `http://localhost:5173` 启动

## 🎯 使用指南

### 图像处理功能

1. **灰度化转换**: 将彩色图像转换为灰度图像
2. **二值化处理**: 将图像转换为黑白二值图像
3. **亮度调整**: 调整图像亮度
4. **高斯模糊**: 应用高斯模糊滤镜
5. **边缘检测**: 使用Sobel算子进行边缘检测
6. **图像拼接**: 将两张图像拼接成全景图

### 性能对比

项目提供了并行版本和串行版本的性能对比功能，可以直观地看到OpenMP并行化带来的性能提升。

## 🔧 API接口

### 健康检查
```http
GET /api/health
```

### 图像变换
```http
POST /api/transform/{transform_type}
Content-Type: multipart/form-data

Parameters:
- transform_type: 变换类型 (grayscale, binary, brightness, gaussian_blur, sobel, stitch)
- image: 图像文件
- threshold: 二值化阈值 (可选)
- delta: 亮度调整值 (可选)
- kernel_size: 高斯核大小 (可选)
- sigma: 高斯标准差 (可选)
```

## 📊 性能特性

- **并行加速**: 利用OpenMP实现多核并行计算
- **内存优化**: 高效的内存管理和数据访问模式
- **算法优化**: 针对图像处理特点的算法优化
- **可扩展性**: 支持不同规模的图像处理任务

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 👨‍💻 作者

**张原鸣**

- GitHub: [@ymzhang-cs](https://github.com/ymzhang-cs)
- Email: hpc@ymzhangcs.com

**冉金禄**

- GitHub: [@JLRan-cs](https://github.com/JLRan-cs)
- Email: linr220908@gmail.com

## 🙏 致谢

- [OpenCV](https://opencv.org/) - 计算机视觉库
- [OpenMP](https://www.openmp.org/) - 并行编程框架
- [Vue.js](https://vuejs.org/) - 前端框架
- [Element Plus](https://element-plus.org/) - UI组件库

---

⭐ 如果这个项目对您有帮助，请给它一个星标！ 