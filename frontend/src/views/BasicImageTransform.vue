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
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import { ElMessage } from 'element-plus'
import type { UploadProps, UploadFile } from 'element-plus'

const imageUrl = ref('')
const resultImageUrl = ref('')
const loading = ref(false)
const currentTransform = ref('')
const binaryThreshold = ref(128)
const brightnessValue = ref(0)

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
        // TODO: 调用后端API，传递相应参数
        // const response = await fetch(`/api/transform/${currentTransform.value}`, {
        //     method: 'POST',
        //     body: JSON.stringify({
        //         image: imageUrl.value,
        //         parameters: {
        //             threshold: binaryThreshold.value,
        //             brightness: brightnessValue.value
        //         }
        //     })
        // })
        // const data = await response.json()
        // resultImageUrl.value = data.result

        // 临时模拟API响应
        await new Promise(resolve => setTimeout(resolve, 1000))
        resultImageUrl.value = imageUrl.value
        currentTransform.value = ''
    } catch (error) {
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
</style>