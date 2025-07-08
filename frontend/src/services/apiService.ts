// API服务文件
// 封装常用的API调用方法

import { buildApiUrl, buildFileUrl, API_ENDPOINTS, type ApiResponse } from '../config/api'

// 图像处理服务
export class ImageProcessingService {
    /**
 * 处理图像变换
 * @param endpoint 变换端点
 * @param formData 表单数据
 * @returns Promise<ApiResponse>
 */
    static async processImage(endpoint: string, formData: FormData): Promise<ApiResponse> {
        // 端点映射
        const endpointMap: Record<string, string> = {
            'gaussian_blur': API_ENDPOINTS.transform.gaussianBlur,
            'grayscale': API_ENDPOINTS.transform.grayscale,
            'binary': API_ENDPOINTS.transform.binary,
            'brightness': API_ENDPOINTS.transform.brightness,
            'sobel': API_ENDPOINTS.transform.sobel,
            'convolution': API_ENDPOINTS.transform.convolution,
            'stitch': API_ENDPOINTS.transform.stitch,
            // 性能对比端点
            'grayscale/compare': API_ENDPOINTS.transform.grayscaleCompare,
            'binary/compare': API_ENDPOINTS.transform.binaryCompare,
            'brightness/compare': API_ENDPOINTS.transform.brightnessCompare,
            'gaussian_blur/compare': API_ENDPOINTS.transform.gaussianBlurCompare,
            'sobel/compare': API_ENDPOINTS.transform.sobelCompare,
            'convolution/compare': API_ENDPOINTS.transform.convolutionCompare
        }

        const apiEndpoint = endpointMap[endpoint] || API_ENDPOINTS.transform[endpoint as keyof typeof API_ENDPOINTS.transform]

        const response = await fetch(buildApiUrl(apiEndpoint), {
            method: 'POST',
            body: formData
        })

        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`)
        }

        return await response.json()
    }

    /**
     * 下载处理后的文件
     * @param filename 文件名
     * @returns Promise<Blob>
     */
    static async downloadFile(filename: string): Promise<Blob> {
        const response = await fetch(buildFileUrl(filename))

        if (!response.ok) {
            throw new Error('下载文件失败')
        }

        return await response.blob()
    }

    /**
     * 处理图像并下载结果
     * @param endpoint 变换端点
     * @param formData 表单数据
     * @returns Promise<Blob>
     */
    static async processAndDownload(endpoint: string, formData: FormData): Promise<Blob> {
        const result = await this.processImage(endpoint, formData)

        if (!result.output_file) {
            throw new Error('未获取到输出文件')
        }

        return await this.downloadFile(result.output_file)
    }
}

// 健康检查服务
export class HealthService {
    /**
 * 检查API健康状态
 * @returns Promise<ApiResponse>
 */
    static async checkHealth(): Promise<ApiResponse> {
        const response = await fetch(buildApiUrl(API_ENDPOINTS.health))

        const result = await response.json()

        return {
            success: response.ok,
            status: response.status,
            ...result
        }
    }
}

// OpenMP服务
export class OmpService {
    /**
     * 获取当前线程数
     * @returns Promise<ApiResponse>
     */
    static async getThreads(): Promise<ApiResponse> {
        const response = await fetch(buildApiUrl(API_ENDPOINTS.omp.threads))

        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`)
        }

        return await response.json()
    }

    /**
     * 设置线程数
     * @param threads 线程数
     * @returns Promise<ApiResponse>
     */
    static async setThreads(threads: number): Promise<ApiResponse> {
        const response = await fetch(buildApiUrl(API_ENDPOINTS.omp.threads), {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ threads })
        })

        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`)
        }

        return await response.json()
    }
} 