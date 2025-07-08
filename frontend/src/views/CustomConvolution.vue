<template>
    <div class="view-container">
        <h2 class="view-title">自定义卷积操作</h2>
        <div class="view-content">
            <!-- 模块说明 -->
            <div class="module-description">
                <p>本模块提供自定义卷积核的卷积操作功能。</p>
                <p>请先上传图片，然后输入自定义卷积核矩阵，最后点击应用卷积按钮。</p>
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
                                <span>请先上传图片并设置卷积核</span>
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

            <!-- 卷积核设置面板 -->
            <div class="kernel-panel">
                <h3>卷积核设置</h3>

                <!-- 卷积核大小选择 -->
                <div class="kernel-size-selector">
                    <span class="parameter-label">卷积核大小：</span>
                    <span class="kernel-size-display">3×3</span>
                </div>

                <!-- 卷积核矩阵输入 -->
                <div class="kernel-matrix">
                    <span class="parameter-label">卷积核矩阵：</span>
                    <div class="matrix-input">
                        <div v-for="(row, rowIndex) in kernelMatrix" :key="rowIndex" class="matrix-row">
                            <el-input-number v-for="(value, colIndex) in row" :key="colIndex"
                                v-model="kernelMatrix[rowIndex][colIndex]" :min="-10" :max="10" :step="0.1" size="small"
                                class="matrix-cell" />
                        </div>
                    </div>
                </div>

                <!-- 缩放因子 -->
                <div class="scale-factor">
                    <span class="parameter-label">缩放因子：</span>
                    <el-input-number v-model="scaleFactor" :min="0.1" :max="10" :step="0.1" />
                </div>

                <!-- 预设卷积核 -->
                <div class="preset-kernels">
                    <span class="parameter-label">预设卷积核：</span>
                    <el-button-group>
                        <el-button size="small" @click="applyPresetKernel('identity')">单位矩阵</el-button>
                        <el-button size="small" @click="applyPresetKernel('sharpen')">锐化</el-button>
                        <el-button size="small" @click="applyPresetKernel('emboss')">浮雕</el-button>
                        <el-button size="small" @click="applyPresetKernel('edge')">边缘检测</el-button>
                    </el-button-group>
                </div>

                <!-- 操作按钮 -->
                <div class="convolution-actions">
                    <el-button type="primary" :loading="loading" @click="applyCustomConvolution">
                        应用卷积
                    </el-button>
                    <el-button @click="resetKernel">重置卷积核</el-button>
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
import { ref, reactive } from 'vue'
import { ElMessage } from 'element-plus'
import { Plus, Picture } from '@element-plus/icons-vue'
import type { UploadProps, UploadFile } from 'element-plus'
import { buildApiUrl, buildFileUrl, API_ENDPOINTS } from '../config/api'
import sample1 from '../assets/sample1.jpg'

const imageUrl = ref('')
const resultImageUrl = ref('')
const loading = ref(false)
const kernelSize = ref(3) // 固定为3×3
const scaleFactor = ref(1.0)
const processingMode = ref('parallel')
const comparisonResult = ref<any>(null)
const kernelMatrix = reactive<number[][]>([
    [0, 0, 0],
    [0, 1, 0],
    [0, 0, 0]
])

// 加载示例图片
const loadSampleImage = () => {
    imageUrl.value = sample1
    ElMessage.success('已加载示例图片')
}

// 预设卷积核
const presetKernels = {
    identity: [[0, 0, 0], [0, 1, 0], [0, 0, 0]],
    sharpen: [[0, -1, 0], [-1, 5, -1], [0, -1, 0]],
    emboss: [[-2, -1, 0], [-1, 1, 1], [0, 1, 2]],
    edge: [[-1, -1, -1], [-1, 8, -1], [-1, -1, -1]]
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
    comparisonResult.value = null
}

// 更新卷积核矩阵
const updateKernelMatrix = () => {
    const newMatrix: number[][] = []
    for (let i = 0; i < 3; i++) {
        newMatrix[i] = []
        for (let j = 0; j < 3; j++) {
            if (i === 1 && j === 1) {
                newMatrix[i][j] = 1
            } else {
                newMatrix[i][j] = 0
            }
        }
    }

    // 清空现有矩阵并重新赋值
    kernelMatrix.length = 0
    newMatrix.forEach(row => kernelMatrix.push([...row]))
}

