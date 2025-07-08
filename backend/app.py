from flask import Flask, request, jsonify, send_file
from flask_cors import CORS
import os
import tempfile
from werkzeug.utils import secure_filename
import uuid
import sys
import time
from PIL import Image

# 添加当前目录到Python路径，以便导入image_processing模块
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

try:
    import image_processing
    IMAGE_PROCESSING_AVAILABLE = True
except ImportError as e:
    print(f"警告: 无法导入image_processing模块: {e}")
    IMAGE_PROCESSING_AVAILABLE = False

app = Flask(__name__)
CORS(app)  # 允许跨域请求

# 配置上传文件夹
UPLOAD_FOLDER = 'uploads'
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024  # 16MB max file size

# 允许的文件扩展名
ALLOWED_EXTENSIONS = {'png', 'jpg', 'jpeg', 'gif', 'bmp', 'tiff'}

def allowed_file(filename):
    """检查文件扩展名是否允许"""
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

def convert_to_bmp(input_path):
    """将图片转换为BMP格式"""
    try:
        with Image.open(input_path) as img:
            # 转换为RGB模式（确保24位）
            if img.mode != 'RGB':
                img = img.convert('RGB')
            
            # 生成BMP文件名
            base_name = os.path.splitext(input_path)[0]
            bmp_path = f"{base_name}.bmp"
            
            # 保存为BMP格式
            img.save(bmp_path, 'BMP')
            return bmp_path
    except Exception as e:
        raise ValueError(f'图片格式转换失败: {str(e)}')

def save_uploaded_file_original(file):
    """保存上传的文件并返回原始文件路径（不转换格式）"""
    if not allowed_file(file.filename):
        raise ValueError('不支持的文件类型')
    
    file_extension = file.filename.rsplit('.', 1)[1].lower()
    unique_filename = f"{uuid.uuid4()}.{file_extension}"
    file_path = os.path.join(app.config['UPLOAD_FOLDER'], unique_filename)
    file.save(file_path)
    return file_path

def save_uploaded_file(file):
    """保存上传的文件并返回文件路径"""
    if not allowed_file(file.filename):
        raise ValueError('不支持的文件类型')
    
    file_extension = file.filename.rsplit('.', 1)[1].lower()
    unique_filename = f"{uuid.uuid4()}.{file_extension}"
    file_path = os.path.join(app.config['UPLOAD_FOLDER'], unique_filename)
    file.save(file_path)
    
    # 如果不是BMP格式，转换为BMP
    if file_extension != 'bmp':
        bmp_path = convert_to_bmp(file_path)
        # 删除原始文件
        os.remove(file_path)
        return bmp_path
    
    return file_path

@app.route('/api/transform/grayscale', methods=['POST'])
def grayscale_transform():
    """灰度转换API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        # 获取处理模式参数，默认为parallel
        mode = request.form.get('mode', 'parallel')
        
        input_path = save_uploaded_file(file)
        output_filename = f"gray_{mode}_{os.path.basename(input_path)}"
        output_path = os.path.join(app.config['UPLOAD_FOLDER'], output_filename)
        
        start_time = time.time()
        
        if mode == 'serial':
            elapsed = image_processing.convert_to_grayscale_serial(input_path, output_path)
        else:
            elapsed = image_processing.convert_to_grayscale(input_path, output_path)
        
        end_time = time.time()
        
        return jsonify({
            'success': True,
            'message': f'灰度转换完成 ({mode}模式)',
            'mode': mode,
            'cpp_time': elapsed,
            'parallel_time': elapsed if mode == 'parallel' else None,
            'serial_time': elapsed if mode == 'serial' else None,
            'total_time': end_time - start_time,
            'output_file': output_filename
        })
        
    except Exception as e:
        return jsonify({'error': f'灰度转换失败: {str(e)}'}), 500

@app.route('/api/transform/grayscale/compare', methods=['POST'])
def grayscale_compare():
    """灰度转换并行vs串行对比API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        input_path = save_uploaded_file(file)
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        
        # 并行版本
        parallel_filename = f"gray_parallel_{os.path.basename(input_path)}"
        parallel_path = os.path.join(app.config['UPLOAD_FOLDER'], parallel_filename)
        
        start_time = time.time()
        parallel_elapsed = image_processing.convert_to_grayscale(input_path, parallel_path)
        parallel_total = time.time() - start_time
        
        # 串行版本
        serial_filename = f"gray_serial_{os.path.basename(input_path)}"
        serial_path = os.path.join(app.config['UPLOAD_FOLDER'], serial_filename)
        
        start_time = time.time()
        serial_elapsed = image_processing.convert_to_grayscale_serial(input_path, serial_path)
        serial_total = time.time() - start_time
        
        # 计算加速比
        speedup = serial_elapsed / parallel_elapsed if parallel_elapsed > 0 else 0
        
        return jsonify({
            'success': True,
            'message': '灰度转换并行vs串行对比完成',
            'parallel': {
                'time': parallel_elapsed,
                'total_time': parallel_total,
                'output_file': parallel_filename
            },
            'serial': {
                'time': serial_elapsed,
                'total_time': serial_total,
                'output_file': serial_filename
            },
            'speedup': speedup
        })
        
    except Exception as e:
        return jsonify({'error': f'灰度转换对比失败: {str(e)}'}), 500

