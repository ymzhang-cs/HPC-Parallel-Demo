<script setup lang="ts">
import { useRouter } from 'vue-router'

const router = useRouter()

const navigateTo = (path: string) => {
    router.push(path)
}
</script>

<template>
    <div class="view-container">
        <h2 class="view-title">首页</h2>
        <div class="view-content">
            <p>
                OMPic 是张原鸣、冉金禄的《高性能计算导论》课程设计，主要用来展示并行计算在图像处理中的应用，目前代码以 MIT 协议开源至 GitHub。
            </p>
            <el-divider />
            <p>
                系统主要分为以下几个部分：
            </p>
            <el-container style="margin: 20px 0;">
                <el-row :gutter="20">
                    <el-col :span="6">
                        <el-card class="section-card" @click="navigateTo('/basic-image-transform')">
                            <h3>图像基础变换</h3>
                            <p>
                                图像基础变换包括图像的灰度化、二值化、亮度调整等基本操作。
                            </p>
                        </el-card>
                    </el-col>
                    <el-col :span="6">
                        <el-card class="section-card" @click="navigateTo('/basic-convolution')">
                            <h3>基本卷积操作</h3>
                            <p>
                                基本卷积操作包括高斯模糊、Sobel边缘检测等经典卷积操作。
                            </p>
                        </el-card>
                    </el-col>
                    <el-col :span="6">
                        <el-card class="section-card" @click="navigateTo('/custom-convolution')">
                            <h3>自定义卷积操作</h3>
                            <p>
                                自定义卷积操作支持用户自定义卷积核矩阵，实现各种图像滤波效果。
                            </p>
                        </el-card>
                    </el-col>
                    <el-col :span="6">
                        <el-card class="section-card" @click="navigateTo('/image-stitching')">
                            <h3>图像拼接</h3>
                            <p>
                                图像拼接功能可以将两张图片进行水平拼接，生成更宽的图像。
                            </p>
                        </el-card>
                    </el-col>
                </el-row>
            </el-container>

            <!-- 技术特点 -->
            <el-divider />
            <h3>技术特点</h3>
            <el-row :gutter="20">
                <el-col :span="8">
                    <el-card class="feature-card">
                        <h4>并行计算</h4>
                        <p>使用OpenMP并行计算技术，充分利用多核CPU资源，大幅提升图像处理速度。</p>
                    </el-card>
                </el-col>
                <el-col :span="8">
                    <el-card class="feature-card">
                        <h4>实时预览</h4>
                        <p>支持实时预览处理效果，用户可以直观地看到参数调整对图像的影响。</p>
                    </el-card>
                </el-col>
                <el-col :span="8">
                    <el-card class="feature-card">
                        <h4>易于使用</h4>
                        <p>简洁直观的用户界面，上传图片方便简单，参数调节简单方便。</p>
                    </el-card>
                </el-col>
            </el-row>
        </div>
    </div>
</template>

<style scoped>
.section-card {
    height: 100%;
    border-radius: 10px;
    padding: 5px;
    cursor: pointer;
    transition: all 0.3s ease;
}

.section-card:hover {
    transform: translateY(-5px);
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
}

.section-card h3 {
    color: #409EFF;
    margin-bottom: 10px;
}

.section-card p {
    color: #666;
    font-size: 14px;
    line-height: 1.6;
}

.feature-card {
    border-radius: 10px;
    padding: 5px;
    margin-bottom: 20px;
}

.feature-card h4 {
    color: #67C23A;
    margin-bottom: 10px;
}

.feature-card p {
    color: #666;
    font-size: 14px;
    line-height: 1.6;
}
</style>