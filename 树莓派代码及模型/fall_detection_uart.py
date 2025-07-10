import serial
import time
import math
import csv
from datetime import datetime
import atexit

# 初始化状态标志
weightless = False
crush = False
stable = False
gesture = False

# 初始化时间戳
swtime = 0  # 失重开始时间
crush_time = 0  # 撞击时间
sstime = 0  # 静止开始时间

# 姿态角计算 - 放宽条件
ANGLE_THRESHOLD = 60  # 角度阈值从45度提高到60度(度)
BUFFER_SIZE = 2  # 更小的缓冲区

# 日志缓冲区
LOG_BUFFER = []
LOG_BUFFER_SIZE = 10

# 初始化串口
ser = serial.Serial(
    port='/dev/ttyUSB0',    
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=0.5  # 匹配0.5秒采样率
)

# 创建日志文件
try:
    with open('sensor_log.csv', 'x', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['时间戳', 'X加速度(g)', 'Y加速度(g)', 'Z加速度(g)', '合加速度(g)', 
                         '俯仰角(度)', '横滚角(度)', '姿态异常', 
                         '失重状态', '撞击状态', '静止状态'])
except FileExistsError:
    pass

# 姿态角计算函数（放宽条件）
def calculate_orientation(x_g, y_g, z_g):
    # 计算俯仰角(pitch)和横滚角(roll)
    pitch = math.degrees(math.atan2(x_g, math.sqrt(y_g**2 + z_g**2)))
    roll = math.degrees(math.atan2(y_g, math.sqrt(x_g**2 + z_g**2)))
    
    # 检查角度是否超过阈值（放宽条件）
    orientation_abnormal = abs(pitch) > ANGLE_THRESHOLD or abs(roll) > ANGLE_THRESHOLD
    
    print(f"姿态角: 俯仰={pitch:.1f}°, 横滚={roll:.1f}°, 异常={orientation_abnormal}")
    return pitch, roll, orientation_abnormal

# 解析串口数据帧
def parse_sensor_data(data):
    if len(data) != 9:
        return None
    
    # 检查帧头和帧尾
    if data[0] != 0xAA or data[8] != 0x55:
        return None
    
    # 校验和验证
    checksum = sum(data[0:7]) & 0xFF
    if checksum != data[7]:
        return None
    
    # 解析三轴数据
    x_raw = data[1] | (data[2] << 8)
    y_raw = data[3] | (data[4] << 8)
    z_raw = data[5] | (data[6] << 8)
    
    # 转换为有符号整数
    def to_signed(n):
        return n - 0x10000 if n >= 0x8000 else n
    
    # 转换为重力加速度值
    SCALE_FACTOR = 0.0039
    x_g = to_signed(x_raw) * SCALE_FACTOR
    y_g = to_signed(y_raw) * SCALE_FACTOR
    z_g = to_signed(z_raw) * SCALE_FACTOR
    
    return (x_g, y_g, z_g)

# 重置所有状态
def reset_states():
    global weightless, crush, stable, gesture, weightless_start, stable_start
    weightless = False
    crush = False
    stable = False
    gesture = False
    weightless_start = None
    stable_start = None
    print("状态已重置")

# 清理函数
def cleanup():
    global LOG_BUFFER
    # 写入剩余日志
    if LOG_BUFFER:
        with open('sensor_log.csv', 'a', newline='') as f:
            writer = csv.writer(f)
            writer.writerows(LOG_BUFFER)
    # 关闭串口
    if ser.is_open:
        ser.close()
    print("资源已清理")

# 注册退出处理
atexit.register(cleanup)