@app.route('/api/transform/binary', methods=['POST'])
def binary_transform():
    """二值化转换API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        # 获取阈值参数，默认为128
        threshold = request.form.get('threshold', 128, type=int)
        # 获取处理模式参数，默认为parallel
        mode = request.form.get('mode', 'parallel')
        
        input_path = save_uploaded_file(file)
        output_filename = f"binary_{mode}_{os.path.basename(input_path)}"
        output_path = os.path.join(app.config['UPLOAD_FOLDER'], output_filename)
        
        start_time = time.time()
        
        if mode == 'serial':
            elapsed = image_processing.convert_to_binary_serial(input_path, output_path, threshold)
        else:
            elapsed = image_processing.convert_to_binary(input_path, output_path, threshold)
        
        end_time = time.time()
        
        return jsonify({
            'success': True,
            'message': f'二值化转换完成 ({mode}模式)',
            'threshold': threshold,
            'mode': mode,
            'parallel_time': elapsed if mode == 'parallel' else None,
            'serial_time': elapsed if mode == 'serial' else None,
            'processing_time': end_time - start_time,
            'output_file': output_filename
        })
        
    except Exception as e:
        return jsonify({'error': f'二值化转换失败: {str(e)}'}), 500

@app.route('/api/transform/binary/compare', methods=['POST'])
def binary_compare():
    """二值化并行vs串行对比API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        # 获取阈值参数，默认为128
        threshold = request.form.get('threshold', 128, type=int)
        
        input_path = save_uploaded_file(file)
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        
        # 并行版本
        parallel_filename = f"binary_parallel_{os.path.basename(input_path)}"
        parallel_path = os.path.join(app.config['UPLOAD_FOLDER'], parallel_filename)
        
        start_time = time.time()
        parallel_elapsed = image_processing.convert_to_binary(input_path, parallel_path, threshold)
        parallel_total = time.time() - start_time
        
        # 串行版本
        serial_filename = f"binary_serial_{os.path.basename(input_path)}"
        serial_path = os.path.join(app.config['UPLOAD_FOLDER'], serial_filename)
        
        start_time = time.time()
        serial_elapsed = image_processing.convert_to_binary_serial(input_path, serial_path, threshold)
        serial_total = time.time() - start_time
        
        # 计算加速比
        speedup = serial_elapsed / parallel_elapsed if parallel_elapsed > 0 else 0
        
        return jsonify({
            'success': True,
            'message': '二值化并行vs串行对比完成',
            'threshold': threshold,
            'parallel': {
                'time': parallel_elapsed,
                'total_time': parallel_total,
                'output_file': parallel_filename
            },
            'serial': {
                'time': serial_elapsed,
                'total_time': serial_total,
                'output_file': serial_filename
            },
            'speedup': speedup
        })
        
    except Exception as e:
        return jsonify({'error': f'二值化对比失败: {str(e)}'}), 500

