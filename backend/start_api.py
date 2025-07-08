#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
图像处理API服务启动脚本
"""

import os
import sys
import subprocess
import time

def check_dependencies():
    """检查依赖包是否安装"""
    required_packages = ['flask', 'flask_cors', 'requests']
    missing_packages = []
    
    for package in required_packages:
        try:
            __import__(package.replace('-', '_'))
        except ImportError:
            missing_packages.append(package)
    
    if missing_packages:
        print(f"缺少依赖包: {', '.join(missing_packages)}")
        print("请运行以下命令安装:")
        print(f"pip install {' '.join(missing_packages)}")
        return False
    
    return True

def check_image_processing_module():
    """检查image_processing模块是否可用"""
    try:
        import image_processing
        print("✓ image_processing模块加载成功")
        return True
    except ImportError as e:
        print(f"✗ image_processing模块加载失败: {e}")
        print("请确保image_processing.cpython-*.so文件在当前目录下")
        return False

def create_uploads_directory():
    """创建uploads目录"""
    uploads_dir = 'uploads'
    if not os.path.exists(uploads_dir):
        os.makedirs(uploads_dir)
        print(f"✓ 创建uploads目录: {uploads_dir}")
    else:
        print(f"✓ uploads目录已存在: {uploads_dir}")

def start_server():
    """启动Flask服务器"""
    print("\n=== 启动图像处理API服务 ===")
    
    # 检查依赖
    if not check_dependencies():
        return False
    
    # 检查image_processing模块
    if not check_image_processing_module():
        return False
    
    # 创建uploads目录
    create_uploads_directory()
    
    print("\n启动Flask服务器...")
    print("服务地址: http://localhost:5000")
    print("API文档: http://localhost:5000/")
    print("健康检查: http://localhost:5000/api/health")
    print("\n按 Ctrl+C 停止服务")
    
    try:
        # 导入并运行Flask应用
        from app import app
        app.run(debug=True, host='0.0.0.0', port=5000)
    except KeyboardInterrupt:
        print("\n\n服务已停止")
    except Exception as e:
        print(f"\n启动服务失败: {e}")
        return False
    
    return True

def main():
    """主函数"""
    print("图像处理API服务启动器")
    print("=" * 40)
    
    # 检查Python版本
    if sys.version_info < (3, 7):
        print("错误: 需要Python 3.7或更高版本")
        return False
    
    # 启动服务器
    return start_server()

if __name__ == "__main__":
    success = main()
    if not success:
        sys.exit(1) 