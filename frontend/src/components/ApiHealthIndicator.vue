<template>
    <div class="api-health-indicator">
        <div class="health-status">
            <span class="health-dot"
                :class="{ 'healthy': isHealthy, 'unhealthy': !isHealthy, 'checking': healthText === '检查中...' }"></span>
            <span class="health-text">{{ healthText }}</span>
        </div>
        <div class="threads-info" v-if="isHealthy">
            <div class="threads-display">
                <span class="threads-label">当前线程数:</span>
                <span class="threads-value">{{ currentThreads }}</span>
            </div>
        </div>
        <div class="threads-control" v-if="isHealthy">
            <input type="number" v-model="newThreads" :min="1" :max="maxThreads" class="threads-input"
                placeholder="设置线程数" />
            <button @click="setThreads" :disabled="settingThreads || !newThreads || newThreads === currentThreads"
                class="threads-button">
                {{ settingThreads ? '设置中...' : '设置' }}
            </button>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { HealthService, OmpService } from '../services/apiService'

const isHealthy = ref(false)
const healthText = ref('检查中...')
const healthCheckInterval = ref<number | null>(null)

// 线程数相关状态
const currentThreads = ref(0)
const maxThreads = ref(0)
const newThreads = ref<number | null>(null)
const settingThreads = ref(false)

// 检查API健康状态
const checkHealth = async () => {
    try {
        const response = await HealthService.checkHealth()
        isHealthy.value = response.success || response.status === 200
        healthText.value = isHealthy.value ? 'API 服务正常 (200)' : `API 服务异常 (${response.status || 'API 服务未知'})`

        // 如果API健康，获取线程数信息
        if (isHealthy.value) {
            await getThreadsInfo()
        }
    } catch (error) {
        isHealthy.value = false
        healthText.value = 'API 服务连接失败'
        console.error('API 健康检查失败:', error)
    }
}

// 获取线程数信息
const getThreadsInfo = async () => {
    try {
        const response = await OmpService.getThreads()
        if (response.success && response.max_threads) {
            currentThreads.value = response.max_threads
            // 如果是第一次获取，设置maxThreads为当前线程数
            if (maxThreads.value === 0) {
                maxThreads.value = response.max_threads
            }
            // 如果没有设置过新线程数，使用当前值作为默认值
            if (newThreads.value === null) {
                newThreads.value = currentThreads.value
            }
        }
    } catch (error) {
        console.error('获取线程数失败:', error)
    }
}

// 设置线程数
const setThreads = async () => {
    if (!newThreads.value || newThreads.value === currentThreads.value) {
        return
    }

    settingThreads.value = true
    try {
        const response = await OmpService.setThreads(newThreads.value)
        if (response.success) {
            currentThreads.value = newThreads.value
            console.log('线程数设置成功:', newThreads.value)
        } else {
            console.error('设置线程数失败:', response.error)
        }
    } catch (error) {
        console.error('设置线程数失败:', error)
    } finally {
        settingThreads.value = false
    }
}

// 组件挂载时开始健康检查
onMounted(() => {
    // 立即检查一次
    checkHealth()

    // 每30秒检查一次
    healthCheckInterval.value = window.setInterval(checkHealth, 30000)
})

// 组件卸载时清理定时器
onUnmounted(() => {
    if (healthCheckInterval.value) {
        clearInterval(healthCheckInterval.value)
    }
})
</script>

<style scoped>
.api-health-indicator {
    padding: 12px 20px;
    border-top: 1px solid #e6e6e6;
    background-color: #fafafa;
    margin-top: auto;
}

.health-status {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 12px;
    color: #606266;
    margin-bottom: 8px;
}

.health-dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    display: inline-block;
}

.health-dot.healthy {
    background-color: #67c23a;
}

.health-dot.unhealthy {
    background-color: #f56c6c;
}

.health-dot.checking {
    background-color: #e6a23c;
    animation: pulse 1.5s infinite;
}

@keyframes pulse {
    0% {
        opacity: 1;
    }

    50% {
        opacity: 0.5;
    }

    100% {
        opacity: 1;
    }
}

.health-text {
    font-weight: 500;
}

.threads-info {
    display: flex;
    align-items: center;
    font-size: 12px;
    color: #606266;
    margin-bottom: 6px;
}

.threads-display {
    display: flex;
    align-items: center;
    gap: 4px;
}

.threads-label {
    color: #909399;
}

.threads-value {
    font-weight: 600;
    color: #409eff;
}

.threads-control {
    display: flex;
    align-items: center;
    gap: 6px;
    justify-content: flex-start;
}

.threads-input {
    width: 60px;
    height: 24px;
    border: 1px solid #dcdfe6;
    border-radius: 4px;
    padding: 0 6px;
    font-size: 12px;
    text-align: center;
    background-color: #fff;
}

.threads-input:focus {
    outline: none;
    border-color: #409eff;
}

.threads-button {
    height: 24px;
    padding: 0 8px;
    border: 1px solid #dcdfe6;
    border-radius: 4px;
    background-color: #fff;
    color: #606266;
    font-size: 12px;
    cursor: pointer;
    transition: all 0.3s;
}

.threads-button:hover:not(:disabled) {
    border-color: #409eff;
    color: #409eff;
}

.threads-button:disabled {
    background-color: #f5f7fa;
    color: #c0c4cc;
    cursor: not-allowed;
}
</style>