@app.route('/api/transform/brightness', methods=['POST'])
def brightness_transform():
    """亮度调整API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        # 获取亮度调整参数，默认为50
        adjustment = request.form.get('adjustment', 50, type=int)
        # 获取处理模式参数，默认为parallel
        mode = request.form.get('mode', 'parallel')
        
        input_path = save_uploaded_file(file)
        output_filename = f"bright_{mode}_{os.path.basename(input_path)}"
        output_path = os.path.join(app.config['UPLOAD_FOLDER'], output_filename)
        
        start_time = time.time()
        
        if mode == 'serial':
            elapsed = image_processing.adjust_brightness_serial(input_path, output_path, adjustment)
        else:
            elapsed = image_processing.adjust_brightness(input_path, output_path, adjustment)
        
        end_time = time.time()
        
        return jsonify({
            'success': True,
            'message': f'亮度调整完成 ({mode}模式)',
            'adjustment': adjustment,
            'mode': mode,
            'parallel_time': elapsed if mode == 'parallel' else None,
            'serial_time': elapsed if mode == 'serial' else None,
            'processing_time': end_time - start_time,
            'output_file': output_filename
        })
        
    except Exception as e:
        return jsonify({'error': f'亮度调整失败: {str(e)}'}), 500

@app.route('/api/transform/brightness/compare', methods=['POST'])
def brightness_compare():
    """亮度调整并行vs串行对比API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        # 获取亮度调整参数，默认为50
        adjustment = request.form.get('adjustment', 50, type=int)
        
        input_path = save_uploaded_file(file)
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        
        # 并行版本
        parallel_filename = f"bright_parallel_{os.path.basename(input_path)}"
        parallel_path = os.path.join(app.config['UPLOAD_FOLDER'], parallel_filename)
        
        start_time = time.time()
        parallel_elapsed = image_processing.adjust_brightness(input_path, parallel_path, adjustment)
        parallel_total = time.time() - start_time
        
        # 串行版本
        serial_filename = f"bright_serial_{os.path.basename(input_path)}"
        serial_path = os.path.join(app.config['UPLOAD_FOLDER'], serial_filename)
        
        start_time = time.time()
        serial_elapsed = image_processing.adjust_brightness_serial(input_path, serial_path, adjustment)
        serial_total = time.time() - start_time
        
        # 计算加速比
        speedup = serial_elapsed / parallel_elapsed if parallel_elapsed > 0 else 0
        
        return jsonify({
            'success': True,
            'message': '亮度调整并行vs串行对比完成',
            'adjustment': adjustment,
            'parallel': {
                'time': parallel_elapsed,
                'total_time': parallel_total,
                'output_file': parallel_filename
            },
            'serial': {
                'time': serial_elapsed,
                'total_time': serial_total,
                'output_file': serial_filename
            },
            'speedup': speedup
        })
        
    except Exception as e:
        return jsonify({'error': f'亮度调整对比失败: {str(e)}'}), 500

