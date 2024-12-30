from datetime import datetime, timedelta
from docx import Document

from docxtpl import DocxTemplate, InlineImage
from docx.shared import Inches

import sys
import json


client = ""
vehiclenumber = ""
clientdate = ""
filename = ""
evaldate = ""
evaladdr = ""
occurdate = ""
occuraddr = ""
hashval = ""
hashname = ""
frame = 0
VIN = ""
VNN = ""
referenceDesFrom = ""
referenceDesTo = ""
distance = ""
carvehicle = ""
speed = 0
lowspeed = 0
maxspeed = 0
appraiser1 = ""
appraiser2 = ""
appraiser1Id = ""
appraiser2Id = ""
standard = ""
savePath = ""
timeMode = ""
maxspeed = ""
lowspeed = ""
OCRText = ""
image1Path = ""
image2Path = ""
image3Path = ""
image1HMS = ""
image2HMS = ""
image3HMS = ""
image1HMSIndex = -1
image2HMSIndex = -1
image3HMSIndex = -1
content1 = ""
content2 = ""
content3 = ""
content4 = ""
content5 = ""
# form1 = ""
# form2 = ""
# form3 = ""
# form4 = ""
# form5 = ""
# form6 = ""
# form7 = ""
def process_data(data):
    global client
    global vehiclenumber
    global clientdate
    global filename
    global evaldate
    global evaladdr
    global occurdate
    global occuraddr
    global hashname
    global hashval
    global frame
    global VIN
    global VNN
    global referenceDesFrom
    global referenceDesTo
    global distance
    global carvehicle
    global speed
    global CNdate
    global appraiser1
    global appraiser2
    global appraiser1Id
    global appraiser2Id
    global standard
    global savePath
    global timeMode
    global maxspeed
    global lowspeed
    global OCRText
    global image1Path
    global image2Path
    global image3Path
    global image1HMS
    global image2HMS
    global image3HMS
    global image1HMSIndex
    global image2HMSIndex
    global image3HMSIndex
    global content1
    global content2
    global content3
    global content4
    global content5
    # global form1
    # global form2
    # global form3
    # global form4
    # global form5
    # global form6
    # global form7
    client = data['client']
    vehiclenumber = data['licensePlateNumber']
    clientdate = data['dateOfCommission']
    filename = data['fileName']
    evaldate = data['appraisalDate']
    evaladdr = data['appraisalLocation']
    occurdate = data['accidentTime']
    occuraddr = data['accidentLocation']
    hashname = data['hashName']
    hashval = data['hashVal']
    frame = data["fps"]
    VIN = data['vehicleVIN']
    VNN = -1
    referenceDesFrom = data['referenceDesFrom']
    referenceDesTo = data['referenceDesTo']
    distance = data['distance']
    carvehicle = data['licensePlateNumber'] + data['vehicleBrand'] + "牌" + data['vehicleType'] 
    speed = data['v01']
    lowspeed = 0
    maxspeed = 0
    appraiser1 = data['appraiser1']
    appraiser2 = data['appraiser2']
    appraiser1Id = data['idOfAppraiser1']
    appraiser2Id = data['idOfAppraiser2']
    standard = data['standard']
    OCRText = data['OCRText']
    image1Path = data['image1Path']
    image2Path = data['image2Path']
    image3Path = data['image3Path']
    image1HMS = data['image1HMS']
    image2HMS = data['image2HMS']
    image3HMS = data['image3HMS']
    image1HMSIndex = data['image1HMSIndex']
    image2HMSIndex = data['image2HMSIndex']
    image3HMSIndex = data['image3HMSIndex']
    content1 = data['content1']
    content2 = data['content2']
    content3 = data['content3']
    content4 = data['content4']
    content5 = data['content5']
    savePath = data['savePath']
    timeMode = data['timeMode']
    maxspeed = data['v11']
    # form1 = f't = \frac{{1}}{{{frame}}}'
    # form2 = f'v = \frac{{l}}{{t}}'
    # form3 = f''
    # form4 = ""
    # form5 = ""
    # form6 = ""
    # form7 = ""

    if(timeMode == 0):
        maxspeed = data['v11']
        lowspeed = data['v01']
    elif(timeMode == 1):
        maxspeed = data['v12']
        lowspeed = data['v02']
    else:
        maxspeed = data['v13']
        lowspeed = data['v03']
    # if(float(maxspeed) <= 0.0):
    #     maxspeed = lowspeed
    speed = (maxspeed + lowspeed)/2

    if(content2 != ""):
        content2 = "\n" + "    " +content2
    if(content3 != ""):
        content3 = "\n" + "    " + content3
    if(content4 != ""):
        content4 = "\n" + "    " + content4
    if(content5 != ""):
        content5 = "\n" + "    " + content5