// 应用预设卷积核
const applyPresetKernel = (preset: string) => {
    const presetMatrix = presetKernels[preset as keyof typeof presetKernels]
    if (presetMatrix) {
        kernelMatrix.length = 0
        presetMatrix.forEach(row => kernelMatrix.push([...row]))
        ElMessage.success(`已应用${preset}卷积核`)
    }
}

// 重置卷积核
const resetKernel = () => {
    updateKernelMatrix()
    scaleFactor.value = 1.0
    ElMessage.info('卷积核已重置')
}

// 应用自定义卷积
const applyCustomConvolution = async () => {
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
        formData.append('kernel', JSON.stringify(kernelMatrix))
        formData.append('scale', scaleFactor.value.toString())

        let apiResponse: Response
        let result: any

        if (processingMode.value === 'compare') {
            // 性能对比模式
            formData.append('mode', 'compare')
            apiResponse = await fetch(buildApiUrl(`${API_ENDPOINTS.transform.convolution}/compare`), {
                method: 'POST',
                body: formData
            })
        } else {
            // 单模式处理
            formData.append('mode', processingMode.value)
            apiResponse = await fetch(buildApiUrl(API_ENDPOINTS.transform.convolution), {
                method: 'POST',
                body: formData
            })
        }

        if (!apiResponse.ok) {
            throw new Error(`HTTP error! status: ${apiResponse.status}`)
        }

        result = await apiResponse.json()

        if (processingMode.value === 'compare') {
            // 性能对比模式
            comparisonResult.value = result

            // 显示并行版本的结果
            if (result.parallel?.output_file) {
                const fileResponse = await fetch(buildFileUrl(result.parallel.output_file))
                if (fileResponse.ok) {
                    const resultBlob = await fileResponse.blob()
                    resultImageUrl.value = URL.createObjectURL(resultBlob)
                }
            }

            const speedup = result.speedup || 0
            ElMessage.success(`性能对比完成！加速比: ${speedup.toFixed(2)}x`)
        } else {
            // 单模式处理
            if (result.output_file) {
                // 下载处理后的文件
                const fileResponse = await fetch(buildFileUrl(result.output_file))
                if (fileResponse.ok) {
                    const resultBlob = await fileResponse.blob()
                    resultImageUrl.value = URL.createObjectURL(resultBlob)
                    const time = result.parallel_time || result.serial_time || result.cpp_time || result.processing_time || result.total_time
                    ElMessage.success(`${result.message}！用时${time ? time.toFixed(8) : '?'}秒`)
                } else {
                    throw new Error('下载处理结果失败')
                }
            } else {
                throw new Error('未获取到输出文件')
            }
        }
    } catch (error) {
        console.error('API调用失败:', error)
        ElMessage.error('处理失败，请重试！')
    } finally {
        loading.value = false
    }
}

// 初始化
updateKernelMatrix()
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

.kernel-panel {
    margin-top: 20px;
    padding: 20px;
    border: 1px solid #dcdfe6;
    border-radius: 4px;
    background-color: #f5f7fa;
}

.kernel-panel h3 {
    margin-top: 0;
    margin-bottom: 20px;
    color: #303133;
}

.kernel-size-selector {
    margin-bottom: 20px;
    display: flex;
    align-items: center;
}

.kernel-size-display {
    color: #409EFF;
    font-weight: bold;
    font-size: 16px;
}

.kernel-matrix,
.kernel-matrix,
.scale-factor,
.preset-kernels {
    margin-bottom: 20px;
    display: flex;
    align-items: center;
}

.parameter-label {
    width: 100px;
    text-align: right;
    margin-right: 15px;
    font-weight: bold;
}

.matrix-input {
    display: flex;
    flex-direction: column;
    gap: 5px;
}

.matrix-row {
    display: flex;
    gap: 5px;
}

.matrix-cell {
    width: 80px;
}

.convolution-actions {
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