@app.route('/api/transform/gaussian_blur', methods=['POST'])
def gaussian_blur_transform():
    """高斯模糊API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        # 获取高斯模糊参数
        kernel_size = request.form.get('kernel_size', 5, type=int)
        sigma = request.form.get('sigma', 1.0, type=float)
        # 获取处理模式参数，默认为parallel
        mode = request.form.get('mode', 'parallel')
        
        input_path = save_uploaded_file(file)
        output_filename = f"blur_{mode}_{os.path.basename(input_path)}"
        output_path = os.path.join(app.config['UPLOAD_FOLDER'], output_filename)
        
        start_time = time.time()
        
        if mode == 'serial':
            elapsed = image_processing.apply_gaussian_blur_serial(input_path, output_path, kernel_size, sigma)
        else:
            elapsed = image_processing.apply_gaussian_blur(input_path, output_path, kernel_size, sigma)
        
        end_time = time.time()
        
        return jsonify({
            'success': True,
            'message': f'高斯模糊完成 ({mode}模式)',
            'kernel_size': kernel_size,
            'sigma': sigma,
            'mode': mode,
            'parallel_time': elapsed if mode == 'parallel' else None,
            'serial_time': elapsed if mode == 'serial' else None,
            'processing_time': end_time - start_time,
            'output_file': output_filename
        })
        
    except Exception as e:
        return jsonify({'error': f'高斯模糊失败: {str(e)}'}), 500

@app.route('/api/transform/gaussian_blur/compare', methods=['POST'])
def gaussian_blur_compare():
    """高斯模糊并行vs串行对比API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        # 获取高斯模糊参数
        kernel_size = request.form.get('kernel_size', 5, type=int)
        sigma = request.form.get('sigma', 1.0, type=float)
        
        input_path = save_uploaded_file(file)
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        
        # 并行版本
        parallel_filename = f"blur_parallel_{os.path.basename(input_path)}"
        parallel_path = os.path.join(app.config['UPLOAD_FOLDER'], parallel_filename)
        
        start_time = time.time()
        parallel_elapsed = image_processing.apply_gaussian_blur(input_path, parallel_path, kernel_size, sigma)
        parallel_total = time.time() - start_time
        
        # 串行版本
        serial_filename = f"blur_serial_{os.path.basename(input_path)}"
        serial_path = os.path.join(app.config['UPLOAD_FOLDER'], serial_filename)
        
        start_time = time.time()
        serial_elapsed = image_processing.apply_gaussian_blur_serial(input_path, serial_path, kernel_size, sigma)
        serial_total = time.time() - start_time
        
        # 计算加速比
        speedup = serial_elapsed / parallel_elapsed if parallel_elapsed > 0 else 0
        
        return jsonify({
            'success': True,
            'message': '高斯模糊并行vs串行对比完成',
            'kernel_size': kernel_size,
            'sigma': sigma,
            'parallel': {
                'time': parallel_elapsed,
                'total_time': parallel_total,
                'output_file': parallel_filename
            },
            'serial': {
                'time': serial_elapsed,
                'total_time': serial_total,
                'output_file': serial_filename
            },
            'speedup': speedup
        })
        
    except Exception as e:
        return jsonify({'error': f'高斯模糊对比失败: {str(e)}'}), 500

@app.route('/api/transform/sobel', methods=['POST'])
def sobel_transform():
    """Sobel边缘检测API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        # 获取处理模式参数，默认为parallel
        mode = request.form.get('mode', 'parallel')
        
        input_path = save_uploaded_file(file)
        output_filename = f"sobel_{mode}_{os.path.basename(input_path)}"
        output_path = os.path.join(app.config['UPLOAD_FOLDER'], output_filename)
        
        start_time = time.time()
        
        if mode == 'serial':
            elapsed = image_processing.apply_sobel_edge_detection_serial(input_path, output_path)
        else:
            elapsed = image_processing.apply_sobel_edge_detection(input_path, output_path)
        
        end_time = time.time()
        
        return jsonify({
            'success': True,
            'message': f'Sobel边缘检测完成 ({mode}模式)',
            'mode': mode,
            'parallel_time': elapsed if mode == 'parallel' else None,
            'serial_time': elapsed if mode == 'serial' else None,
            'processing_time': end_time - start_time,
            'output_file': output_filename
        })
        
    except Exception as e:
        return jsonify({'error': f'Sobel边缘检测失败: {str(e)}'}), 500

@app.route('/api/transform/sobel/compare', methods=['POST'])
def sobel_compare():
    """Sobel边缘检测并行vs串行对比API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        input_path = save_uploaded_file(file)
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        
        # 并行版本
        parallel_filename = f"sobel_parallel_{os.path.basename(input_path)}"
        parallel_path = os.path.join(app.config['UPLOAD_FOLDER'], parallel_filename)
        
        start_time = time.time()
        parallel_elapsed = image_processing.apply_sobel_edge_detection(input_path, parallel_path)
        parallel_total = time.time() - start_time
        
        # 串行版本
        serial_filename = f"sobel_serial_{os.path.basename(input_path)}"
        serial_path = os.path.join(app.config['UPLOAD_FOLDER'], serial_filename)
        
        start_time = time.time()
        serial_elapsed = image_processing.apply_sobel_edge_detection_serial(input_path, serial_path)
        serial_total = time.time() - start_time
        
        # 计算加速比
        speedup = serial_elapsed / parallel_elapsed if parallel_elapsed > 0 else 0
        
        return jsonify({
            'success': True,
            'message': 'Sobel边缘检测并行vs串行对比完成',
            'parallel': {
                'time': parallel_elapsed,
                'total_time': parallel_total,
                'output_file': parallel_filename
            },
            'serial': {
                'time': serial_elapsed,
                'total_time': serial_total,
                'output_file': serial_filename
            },
            'speedup': speedup
        })
        
    except Exception as e:
        return jsonify({'error': f'Sobel边缘检测对比失败: {str(e)}'}), 500