if len(sys.argv) != 2:
    print("Incorrect number of arguments!")
    sys.exit(1)
    
data = json.loads(sys.argv[1])

process_data(data)

current_time = datetime.now()

# 格式化时间为 "YYYY年MM月DD日HH时MM分"
def format_date(input_date):
    # 解析输入日期
    date = datetime.strptime(input_date, "%Y-%m-%d")
    
    # 减去 3 天
    new_date = date - timedelta(days=3)
    
    # 格式化为目标格式
    return new_date.strftime("%Y年%m月%d日")

def format_datetime(input_datetime):
    # 解析输入的日期时间
    dt = datetime.strptime(input_datetime, "%Y-%m-%d %H:%M:%S")
    
    # 格式化为目标格式
    return dt.strftime("%Y年%m月%d日%H时%M分")


def format_chinese_time1(dt):
    # 将数字转换为中文数字的映射
    chinese_numerals = {str(i): "○一二三四五六七八九"[i] for i in range(10)}
    
    # 替换年份中的数字为中文数字
    year = "".join(chinese_numerals[digit] for digit in dt.strftime("%Y"))
    
    # 月份和日期的特殊处理
    def convert_to_chinese_number(number):
        if number < 10:
            return chinese_numerals[str(number)]
        elif number == 10:
            return "十"
        else:
            tens = number // 10
            units = number % 10
            if tens == 1:
                return f"十{chinese_numerals[str(units)]}" if units > 0 else "十"
            return f"{chinese_numerals[str(tens)]}十{chinese_numerals[str(units)]}" if units > 0 else f"{chinese_numerals[str(tens)]}十"
    
    # 月份和日期转换
    month = convert_to_chinese_number(dt.month)
    day = convert_to_chinese_number(dt.day)
    
    # 时间的小时和分钟转换
    hour = convert_to_chinese_number(dt.hour)
    minute = convert_to_chinese_number(dt.minute)
    
    # 拼接成最终格式
    return f"{year}年{month}月{day}日"


clientdate = format_date(clientdate)
evaldate = format_date(evaldate)
occurdate = format_datetime(occurdate)
CNdate = format_chinese_time1(current_time)

# 加载 Word 模板文件
template_path = r"C:\\Users\\fecwo\\Desktop\\车速.docx"  # 模板文件路径
# output_path = r"C:\\Users\\fecwo\\Desktop\\车速_生成1.docx"  # 替换后的输出文件路径
output_path = savePath

doc = DocxTemplate(template_path)

# 替换内容的字典
values = {
    "yeardate": datetime.now().year,
    "client": client,
    "vehiclenumber": vehiclenumber,
    "clientdate": clientdate,
    "filename": filename,
    "evaldate": evaldate,
    "evaladdr": evaladdr,
    "occurdate": occurdate,
    "occuraddr": occuraddr,
    "hashname": hashname,
    "hashval": hashval,
    "frame": frame,
    "VIN": VIN,
    "VNN": VNN,
    "referenceDesFrom": referenceDesFrom,
    "referenceDesTo": referenceDesTo,
    "distance": distance,
    "carvehicle": carvehicle,
    "speed": speed,
    "CNdate": CNdate,
    "appraiser1" : appraiser1,
    "appraiser2" : appraiser2,
    "appraiser1Id" : appraiser1Id,
    "appraiser2Id" : appraiser2Id,
    "standard": standard,
    "maxspeed": maxspeed,
    "lowspeed": lowspeed,
    "OCRText": OCRText,
    "image1": InlineImage(doc, image1Path, width=Inches(4)),
    "image2": InlineImage(doc, image2Path, width=Inches(4)),
    "image3": InlineImage(doc, image3Path, width=Inches(4)),
    "image1HMS":image1HMS,
    "image2HMS":image2HMS,
    "image3HMS":image3HMS,
    "image1HMSIndex":image1HMSIndex,
    "image2HMSIndex":image2HMSIndex,
    "image3HMSIndex":image3HMSIndex,
    "content1":content1,
    "content2":content2,
    "content3":content3,
    "content4":content4,
    "content5":content5,
    # "form1" : form1,
    # "form2" : form2,
    # "form3" : form3,
    # "form4" : form4,
    # "form5" : form5,
    # "form6" : form6,
    # "form7" : form7
}

def replace_placeholders_in_word(values, output_path):
  
    # 渲染模板文件
    doc.render(values)

    # 保存替换后的文档
    doc.save(output_path)


# 替换占位符
replace_placeholders_in_word(values, output_path)

print(f"文档已保存为 '{output_path}'")