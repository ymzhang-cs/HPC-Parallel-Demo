<script setup lang="ts">
import { RouterView } from 'vue-router'
import { HomeFilled, Picture, InfoFilled } from '@element-plus/icons-vue'
import githubIcon from './assets/github-mark-white.svg'
</script>

<template>
  <el-container class="common-layout">
    <el-header>
      <h2>HPC 并行计算演示系统</h2>
      <div class="header-right">
        <a href="https://github.com/ymzhang-cs/HPC-Parallel-Demo" target="_blank" class="github-link">
          <img :src="githubIcon" alt="GitHub" class="github-icon" />
        </a>
      </div>
    </el-header>
    <el-container>
      <el-aside width="200px">
        <el-menu router :default-active="$route.path" class="el-menu-vertical">
          <el-menu-item index="/">
            <el-icon>
              <HomeFilled />
            </el-icon>
            <span>首页</span>
          </el-menu-item>
          <el-menu-item index="/basic-image-transform">
            <el-icon>
              <Picture />
            </el-icon>
            <span>图像基础变换</span>
          </el-menu-item>
          <el-menu-item index="/basic-convolution">
            <el-icon>
              <Picture />
            </el-icon>
            <span>基本卷积操作</span>
          </el-menu-item>
          <el-menu-item index="/custom-convolution">
            <el-icon>
              <Picture />
            </el-icon>
            <span>自定义卷积操作</span>
          </el-menu-item>
          <el-menu-item index="/about">
            <el-icon>
              <InfoFilled />
            </el-icon>
            <span>关于</span>
          </el-menu-item>
        </el-menu>
      </el-aside>
      <el-main>
        <router-view v-slot="{ Component }">
          <transition name="fade-slide" mode="out-in">
            <component :is="Component" />
          </transition>
        </router-view>
      </el-main>
    </el-container>
  </el-container>
</template>

<style scoped>
.common-layout {
  height: 100vh;
  margin: 0;
  padding: 0;
}

.el-header {
  background-color: #409EFF;
  color: white;
  display: flex;
  align-items: center;
  padding: 0 20px;
}

.el-aside {
  background-color: #f5f7fa;
  border-right: solid 1px #e6e6e6;
}

.el-menu-vertical {
  border-right: none;
}

.el-main {
  padding: 20px;
  overflow-x: hidden;
}

/* 页面过渡动画 */
.fade-slide-enter-active,
.fade-slide-leave-active {
  transition: all 0.2s ease;
  position: absolute;
  width: 100%;
}

.fade-slide-enter-from {
  opacity: 0;
  transform: translateX(30px);
}

.fade-slide-leave-to {
  opacity: 0;
  transform: translateX(-30px);
}

/* 防止滚动条闪现 */
.el-container {
  overflow: hidden;
}

.el-main {
  position: relative;
}

.header-right {
  margin-left: auto;
}

.github-link {
  color: white;
  text-decoration: none;
  display: flex;
  align-items: center;
}

.github-link:hover {
  opacity: 0.8;
}

.github-icon {
  width: 24px;
  height: 24px;
}
</style>