# 主程序 - 放宽条件版本
if __name__ == "__main__":
    try:
        print("摔倒检测系统启动")
        print(f"采样率: 0.5 Hz")
        print(f"姿态阈值: {ANGLE_THRESHOLD}度")
        
        # 状态变量
        weightless_start = None
        crush_time = None
        stable_start = None
        last_data_time = time.time()
        
        # 状态机状态
        WAITING_FOR_HEADER = 0
        COLLECTING_DATA = 1
        state = WAITING_FOR_HEADER
        
        current_frame = bytearray()
        frame_count = 0
        
        # 初始化姿态角
        pitch_angle = 0.0
        roll_angle = 0.0
        
        while True:
            current_time = time.time()
            
            # 检查数据超时（2倍采样间隔）
            if current_time - last_data_time > 1.0:
                print("警告: 超过1秒未收到数据")
                last_data_time = current_time
                reset_states()
            
            # 读取串口数据
            data = ser.read(ser.in_waiting or 1)
            if data:
                last_data_time = current_time
                for byte in data:
                    if state == WAITING_FOR_HEADER:
                        if byte == 0xAA:
                            state = COLLECTING_DATA
                            current_frame = bytearray([byte])
                    elif state == COLLECTING_DATA:
                        current_frame.append(byte)
                        
                        if len(current_frame) == 9:
                            state = WAITING_FOR_HEADER
                            
                            if current_frame[8] == 0x55:
                                sensor_data = parse_sensor_data(current_frame)
                                
                                if sensor_data:
                                    frame_count += 1
                                    x_g, y_g, z_g = sensor_data
                                    a = math.sqrt(x_g**2 + y_g**2 + z_g**2)
                                    
                                    # 计算姿态角（每次数据都计算）
                                    pitch_angle, roll_angle, orientation_abnormal = calculate_orientation(x_g, y_g, z_g)
                                    
                                    # 记录数据（包含姿态角）
                                    LOG_BUFFER.append([
                                        datetime.now(), 
                                        x_g, y_g, z_g, a,
                                        pitch_angle, roll_angle, orientation_abnormal,
                                        weightless, crush, stable
                                    ])
                                    
                                    # 定期写入日志
                                    if frame_count % LOG_BUFFER_SIZE == 0:
                                        with open('sensor_log.csv', 'a', newline='') as f:
                                            writer = csv.writer(f)
                                            writer.writerows(LOG_BUFFER)
                                        LOG_BUFFER = []
                                        print(f"已记录 {frame_count} 个数据点")
                                    
                                    # 放宽条件的摔倒检测逻辑
                                    # 1. 失重检测（降低阈值，放宽持续时间）
                                    if a < 0.85:  # 阈值从0.7提高到0.85g
                                        if not weightless:
                                            weightless = True
                                            weightless_start = current_time
                                            print(f"失重开始: {a:.3f}g")
                                    else:
                                        if weightless:
                                            # 失重结束，检查持续时间（放宽要求）
                                            if weightless_start is not None:
                                                duration = current_time - weightless_start
                                                # 不再要求最小持续时间，只要检测到失重就有效
                                                print(f"失重结束: 持续{duration:.2f}秒")
                                            weightless = False
                                            weightless_start = None
                                    
                                    # 2. 撞击检测（降低阈值）
                                    if a > 1.5:  # 阈值从1.8降低到1.5g
                                        if not crush:
                                            crush = True
                                            crush_time = current_time
                                            print(f"撞击检测: {a:.3f}g")
                                            # 更新姿态异常标志
                                            gesture = orientation_abnormal
                                    
                                    # 3. 静止检测（放宽范围）
                                    if 0.9 < a < 1.1:  # 范围从0.9-1.1放宽到0.85-1.15g
                                        if not stable:
                                            stable = True
                                            stable_start = current_time
                                            print(f"静止: {a:.3f}g")
                                    else:
                                        if stable:
                                            stable = False
                                            stable_start = None
                                    
                                    # 4. 摔倒判定（放宽条件）
                                    # 现在只需要满足以下任一条件即可判定为摔倒：
                                    # a) 撞击后姿态异常（即使没有静止状态）
                                    # b) 失重后撞击（即使姿态正常）
                                    # c) 撞击后静止（即使姿态正常）
                                    if crush and (gesture or stable):
                                        # 检查时间条件（放宽要求）
                                        if crush_time is not None and (current_time - crush_time) <= 5.0:
                                            print("！！！检测到摔倒！！！")
                                            print(f"  姿态: 俯仰={pitch_angle:.1f}°, 横滚={roll_angle:.1f}°")
                                            # 报警操作...
                                            reset_states()
                                    
                                    # 5. 超时重置（缩短时间）
                                    if crush_time is not None and current_time - crush_time > 5.0:
                                        print("超时重置")
                                        reset_states()
                                    
                                    # 显示当前状态和姿态角
                                    print(f"时间: {datetime.now().strftime('%H:%M:%S')}, 合加速度: {a:.3f}g, "
                                          f"俯仰角: {pitch_angle:.1f}°, 横滚角: {roll_angle:.1f}°")
                                    
                            current_frame = bytearray()
            else:
                # 没有数据时等待
                time.sleep(0.1)
            
    except KeyboardInterrupt:
        print("程序已终止")
    except Exception as e:
        print(f"错误发生: {str(e)}")
    finally:
        cleanup()