@app.route('/api/transform/convolution', methods=['POST'])
def convolution_transform():
    """自定义卷积API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        # 获取卷积核参数，默认为锐化卷积核
        kernel_data = request.form.get('kernel', '[[0,-1,0],[-1,5,-1],[0,-1,0]]')
        scale = request.form.get('scale', 1, type=float)
        # 获取处理模式参数，默认为parallel
        mode = request.form.get('mode', 'parallel')
        
        # 解析卷积核
        import json
        try:
            kernel = json.loads(kernel_data)
        except json.JSONDecodeError:
            return jsonify({'error': '卷积核格式错误'}), 400
        
        input_path = save_uploaded_file(file)
        output_filename = f"conv_{mode}_{os.path.basename(input_path)}"
        output_path = os.path.join(app.config['UPLOAD_FOLDER'], output_filename)
        
        start_time = time.time()
        
        if mode == 'serial':
            elapsed = image_processing.apply_custom_convolution_serial(input_path, output_path, kernel, scale)
        else:
            elapsed = image_processing.apply_custom_convolution(input_path, output_path, kernel, scale)
        
        end_time = time.time()
        
        return jsonify({
            'success': True,
            'message': f'自定义卷积完成 ({mode}模式)',
            'kernel': kernel,
            'scale': scale,
            'mode': mode,
            'parallel_time': elapsed if mode == 'parallel' else None,
            'serial_time': elapsed if mode == 'serial' else None,
            'processing_time': end_time - start_time,
            'output_file': output_filename
        })
        
    except Exception as e:
        return jsonify({'error': f'自定义卷积失败: {str(e)}'}), 500

@app.route('/api/transform/convolution/compare', methods=['POST'])
def convolution_compare():
    """自定义卷积并行vs串行对比API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image' not in request.files:
            return jsonify({'error': '没有上传文件'}), 400
        
        file = request.files['image']
        if file.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        # 获取卷积核参数，默认为锐化卷积核
        kernel_data = request.form.get('kernel', '[[0,-1,0],[-1,5,-1],[0,-1,0]]')
        scale = request.form.get('scale', 1, type=float)
        
        # 解析卷积核
        import json
        try:
            kernel = json.loads(kernel_data)
        except json.JSONDecodeError:
            return jsonify({'error': '卷积核格式错误'}), 400
        
        input_path = save_uploaded_file(file)
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        
        # 并行版本
        parallel_filename = f"conv_parallel_{os.path.basename(input_path)}"
        parallel_path = os.path.join(app.config['UPLOAD_FOLDER'], parallel_filename)
        
        start_time = time.time()
        parallel_elapsed = image_processing.apply_custom_convolution(input_path, parallel_path, kernel, scale)
        parallel_total = time.time() - start_time
        
        # 串行版本
        serial_filename = f"conv_serial_{os.path.basename(input_path)}"
        serial_path = os.path.join(app.config['UPLOAD_FOLDER'], serial_filename)
        
        start_time = time.time()
        serial_elapsed = image_processing.apply_custom_convolution_serial(input_path, serial_path, kernel, scale)
        serial_total = time.time() - start_time
        
        # 计算加速比
        speedup = serial_elapsed / parallel_elapsed if parallel_elapsed > 0 else 0
        
        return jsonify({
            'success': True,
            'message': '自定义卷积并行vs串行对比完成',
            'kernel': kernel,
            'scale': scale,
            'parallel': {
                'time': parallel_elapsed,
                'total_time': parallel_total,
                'output_file': parallel_filename
            },
            'serial': {
                'time': serial_elapsed,
                'total_time': serial_total,
                'output_file': serial_filename
            },
            'speedup': speedup
        })
        
    except Exception as e:
        return jsonify({'error': f'自定义卷积对比失败: {str(e)}'}), 500

