<template>
    <div class="view-container">
        <h2 class="view-title">基本卷积操作</h2>
        <div class="view-content">
            <!-- 模块说明 -->
            <div class="module-description">
                <p>本模块提供基本的卷积操作功能，包括高斯模糊、Sobel边缘检测等。</p>
                <p>请先上传图片，然后选择卷积操作类型，并设置相关参数，最后点击应用操作按钮。</p>
            </div>

            <el-row :gutter="20">
                <!-- 左侧：图片上传和预览 -->
                <el-col :span="12">
                    <el-card class="preview-card">
                        <template #header>
                            <div class="card-header">
                                <span>原始图片</span>
                                <el-button size="small" @click="loadSampleImage">示例图片</el-button>
                            </div>
                        </template>
                        <el-upload class="image-uploader" action="#" :auto-upload="false" :show-file-list="false"
                            :on-change="handleUpload" :before-upload="beforeUpload" accept="image/*">
                            <img v-if="imageUrl" :src="imageUrl" class="preview-image" />
                            <el-icon v-else class="upload-icon">
                                <Plus />
                            </el-icon>
                        </el-upload>
                    </el-card>
                </el-col>

                <!-- 右侧：处理结果预览 -->
                <el-col :span="12">
                    <el-card class="preview-card">
                        <template #header>
                            <div class="card-header">
                                <span>处理结果</span>
                            </div>
                        </template>
                        <div class="result-preview">
                            <img v-if="resultImageUrl" :src="resultImageUrl" class="preview-image" />
                            <div v-else class="empty-result">
                                <el-icon>
                                    <Picture />
                                </el-icon>
                                <span>请先上传图片并选择卷积操作</span>
                            </div>
                        </div>
                    </el-card>
                </el-col>
            </el-row>

            <!-- 处理模式选择 -->
            <div class="mode-selection">
                <el-radio-group v-model="processingMode" @change="handleModeChange">
                    <el-radio label="parallel">并行模式</el-radio>
                    <el-radio label="serial">串行模式</el-radio>
                    <el-radio label="compare">性能对比</el-radio>
                </el-radio-group>
            </div>

            <!-- 卷积操作按钮 -->
            <div class="convolution-actions">
                <el-button type="primary" :loading="loading" @click="handleConvolution('gaussian_blur')">
                    高斯模糊
                </el-button>
                <el-button type="primary" :loading="loading" @click="handleConvolution('sobel')">
                    Sobel边缘检测
                </el-button>
            </div>

            <!-- 参数控制面板 -->
            <div class="parameter-panel" v-if="currentConvolution">
                <template v-if="currentConvolution === 'gaussian_blur'">
                    <div class="parameter-item">
                        <span class="parameter-label">标准差：</span>
                        <el-slider v-model="gaussianSigma" :min="0.1" :max="5.0" :step="0.1" />
                        <span class="parameter-value">{{ gaussianSigma }}</span>
                    </div>
                    <div class="parameter-info">
                        <el-alert title="高斯模糊说明" type="info" :closable="false" show-icon>
                            <template #default>
                                <p>• 使用固定的5×5卷积核</p>
                                <p>• 标准差越大，模糊效果越明显</p>
                                <p>• 建议值范围：0.1 - 5.0</p>
                            </template>
                        </el-alert>
                    </div>
                </template>
                <div class="parameter-actions">
                    <el-button type="primary" @click="applyConvolution">应用卷积</el-button>
                    <el-button @click="cancelConvolution">取消</el-button>
                </div>
            </div>

            <!-- 性能对比结果 -->
            <div class="comparison-result" v-if="comparisonResult">
                <el-card>
                    <template #header>
                        <div class="card-header">
                            <span>性能对比结果</span>
                        </div>
                    </template>
                    <div class="comparison-content">
                        <div class="comparison-item">
                            <h4>并行版本</h4>
                            <p>执行时间: {{ comparisonResult.parallel?.time?.toFixed(6) || 'N/A' }} 秒</p>
                            <p>总时间: {{ comparisonResult.parallel?.total_time?.toFixed(6) || 'N/A' }} 秒</p>
                        </div>
                        <div class="comparison-item">
                            <h4>串行版本</h4>
                            <p>执行时间: {{ comparisonResult.serial?.time?.toFixed(6) || 'N/A' }} 秒</p>
                            <p>总时间: {{ comparisonResult.serial?.total_time?.toFixed(6) || 'N/A' }} 秒</p>
                        </div>
                        <div class="comparison-item">
                            <h4>加速比</h4>
                            <p class="speedup">{{ (comparisonResult.speedup || 0).toFixed(2) }}x</p>
                        </div>
                    </div>
                </el-card>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import { ElMessage } from 'element-plus'
import { Plus, Picture } from '@element-plus/icons-vue'
import type { UploadProps, UploadFile } from 'element-plus'
import { ImageProcessingService } from '../services/apiService'
import sample1 from '../assets/sample1.jpg'

const imageUrl = ref('')
const resultImageUrl = ref('')
const loading = ref(false)
const currentConvolution = ref('')
const gaussianSigma = ref(1.0)
const processingMode = ref('parallel')
const comparisonResult = ref<any>(null)

// 加载示例图片
const loadSampleImage = () => {
    imageUrl.value = sample1
    ElMessage.success('已加载示例图片')
}

// 处理图片上传
const handleUpload: UploadProps['onChange'] = (uploadFile: UploadFile) => {
    if (uploadFile.raw) {
        imageUrl.value = URL.createObjectURL(uploadFile.raw)
    }
}

