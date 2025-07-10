import serial
import struct
import time
import requests
from datetime import datetime

# OneNet配置
PRODUCT_ID = "7aLgOr0642"
DEVICE_NAME = "zxy_temp"
API_URL = f"https://open.iot.10086.cn/fuse/http/device/thing/property/post?topic=$sys/{PRODUCT_ID}/{DEVICE_NAME}/thing/property/post&protocol=http"
TOKEN = "version=2018-10-31&res=products%2F7aLgOr0642%2Fdevices%2fzxy_temp&et=2079998482&method=sha1&sign=vkQcdbLdrkooYFBCyd4tLjIIim8%3D"

# 传感器类型标识
SENSOR_ADXL345 = 1
SENSOR_MAX30102 = 2
SENSOR_DHT11 = 3

def upload_to_onenet(data_dict):
    """上传数据到OneNet平台"""
    headers = {"Content-Type": "application/json", "token": TOKEN}
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    # 构建符合物模型的数据结构
    params = {}
    for key, value in data_dict.items():
        # 根据物模型要求处理数据格式
        if key == "temp_value":
            value = round(float(value), 1)  # 浮点型，保留1位小数
        params[key] = {"value": value, "time": current_time}
    
    payload = {
        "id": str(int(time.time())),
        "version": "1.0",
        "params": params
    }
    
    try:
        response = requests.post(API_URL, headers=headers, json=payload)
        result = response.json()
        if result.get("errno") == 0:
            print(f"上传成功: {data_dict}")
        else:
            print(f"上传失败: {result}")
        return result
    except Exception as e:
        print(f"上传错误: {e}")
        return None

def parse_adxl345(data):
    """解析ADXL345加速度计数据"""
    if len(data) != 10:
        return None
    
    # 校验帧结构
    if data[0] != 0xAA or data[9] != 0x55 or data[1] != SENSOR_ADXL345:
        return None
    
    # 解析三轴加速度
    x = struct.unpack('<h', bytes(data[2:4]))[0]
    y = struct.unpack('<h', bytes(data[4:6]))[0]
    z = struct.unpack('<h', bytes(data[6:8]))[0]
    
    # 校验和验证
    checksum = sum(data[:8]) & 0xFF
    if checksum != data[8]:
        print(f"ADXL345校验失败: 计算值{checksum} != 接收值{data[8]}")
        return None
    
    return {'sensor': 'ADXL345', 'x': x, 'y': y, 'z': z}

def parse_max30102(data):
    """解析MAX30102心率血氧数据"""
    if len(data) != 7:
        return None
    
    # 校验帧结构
    if data[0] != 0xAA or data[6] != 0x55 or data[1] != SENSOR_MAX30102:
        return None
    
    # 解析心率和血氧
    heart_rate = struct.unpack('<H', bytes(data[2:4]))[0]
    spo2 = data[4]
    
    # 校验和验证
    checksum = sum(data[:5]) & 0xFF
    if checksum != data[5]:
        print(f"MAX30102校验失败: 计算值{checksum} != 接收值{data[5]}")
        return None
    
    return {'sensor': 'MAX30102', 'heart_rate': heart_rate, 'spo2': spo2}

def parse_dht11(data):
    """解析DHT11温湿度数据"""
    if len(data) != 6:  # 更新为6字节帧
        return None
    
    # 验证帧结构
    if data[0] != 0xAA or data[5] != 0x55 or data[1] != SENSOR_DHT11:
        return None
    
    # 校验和验证
    checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF
    if checksum != data[4]:
        print(f"DHT11校验失败: 计算值{checksum} != 接收值{data[4]}")
        return None
    
    return {
        'sensor': 'DHT11',
        'temperature': data[2],
        'humidity': data[3]
    }

def main():
    ser = serial.Serial('/dev/ttyUSB1', 115200, timeout=1)
    print(f"开始监听串口: {ser.name}")
    
    buffer = bytearray()
    try:
        while True:
            # 读取串口数据
            data = ser.read(ser.in_waiting or 1)
            if data:
                buffer.extend(data)
                
                # 处理缓冲区中的数据
                while len(buffer) >= 2:
                    # 查找帧头0xAA
                    start_idx = buffer.find(b'\xAA')
                    if start_idx == -1:
                        buffer.clear()
                        break
                    
                    # 移除帧头前的无效数据
                    if start_idx > 0:
                        del buffer[:start_idx]
                        continue
                    
                    # 检查是否有足够的数据确定类型
                    if len(buffer) < 2:
                        break
                    
                    sensor_type = buffer[1]
                    frame_length = 0
                    
                    # 根据传感器类型确定帧长度
                    if sensor_type == SENSOR_ADXL345:
                        frame_length = 10
                    elif sensor_type == SENSOR_MAX30102:
                        frame_length = 7
                    elif sensor_type == SENSOR_DHT11:
                        frame_length = 6
                    else:
                        # 未知类型，跳过帧头
                        del buffer[0]
                        continue
                    
                    # 检查是否收到完整帧
                    if len(buffer) < frame_length:
                        break
                    
                    # 提取完整帧
                    frame = buffer[:frame_length]
                    del buffer[:frame_length]
                    
                    # 解析传感器数据
                    result = None
                    if sensor_type == SENSOR_ADXL345:
                        result = parse_adxl345(frame)
                    elif sensor_type == SENSOR_MAX30102:
                        result = parse_max30102(frame)
                    elif sensor_type == SENSOR_DHT11:
                        result = parse_dht11(frame)
                    
                    # 处理解析结果
                    if result:
                        if result['sensor'] == 'ADXL345':
                            print(f"加速度: X={result['x']}, Y={result['y']}, Z={result['z']}")
                        elif result['sensor'] == 'MAX30102':
                            print(f"心率: {result['heart_rate']}bpm, 血氧: {result['spo2']}%")
                            # 上传心率血氧数据
                            upload_to_onenet({
                                "heart_rate": result['heart_rate'],
                                "spo2": result['spo2']
                            })
                        elif result['sensor'] == 'DHT11':
                            print(f"温度: {result['temperature']}°C, 湿度: {result['humidity']}%")
                            # 上传温湿度数据
                            upload_to_onenet({
                                "temp_value": result['temperature'],
                                "humidity_value": result['humidity']
                            })
                    else:
                        print(f"数据解析失败: {frame.hex(' ')}")
            
            time.sleep(0.01)
            
    except KeyboardInterrupt:
        print("程序终止")
    finally:
        ser.close()

if __name__ == "__main__":
    main()