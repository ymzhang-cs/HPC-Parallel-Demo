<template>
    <div class="view-container">
        <h2 class="view-title">图像基础变换</h2>
        <div class="view-content">
            <!-- 模块说明 -->
            <div class="module-description">
                <p>本模块提供基本的图像变换功能，包括灰度化、二值化、亮度调整等。</p>
                <p>请先上传图片，然后选择变换方式，并设置相关参数，最后点击应用变换按钮。</p>
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

                <!-- 右侧：变换结果预览 -->
                <el-col :span="12">
                    <el-card class="preview-card">
                        <template #header>
                            <div class="card-header">
                                <span>变换结果</span>
                            </div>
                        </template>
                        <div class="result-preview">
                            <img v-if="resultImageUrl" :src="resultImageUrl" class="preview-image" />
                            <div v-else class="empty-result">
                                <el-icon>
                                    <Picture />
                                </el-icon>
                                <span>请先上传图片并选择变换方式</span>
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

            <!-- 变换操作按钮 -->
            <div class="transform-actions">
                <el-button type="primary" :loading="loading" @click="handleTransform('grayscale')">
                    灰度化
                </el-button>
                <el-button type="primary" :loading="loading" @click="handleTransform('binary')">
                    二值化
                </el-button>
                <el-button type="primary" :loading="loading" @click="handleTransform('brightness')">
                    亮度调整
                </el-button>
            </div>

            <!-- 参数控制面板 -->
            <div class="parameter-panel" v-if="currentTransform">
                <template v-if="currentTransform === 'binary'">
                    <div class="parameter-item">
                        <span class="parameter-label">阈值：</span>
                        <el-slider v-model="binaryThreshold" :min="0" :max="255" :step="1" />
                    </div>
                </template>
                <template v-if="currentTransform === 'brightness'">
                    <div class="parameter-item">
                        <span class="parameter-label">亮度值：</span>
                        <el-slider v-model="brightnessValue" :min="-100" :max="100" :step="1" />
                    </div>
                </template>
                <div class="parameter-actions">
                    <el-button type="primary" @click="applyTransform">应用变换</el-button>
                    <el-button @click="cancelTransform">取消</el-button>
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
const currentTransform = ref('')
const binaryThreshold = ref(128)
const brightnessValue = ref(0)
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
    currentTransform.value = ''
    comparisonResult.value = null
}

// 处理图片变换
const handleTransform = async (type: string) => {
    if (!imageUrl.value) {
        ElMessage.warning('请先上传图片！')
        return
    }
    currentTransform.value = type
}

// 应用变换
const applyTransform = async () => {
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
        if (currentTransform.value === 'binary') {
            formData.append('threshold', binaryThreshold.value.toString())
        } else if (currentTransform.value === 'brightness') {
            formData.append('adjustment', brightnessValue.value.toString())
        }

        let apiResponse: any

        if (processingMode.value === 'compare') {
            // 性能对比模式
            formData.append('mode', 'compare')
            apiResponse = await ImageProcessingService.processImage(`${currentTransform.value}/compare`, formData)
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
            apiResponse = await ImageProcessingService.processImage(currentTransform.value, formData)
            if (!apiResponse.output_file) throw new Error('未获取到输出文件')

            // 下载处理后的文件
            const resultBlob = await ImageProcessingService.downloadFile(apiResponse.output_file)
            resultImageUrl.value = URL.createObjectURL(resultBlob)

            const time = apiResponse.parallel_time || apiResponse.serial_time || apiResponse.cpp_time || apiResponse.processing_time || apiResponse.total_time
            ElMessage.success(`${apiResponse.message}！用时${time ? time.toFixed(8) : '?'}秒`)
        }

        currentTransform.value = ''
    } catch (error) {
        console.error('API调用失败:', error)
        ElMessage.error('处理失败，请重试！')
    } finally {
        loading.value = false
    }
}

// 取消变换
const cancelTransform = () => {
    currentTransform.value = ''
    binaryThreshold.value = 128
    brightnessValue.value = 0
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

.transform-actions {
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

.parameter-actions {
    display: flex;
    justify-content: center;
    gap: 20px;
    margin-top: 20px;
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