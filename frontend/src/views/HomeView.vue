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
                该系统为张原鸣、冉金禄的《高性能计算导论》课程设计，主要用来展示并行计算在图像处理中的应用，目前代码以 MIT 协议开源至 GitHub。
            </p>
            <el-divider />
            <p>
                系统主要分为以下几个部分：
            </p>
            <el-container style="margin: 20px 0;">
                <el-row :gutter="20">
                    <el-col :span="8">
                        <el-card class="section-card" @click="navigateTo('/basic-image-transform')">
                            <h3>图像基础变换</h3>
                            <p>
                                图像基础变换包括图像的灰度化、二值化、亮度调整等。
                            </p>
                        </el-card>
                    </el-col>
                    <el-col :span="8">
                        <el-card class="section-card" @click="navigateTo('/basic-convolution')">
                            <h3>基本卷积操作</h3>
                            <p>
                                基本卷积操作包括高斯模糊、边缘检测等。
                            </p>
                        </el-card>
                    </el-col>
                    <el-col :span="8">
                        <el-card class="section-card" @click="navigateTo('/custom-convolution')">
                            <h3>自定义卷积操作</h3>
                            <p>
                                自定义卷积操作包括自定义卷积核的生成、自定义卷积操作的实现等。
                            </p>
                        </el-card>
                    </el-col>
                </el-row>
            </el-container>
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
</style>