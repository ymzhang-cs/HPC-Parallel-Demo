#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
API测试脚本
用于测试图像处理API的各种功能
"""

import requests
import json
import os
import time

# API基础URL
BASE_URL = "http://localhost:5000"

def test_health_check():
    """测试健康检查API"""
    print("=== 测试健康检查 ===")
    try:
        response = requests.get(f"{BASE_URL}/api/health")
        print(f"状态码: {response.status_code}")
        print(f"响应: {response.json()}")
        return response.status_code == 200
    except Exception as e:
        print(f"健康检查失败: {e}")
        return False

def test_omp_threads():
    """测试OpenMP线程数API"""
    print("\n=== 测试OpenMP线程数 ===")
    try:
        # 获取当前线程数
        response = requests.get(f"{BASE_URL}/api/omp/threads")
        print(f"获取线程数状态码: {response.status_code}")
        print(f"当前线程数: {response.json()}")
        
        # 设置线程数为4
        data = {"threads": 4}
        response = requests.post(f"{BASE_URL}/api/omp/threads", json=data)
        print(f"设置线程数状态码: {response.status_code}")
        print(f"设置结果: {response.json()}")
        
        return True
    except Exception as e:
        print(f"OpenMP测试失败: {e}")
        return False

def test_image_processing_api(image_path, api_endpoint, params=None, files_key='image'):
    """测试图像处理API"""
    if not os.path.exists(image_path):
        print(f"测试图片不存在: {image_path}")
        return False
    
    try:
        files = {files_key: open(image_path, 'rb')}
        data = params or {}
        
        response = requests.post(f"{BASE_URL}/api/transform/{api_endpoint}", 
                               files=files, data=data)
        
        print(f"{api_endpoint} API状态码: {response.status_code}")
        result = response.json()
        print(f"响应: {result}")
        
        if response.status_code == 200 and 'output_file' in result:
            # 尝试下载处理后的文件
            output_file = result['output_file']
            file_response = requests.get(f"{BASE_URL}/api/files/{output_file}")
            if file_response.status_code == 200:
                print(f"成功下载处理后的文件: {output_file}")
                return True
            else:
                print(f"下载文件失败: {file_response.status_code}")
                return False
        
        return response.status_code == 200
    except Exception as e:
        print(f"{api_endpoint} API测试失败: {e}")
        return False

def test_all_image_processing():
    """测试所有图像处理功能"""
    print("\n=== 测试图像处理功能 ===")
    
    # 测试图片路径（需要根据实际情况调整）
    test_image = "1.bmp"  # 假设在backend目录下有这个测试图片
    
    if not os.path.exists(test_image):
        print(f"测试图片不存在: {test_image}")
        print("请确保backend目录下有测试图片文件")
        return False
    
    # 测试各种图像处理功能
    tests = [
        ("grayscale", "灰度转换"),
        ("binary", "二值化", {"threshold": "128"}),
        ("brightness", "亮度调整", {"adjustment": "50"}),
        ("gaussian_blur", "高斯模糊", {"kernel_size": "5", "sigma": "1.0"}),
        ("sobel", "Sobel边缘检测"),
        ("convolution", "自定义卷积", {"kernel": "[[0,-1,0],[-1,5,-1],[0,-1,0]]", "scale": "1"})
    ]
    
    success_count = 0
    for test in tests:
        print(f"\n--- 测试{test[1]} ---")
        if len(test) == 2:
            success = test_image_processing_api(test_image, test[0])
        else:
            success = test_image_processing_api(test_image, test[0], test[2])
        
        if success:
            success_count += 1
        time.sleep(1)  # 避免请求过于频繁
    
    print(f"\n图像处理测试完成: {success_count}/{len(tests)} 成功")
    return success_count == len(tests)

def test_image_stitching():
    """测试图像拼接功能"""
    print("\n=== 测试图像拼接 ===")
    
    # 测试图片路径
    image1 = "2(1).bmp"
    image2 = "2(2).bmp"
    
    if not os.path.exists(image1) or not os.path.exists(image2):
        print(f"拼接测试图片不存在: {image1} 或 {image2}")
        return False
    
    try:
        files = {
            'image1': open(image1, 'rb'),
            'image2': open(image2, 'rb')
        }
        
        response = requests.post(f"{BASE_URL}/api/transform/stitch", files=files)
        
        print(f"图像拼接API状态码: {response.status_code}")
        result = response.json()
        print(f"响应: {result}")
        
        if response.status_code == 200 and 'output_file' in result:
            # 尝试下载拼接后的文件
            output_file = result['output_file']
            file_response = requests.get(f"{BASE_URL}/api/files/{output_file}")
            if file_response.status_code == 200:
                print(f"成功下载拼接后的文件: {output_file}")
                return True
            else:
                print(f"下载文件失败: {file_response.status_code}")
                return False
        
        return response.status_code == 200
    except Exception as e:
        print(f"图像拼接测试失败: {e}")
        return False

def main():
    """主函数"""
    print("开始API测试...")
    
    # 测试健康检查
    if not test_health_check():
        print("健康检查失败，请确保服务器正在运行")
        return
    
    # 测试OpenMP线程数
    test_omp_threads()
    
    # 测试图像处理功能
    test_all_image_processing()
    
    # 测试图像拼接
    test_image_stitching()
    
    print("\n所有测试完成！")

if __name__ == "__main__":
    main() 