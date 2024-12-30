# VTMR

Video Traffic Motion Reconstruction 是一款面向道路交通事故视频车速鉴定的软件系统。

## 开发环境

vs2019 + QT6.5.1

## 代码目录结构

|3rd 第三方依赖库   
|----EvKeyptTrack 弘目特征跟踪算法   
|----ffmpeg421    开源音视频库   
|----openssl      加密库   
|----PaddleOCR    百度飞桨PaddleOCR   
|----qwt          Qt图表组件   
|bin              编译生成目标   
|doc              文档   
|methodlib        自定义算法   
|VTMR             视频速度重建系统   
|VtmrKeyGenerator 视频速度重建系统注册机   

## 备注
运行缺少的dll库从3rd中获取，opencv库从3rd/opencv-3.4.6-vc14_vc15.exe解压后build/x64/vc15/bin中获取