// 处理图片上传前的验证
const beforeUpload: UploadProps['beforeUpload'] = (file) => {
    const isImage = file.type.startsWith('image/')
    const isLt2M = file.size / 1024 / 1024 < 2

    if (!isImage) {
        ElMessage.error('只能上传图片文件！')
        return false
    }
    if (!isLt2M) {
        ElMessage.error('图片大小不能超过 2MB！')
        return false
    }
    return true
}

// 处理模式变化
const handleModeChange = () => {
    currentConvolution.value = ''
    comparisonResult.value = null
}

// 处理卷积操作
const handleConvolution = async (type: string) => {
    if (!imageUrl.value) {
        ElMessage.warning('请先上传图片！')
        return
    }
    currentConvolution.value = type
}

// 应用卷积
const applyConvolution = async () => {
    if (!imageUrl.value) {
        ElMessage.warning('请先上传图片！')
        return
    }

    loading.value = true
    try {
        // 从imageUrl获取文件对象
        const response = await fetch(imageUrl.value)
        const blob = await response.blob()

        // 创建FormData对象
        const formData = new FormData()
        formData.append('image', blob, 'image.jpg')

        // 添加参数
        if (currentConvolution.value === 'gaussian_blur') {
            formData.append('sigma', gaussianSigma.value.toString())
        }

        let apiResponse: any

        if (processingMode.value === 'compare') {
            // 性能对比模式
            formData.append('mode', 'compare')
            apiResponse = await ImageProcessingService.processImage(`${currentConvolution.value}/compare`, formData)
            comparisonResult.value = apiResponse

            // 显示并行版本的结果
            if (apiResponse.parallel?.output_file) {
                const resultBlob = await ImageProcessingService.downloadFile(apiResponse.parallel.output_file)
                resultImageUrl.value = URL.createObjectURL(resultBlob)
            }

            const speedup = apiResponse.speedup || 0
            ElMessage.success(`性能对比完成！加速比: ${speedup.toFixed(2)}x`)
        } else {
            // 单模式处理
            formData.append('mode', processingMode.value)
            apiResponse = await ImageProcessingService.processImage(currentConvolution.value, formData)
            if (!apiResponse.output_file) throw new Error('未获取到输出文件')

            // 下载处理后的文件
            const resultBlob = await ImageProcessingService.downloadFile(apiResponse.output_file)
            resultImageUrl.value = URL.createObjectURL(resultBlob)

            const time = apiResponse.parallel_time || apiResponse.serial_time || apiResponse.cpp_time || apiResponse.processing_time || apiResponse.total_time
            ElMessage.success(`${apiResponse.message}！用时${time ? time.toFixed(8) : '?'}秒`)
        }

        currentConvolution.value = ''
    } catch (error) {
        console.error('API调用失败:', error)
        ElMessage.error('处理失败，请重试！')
    } finally {
        loading.value = false
    }
}

// 取消卷积
const cancelConvolution = () => {
    currentConvolution.value = ''
    gaussianSigma.value = 1.0
}
</script>

<style scoped>
.module-description {
    margin-bottom: 20px;
}

.preview-card {
    margin-bottom: 20px;
}

.card-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
}

.image-uploader {
    width: 100%;
    height: 350px;
    border: 1px dashed #d9d9d9;
    border-radius: 6px;
    cursor: pointer;
    position: relative;
    overflow: hidden;
    display: flex;
    justify-content: center;
    align-items: center;
}

.image-uploader :deep(.el-upload) {
    width: 100%;
    height: 100%;
    display: flex;
    justify-content: center;
    align-items: center;
}

.image-uploader:hover {
    border-color: #409EFF;
}

.upload-icon {
    font-size: 28px;
    color: #8c939d;
}

.preview-image {
    width: 100%;
    height: 100%;
    max-height: 350px;
    object-fit: contain;
}

.result-preview {
    width: 100%;
    height: 350px;
    border: 1px dashed #d9d9d9;
    border-radius: 6px;
    display: flex;
    justify-content: center;
    align-items: center;
    overflow: hidden;
}

.empty-result {
    display: flex;
    flex-direction: column;
    align-items: center;
    color: #909399;
}

.empty-result .el-icon {
    font-size: 48px;
    margin-bottom: 10px;
}

.convolution-actions {
    display: flex;
    justify-content: center;
    gap: 20px;
    margin-top: 20px;
}

.parameter-panel {
    margin-top: 20px;
    padding: 20px;
    border: 1px solid #dcdfe6;
    border-radius: 4px;
    background-color: #f5f7fa;
}

.parameter-item {
    display: flex;
    align-items: center;
    margin-bottom: 15px;
}

.parameter-label {
    width: 80px;
    text-align: right;
    margin-right: 15px;
}

.parameter-value {
    width: 60px;
    text-align: center;
    margin-left: 15px;
    color: #409EFF;
    font-weight: bold;
}

.parameter-actions {
    display: flex;
    justify-content: center;
    gap: 20px;
    margin-top: 20px;
}

.parameter-info {
    margin-top: 15px;
}

.parameter-info .el-alert {
    margin-bottom: 0;
}

.parameter-info .el-alert__content p {
    margin: 5px 0;
    font-size: 13px;
}

.mode-selection {
    margin: 20px 0;
    text-align: center;
}

.comparison-result {
    margin-top: 20px;
}

.comparison-content {
    display: flex;
    justify-content: space-around;
    align-items: flex-start;
    gap: 20px;
}

.comparison-item {
    text-align: center;
    flex: 1;
}

.comparison-item h4 {
    margin-bottom: 10px;
    color: #409EFF;
}

.comparison-item p {
    margin: 5px 0;
    font-size: 14px;
}

.speedup {
    font-size: 18px !important;
    font-weight: bold;
    color: #67C23A;
}
</style>