// API配置文件
// 集中管理所有API端点

// API基础URL
export const API_BASE_URL = 'http://localhost:5000'

// API端点配置
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
        stitch: '/api/transform/stitch',

        // 性能对比API
        grayscaleCompare: '/api/transform/grayscale/compare',
        binaryCompare: '/api/transform/binary/compare',
        brightnessCompare: '/api/transform/brightness/compare',
        gaussianBlurCompare: '/api/transform/gaussian_blur/compare',
        sobelCompare: '/api/transform/sobel/compare',
        convolutionCompare: '/api/transform/convolution/compare'
    },

    // 文件下载
    files: '/api/files'
}

// 构建完整的API URL
export const buildApiUrl = (endpoint: string): string => {
    return `${API_BASE_URL}${endpoint}`
}

// 构建文件下载URL
export const buildFileUrl = (filename: string): string => {
    return `${API_BASE_URL}${API_ENDPOINTS.files}/${filename}`
}

// API响应类型定义
export interface ApiResponse {
    success: boolean
    message?: string
    output_file?: string
    error?: string
    status?: number
    mode?: string
    parallel_time?: number
    serial_time?: number
    cpp_time?: number
    processing_time?: number
    total_time?: number
    // 性能对比相关字段
    parallel?: {
        time: number
        total_time: number
        output_file: string
    }
    serial?: {
        time: number
        total_time: number
        output_file: string
    }
    speedup?: number
    // OpenMP线程数相关字段
    max_threads?: number
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