@app.route('/api/transform/stitch', methods=['POST'])
def stitch_transform():
    """图像拼接API"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        if 'image1' not in request.files or 'image2' not in request.files:
            return jsonify({'error': '需要上传两张图片'}), 400
        
        file1 = request.files['image1']
        file2 = request.files['image2']
        
        if file1.filename == '' or file2.filename == '':
            return jsonify({'error': '没有选择文件'}), 400
        
        # 图像拼接使用OpenCV，可以处理多种格式，不需要强制转换为BMP
        input_path1 = save_uploaded_file_original(file1)
        input_path2 = save_uploaded_file_original(file2)
        output_filename = f"stitched_{uuid.uuid4()}.bmp"
        output_path = os.path.join(app.config['UPLOAD_FOLDER'], output_filename)
        
        start_time = time.time()
        elapsed = image_processing.stitch_images_surf(input_path1, input_path2, output_path)
        end_time = time.time()
        
        return jsonify({
            'success': True,
            'message': '图像拼接完成',
            'cpp_time': elapsed,
            'parallel_time': elapsed,
            'total_time': end_time - start_time,
            'output_file': output_filename
        })
        
    except Exception as e:
        return jsonify({'error': f'图像拼接失败: {str(e)}'}), 500

@app.route('/api/omp/threads', methods=['GET'])
def get_omp_threads():
    """获取OpenMP线程数"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        max_threads = image_processing.get_omp_threads()
        return jsonify({
            'success': True,
            'max_threads': max_threads
        })
    except Exception as e:
        return jsonify({'error': f'获取线程数失败: {str(e)}'}), 500

@app.route('/api/omp/threads', methods=['POST'])
def set_omp_threads():
    """设置OpenMP线程数"""
    if not IMAGE_PROCESSING_AVAILABLE:
        return jsonify({'error': '图像处理模块不可用'}), 500
    
    try:
        data = request.get_json()
        if not data or 'threads' not in data:
            return jsonify({'error': '缺少线程数参数'}), 400
        
        threads = data['threads']
        image_processing.set_omp_threads(threads)
        
        return jsonify({
            'success': True,
            'message': f'已设置线程数为 {threads}'
        })
    except Exception as e:
        return jsonify({'error': f'设置线程数失败: {str(e)}'}), 500

@app.route('/api/files/<filename>', methods=['GET'])
def get_file(filename):
    """获取处理后的文件"""
    try:
        file_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        if not os.path.exists(file_path):
            return jsonify({'error': '文件不存在'}), 404
        
        return send_file(file_path)
    except Exception as e:
        return jsonify({'error': f'获取文件失败: {str(e)}'}), 500

@app.route('/api/health', methods=['GET'])
def health_check():
    """健康检查API"""
    return jsonify({
        'status': 'healthy', 
        'message': '图像处理服务运行正常',
        'image_processing_available': IMAGE_PROCESSING_AVAILABLE
    })

@app.route('/', methods=['GET'])
def index():
    """根路径"""
    return jsonify({
        'message': '图像处理API服务',
        'endpoints': {
            'health': '/api/health',
            'grayscale': '/api/transform/grayscale',
            'binary': '/api/transform/binary',
            'brightness': '/api/transform/brightness',
            'gaussian_blur': '/api/transform/gaussian_blur',
            'sobel': '/api/transform/sobel',
            'convolution': '/api/transform/convolution',
            'stitch': '/api/transform/stitch',
            'get_omp_threads': '/api/omp/threads (GET)',
            'set_omp_threads': '/api/omp/threads (POST)',
            'get_file': '/api/files/<filename>'
        }
    })

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000) 