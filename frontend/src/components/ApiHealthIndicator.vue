<template>
    <div class="api-health-indicator">
        <div class="health-status">
            <span class="health-dot"
                :class="{ 'healthy': isHealthy, 'unhealthy': !isHealthy, 'checking': healthText === '检查中...' }"></span>
            <span class="health-text">{{ healthText }}</span>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { HealthService } from '../services/apiService'

const isHealthy = ref(false)
const healthText = ref('检查中...')
const healthCheckInterval = ref<number | null>(null)

// 检查API健康状态
const checkHealth = async () => {
    try {
        const response = await HealthService.checkHealth()
        isHealthy.value = response.success || response.status === 200
        healthText.value = isHealthy.value ? 'API 服务正常 (200)' : `API 服务异常 (${response.status || 'API 服务未知'})`
    } catch (error) {
        isHealthy.value = false
        healthText.value = 'API 服务连接失败'
        console.error('API 健康检查失败:', error)
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
</style>