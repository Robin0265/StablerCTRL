#2023.6 颜色识别小球导出横纵坐标差和距离
import sensor, image, time , pyb, os, tf, math, uos, gc
from math import pi
from pyb import UART
from pyb import Timer
from pyb import LED
import json
import math
# import micropython
import ustruct

# micropython.alloc_emergency_exception_buf(1000)


led = pyb.LED(2) # Red LED = 1, Green LED = 2, Blue LED = 3, IR LEDs = 4.
thresholds = [(48, 89, 54, -70, 98, 44), # 颜色阈值
             (26, 74, -58, -10, 61, 14),
             (97, 56, -38, 73, 23, 127)]
threshold_index = 0 # 0 for red, 1 for green, 2 for blue




sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)  # 320*240
# sensor.set_windowing((240, 240))
sensor.skip_frames(time=2000)
sensor.set_auto_gain(False) # must be turned off for color tracking
sensor.set_auto_whitebal(False) # must be turned off for color tracking
sensor.set_auto_exposure(False)
Focus = 1.25                                 # 焦距 in mm

# In QQVGA:
# 每个像素点实际尺寸
W_Pix = 2.05/160                                # in mm
H_Pix = 2.755/120                               # in mm

# 像素点坐标差换算角度差
def pix2ang(pix):
    delta_angle = 180/pi*math.atan((pix*W_Pix)/(2*Focus))         # 近似为视场角计算
    return delta_angle

clock = time.clock()
uart = UART(3, 115200)
uart.init(115200, bits=8, parity=None, stop=1, timeout_char=1000) # 使用给定参数初始化

def tick(timer):            # we will receive the timer object when being called
    global data
    if blobs:
        # print("Find")
        # print('Distance(mm):',output_cood)
        # print('Coordinate (x,y)', output_pix)
        uart.write(data)
    else:
        print("NO FIND")
        uart.write(data)
        # print("Data to be sent: ", data) ## 查看else情况下的data字符数


tim = Timer(4, freq=10)      # create a timer object using timer 4 - trigger at 1Hz
tim.callback(tick)          # set the callback to our tick function

def find_max(blobs):
    max_size=0
    for blob in blobs:
        if blob[2]*blob[3] > max_size:
            max_blob=blob
            max_size = blob[2]*blob[3]
    return max_blob

def Uart_Receive():   #UART接收 改变框小球的颜色阈值
    global threshold_index
    if uart.any():
        temp_data = uart.readchar()
        if temp_data==0:   #红色
           threshold_index=0

           # print(temp_data,threshold_index)
        elif temp_data==1:
           threshold_index=1
           # print(temp_data,threshold_index)

#滑动切片取l均值的函数定义
def process_l_value(l):
    global l_window, l_index, avg_l
    l_window[l_index] = l
    # print("l_window:", l_window)
    # print("l_index:", l_index)
    avg_l = sum(l_window) / len(l_window)
    # print("Average of last 10 values:", avg_l)
    l_index = (l_index + 1) % 10

l_window = [0] * 10  # 初始化一个长度为10的列表，初始值为0
l_index = 0  # 初始化列表索引

l_ave = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

while True:
    clock.tick()
    img = sensor.snapshot()
    Uart_Receive()
    blobs = img.find_blobs([thresholds[threshold_index]])


    if blobs:
        led.on()
        max_blob = find_max(blobs)
        img.draw_rectangle(max_blob[0:4])                               # 画矩形
        img.draw_cross(max_blob.cx(), max_blob.cy())                    # 画十字
        b = max_blob.x() # x-coordinate for the blob
        L = (max_blob.w()+max_blob.h())/2
        for i in range(len(l_ave)-1):
            l_ave[i] = l_ave[i+1]
        l_ave[9] = L
        l_sum = 0
        for j in range(len(l_ave)):
            l_sum += l_ave[j]
        # print("L*10_ave:", l_sum/10)
        l = int(860/L)
        process_l_value(l)
        print("l in cm", l)
# 100cm = K/8.6
# K = 860
# L_real = K/(Radius of Object, in pixels)
# 标定：在真实距离为60cm时，L近似=（33.5+34.0)/2=33.75
# k=60*33.75=2025
# 经验证，在范围[20cm~80cm]区间,误差在2%内

        # 像素位置差(in pixels)
        x_err = max_blob.cx()-img.width()/2                             # 由pan调节x方向位置差
        pan_err_real = pix2ang(x_err);
        # 在线调试打印
        # print("pan error:", pan_err_real)
        y_err = max_blob.cy()-img.height()/2                            # 由tilt调节y方向位置差
        tilt_err_real = pix2ang(y_err);
        # 在线调试打印
        # print("tilt error:", tilt_err_real)

        output_cood = "%d" % (avg_l)


        #算角度
        #发数据
        data = ustruct.pack(
                   "fff", # 三个整数（每个整数占四个字节）
                   float(avg_l), # distance in mm
                   float(tilt_err_real), #数据2
                   float(pan_err_real) #数据3
                   )


    else:
        # micropython.mem_info()
        l = -1
        tanangle = -181
        sin = -181
        data = ustruct.pack(
                   "fff", # 三个整数（每个整数占四个字节）
                   float(l), # up sample by 4    #数据1
                   float(tanangle), # up sample by 4    #数据2
                   float(sin) #数据3（长度为1的字节序列）
                   )

        led.off()
    time.sleep_ms(100)

