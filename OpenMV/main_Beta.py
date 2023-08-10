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

#红球 (0, 100, -128, 18, -128, 127)
#黄球 (20, 79, 10, 53, 78, 23)

led = pyb.LED(2) # Red LED = 1, Green LED = 2, Blue LED = 3, IR LEDs = 4.
thresholds = [(20, 79, 10, 53, 78, 23), # 彩色阈值
              (171, 255), #灰度阈值
             (97, 56, -38, 73, 23, 127)]
threshold_index = 0 # 0是小球彩色阈值 1是2进制下的


#边缘检测
kernel_size = 1 # kernel width = (size*2)+1, kernel height = (size*2)+1
kernel = [-1, -1, -1,\
          -1, +8, -1,\
          -1, -1, -1]
# 这个一个高通滤波器。见这里有更多的kernel

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)  # 320*240
# sensor.set_windowing((240, 240))
sensor.skip_frames(time=2000)
sensor.set_auto_gain(False) # must be turned off for color tracking
sensor.set_auto_whitebal(False) # must be turned off for color tracking
sensor.set_auto_exposure(False)
Focus = 1.25                                 # 焦距 in mm
# F=1.25
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
        print("Find")
        #print('Distance(mm):',output_cood)
        # uart.write(data)
        # print('Coordinate (x,y)', output_pix)
    else:
        print("NO FIND")
        #uart.write(data)
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

def find_max_circle(circles):
    max_radius=0
    for circle in circles:
        if circle[0]*1>max_radius:
            max_radius=circle[0]
            max_circle=circle
    return max_circle

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

while True:


    clock.tick()  ##!!!此处是什么意思？
    img = sensor.snapshot()#拍照，获取此时位置
    Uart_Receive()

    blobs = img.find_blobs([thresholds[threshold_index]])  #颜色识别
    if blobs:
        led.on()

        #为找圆做的图像预处理
        img.binary([thresholds[threshold_index]])  # 应用当前颜色阈值进行二值化  相当于已经进行了一次颜色筛选
        img.laplacian(1)#边缘滤波
        img.morph(kernel_size, kernel,add=5)


        circles=img.find_circles(threshold=3500, x_margin=10, y_margin=10, r_margin=10,r_min=10, r_max=20, r_step=2)#找出画面中的圆形
        Judge=0 #判断颜色中心和形状中心是否重合
        count=0
        max_blob = find_max(blobs)#赋予其数据格式
        #挨个比较颜色中心和形状中心
        for blob in blobs:
            for circle in circles:
                img.draw_circle(circle.x(),circle.y(),circle.r(),color=(255,0,0))
                x_judge=abs(circle.x()-blob.cx())
                y_judge=abs(circle.y()-blob.cy())
                if x_judge<10 and y_judge<10:
                    max_blob=blob #当发现重叠时替换max_blob的数据
                    count=count+1
                    break
            if count>0:
                Judge=1
            else:
                Judge=0


    # L_real = K/(Radius of Object, in pixels)
    # 标定：在真实距离为60cm时，L近似=（33.5+34.0)/2=33.75
    # k=60*33.75=2025
    # 经验证，在范围[20cm~80cm]区间,误差在2%内
        if Judge==1: #数据采用后计算数据
            img.draw_rectangle(max_blob[0:4],color=(0,0,255))                               # 画矩形
            img.draw_cross(max_blob.cx(), max_blob.cy())                    # 画十字
            b = max_blob.x() # x-coordinate for the blob
            L = (max_blob.w()+max_blob.h())/2
            # print("L*10:", L*10)
            l = int(8800/L*10)
            process_l_value(l)

            # 像素位置差(in pixels)
            x_err = max_blob.cx()-img.width()/2                             # 由pan调节x方向位置差
            pan_err_real = pix2ang(x_err);
            # 在线调试打印
            print("pan error:", pan_err_real)
            y_err = max_blob.cy()-img.height()/2                            # 由tilt调节y方向位置差
            tilt_err_real = pix2ang(y_err);
            # 在线调试打印
            print("tilt error:", tilt_err_real)
            output_cood = "%d" % (avg_l)

        #算角度
        #发数据
        if Judge==1: #数据采用
            data = ustruct.pack(
                       "fff", # 三个整数（每个整数占四个字节）
                       float(avg_l), # distance in mm
                       float(tilt_err_real), #数据2
                       float(pan_err_real) #数据3
                       )
            print("Pass!")
        else: #数据作废
             data = ustruct.pack(
                   "fff", # 三个整数（每个整数占四个字节）
                   float(-1), # up sample by 4    #数据1
                   float(-181), # up sample by 4    #数据2
                   float(-181) #数据3（长度为1的字节序列）
                   )
             print("Fail!")
        uart.write(data)


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
        uart.write(data)

        led.off()
    # time.sleep_ms(70)

