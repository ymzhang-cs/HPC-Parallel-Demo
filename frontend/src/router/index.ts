import { createRouter, createWebHistory } from 'vue-router'
import HomeView from '../views/HomeView.vue'
import BasicImageTransform from '../views/BasicImageTransform.vue'
import BasicConvolution from '../views/BasicConvolution.vue'
import CustomConvolution from '../views/CustomConvolution.vue'
import AboutView from '../views/AboutView.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'home',
      component: HomeView
    },
    {
      path: '/basic-image-transform',
      name: 'basic-image-transform',
      component: BasicImageTransform
    },
    {
      path: '/basic-convolution',
      name: 'basic-convolution',
      component: BasicConvolution
    },
    {
      path: '/custom-convolution',
      name: 'custom-convolution',
      component: CustomConvolution
    },
    {
      path: '/about',
      name: 'about',
      component: AboutView
    }
  ]
})

export default router
