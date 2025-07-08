<template>
    <div class="view-container">
        <h2 class="view-title">图像拼接</h2>
        <div class="view-content">
            <!-- 模块说明 -->
            <div class="module-description">
                <p>本模块提供图像拼接功能，可以将两张图片进行水平拼接，使用OpenCV实现。</p>
                <p>请先上传两张图片，然后点击拼接按钮进行处理。</p>
            </div>

            <el-row :gutter="20">
                <!-- 左侧：第一张图片上传和预览 -->
                <el-col :span="8">
                    <el-card class="preview-card">
                        <template #header>
                            <div class="card-header">
                                <span>第一张图片</span>
                                <el-button size="small" @click="loadSampleImage('image1')">示例图片</el-button>
                            </div>
                        </template>
                        <el-upload class="image-uploader" action="#" :auto-upload="false" :show-file-list="false"
                            :on-change="(file: any) => handleUpload(file, 'image1')" :before-upload="beforeUpload"
                            accept="image/*">
                            <img v-if="image1Url" :src="image1Url" class="preview-image" />
                            <el-icon v-else class="upload-icon">
                                <Plus />
                            </el-icon>
                        </el-upload>
                    </el-card>
                </el-col>

                <!-- 中间：第二张图片上传和预览 -->
                <el-col :span="8">
                    <el-card class="preview-card">
                        <template #header>
                            <div class="card-header">
                                <span>第二张图片</span>
                                <el-button size="small" @click="loadSampleImage('image2')">示例图片</el-button>
                            </div>
                        </template>
                        <el-upload class="image-uploader" action="#" :auto-upload="false" :show-file-list="false"
                            :on-change="(file: any) => handleUpload(file, 'image2')" :before-upload="beforeUpload"
                            accept="image/*">
                            <img v-if="image2Url" :src="image2Url" class="preview-image" />
                            <el-icon v-else class="upload-icon">
                                <Plus />
                            </el-icon>
                        </el-upload>
                    </el-card>
                </el-col>

                <!-- 右侧：拼接结果预览 -->
                <el-col :span="8">
                    <el-card class="preview-card">
                        <template #header>
                            <div class="card-header">
                                <span>拼接结果</span>
                            </div>
                        </template>
                        <div class="result-preview">
                            <img v-if="resultImageUrl" :src="resultImageUrl" class="preview-image" />
                            <div v-else class="empty-result">
                                <el-icon>
                                    <Picture />
                                </el-icon>
                                <span>请先上传两张图片</span>
                            </div>
                        </div>
                    </el-card>
                </el-col>
            </el-row>

            <!-- 拼接操作按钮 -->
            <div class="stitching-actions">
                <el-button type="primary" :loading="loading" @click="applyStitching" :disabled="!canStitch">
                    开始拼接
                </el-button>
                <el-button @click="clearImages">清空图片</el-button>
            </div>

            <!-- 拼接说明 -->
            <div class="stitching-info">
                <el-alert title="拼接说明" type="info" :closable="false" show-icon>
                    <template #default>
                        <p>• 两张图片将进行水平拼接</p>
                        <p>• 建议上传相同高度的图片以获得最佳效果</p>
                    </template>
                </el-alert>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import { ElMessage } from 'element-plus'
import { Plus, Picture } from '@element-plus/icons-vue'
import type { UploadProps, UploadFile } from 'element-plus'
import { buildApiUrl, buildFileUrl, API_ENDPOINTS } from '../config/api'
import sample2_1 from '../assets/sample2-1.bmp'
import sample2_2 from '../assets/sample2-2.bmp'

const image1Url = ref('')
const image2Url = ref('')
const resultImageUrl = ref('')
const loading = ref(false)

// 加载示例图片
const loadSampleImage = (imageType: 'image1' | 'image2') => {
    if (imageType === 'image1') {
        image1Url.value = sample2_1
        ElMessage.success('已加载第一张示例图片')
    } else if (imageType === 'image2') {
        image2Url.value = sample2_2
        ElMessage.success('已加载第二张示例图片')
    }
}

// 计算是否可以拼接
const canStitch = computed(() => {
    return image1Url.value && image2Url.value
})

// 处理图片上传
const handleUpload = (uploadFile: UploadFile, imageType: string) => {
    if (uploadFile.raw) {
        if (imageType === 'image1') {
            image1Url.value = URL.createObjectURL(uploadFile.raw)
        } else if (imageType === 'image2') {
            image2Url.value = URL.createObjectURL(uploadFile.raw)
        }
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

// 应用拼接
const applyStitching = async () => {
    if (!image1Url.value || !image2Url.value) {
        ElMessage.warning('请先上传两张图片！')
        return
    }

    loading.value = true
    try {
        // 从imageUrl获取文件对象
        const response1 = await fetch(image1Url.value)
        const blob1 = await response1.blob()

        const response2 = await fetch(image2Url.value)
        const blob2 = await response2.blob()

        // 创建FormData对象
        const formData = new FormData()
        formData.append('image1', blob1, 'image1.jpg')
        formData.append('image2', blob2, 'image2.jpg')

        // 调用后端API
        const apiResponse = await fetch(buildApiUrl(API_ENDPOINTS.transform.stitch), {
            method: 'POST',
            body: formData
        })

        if (!apiResponse.ok) {
            throw new Error(`HTTP error! status: ${apiResponse.status}`)
        }

        const result = await apiResponse.json()

        if (result.output_file) {
            // 下载处理后的文件
            const fileResponse = await fetch(buildFileUrl(result.output_file))
            if (fileResponse.ok) {
                const resultBlob = await fileResponse.blob()
                resultImageUrl.value = URL.createObjectURL(resultBlob)
                const time = result.parallel_time || result.cpp_time || result.processing_time || result.total_time
                ElMessage.success(`图像拼接成功！用时${time ? time.toFixed(8) : '?'}秒`)
            } else {
                throw new Error('下载处理结果失败')
            }
        } else {
            throw new Error('未获取到输出文件')
        }
    } catch (error) {
        console.error('API调用失败:', error)
        ElMessage.error('拼接失败，请重试！')
    } finally {
        loading.value = false
    }
}

// 清空图片
const clearImages = () => {
    image1Url.value = ''
    image2Url.value = ''
    resultImageUrl.value = ''
    ElMessage.info('已清空所有图片')
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
    height: 300px;
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
    max-height: 300px;
    object-fit: contain;
}

.result-preview {
    width: 100%;
    height: 300px;
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

.stitching-actions {
    display: flex;
    justify-content: center;
    gap: 20px;
    margin-top: 20px;
    margin-bottom: 20px;
}

.stitching-info {
    margin-top: 20px;
}

.stitching-info p {
    margin: 5px 0;
}
</style>
