#!/bin/bash
# 编译命令

dir="/home/gec/HDD_STORAGE/ISP_1.0/RGB"
# 统计文件数量
count=$(find $dir -type f | wc -l)
echo "目录中的文件数量为：$count"
# 如果文件数量大于10，清空目录
if [ $count -gt 10 ]; then
    echo "BMP文件数量大于10，正在清空目录..."
    rm -rf $dir/*
    echo "目录已清空"
else
    echo "BMP文件数量未超过10，无需清空目录"
fi

dir="/home/gec/HDD_STORAGE/ISP_1.0/BMP"
# 统计文件数量
count=$(find $dir -type f | wc -l)
echo "目录中的文件数量为：$count"
# 如果文件数量大于10，清空目录
if [ $count -gt 10 ]; then
    echo "BMP文件数量大于10，正在清空目录..."
    rm -rf $dir/*
    echo "目录已清空"
else
    echo "BMP文件数量未超过10，无需清空目录"
fi

dir="/home/gec/HDD_STORAGE/ISP_1.0/JPG"
# 统计文件数量
count=$(find $dir -type f | wc -l)
echo "目录中的文件数量为：$count"
# 如果文件数量大于10，清空目录
if [ $count -gt 10 ]; then
    echo "JPG文件数量大于10，正在清空目录..."
    rm -rf $dir/*
    echo "目录已清空"
else
    echo "JPG文件数量未超过10，无需清空目录"
fi



echo "删除原有demo..."
rm demo
echo "编译中..."
g++ ISP.cpp Pre_correction.cpp Thread_action.cpp jpeg_action.cpp -o demo -ljpeg -pthread -lz -L /usr/local/lib/libjpeg.so.9 -L /usr/local/lib/libjpeg.so -L /usr/local/lib/libjpeg.so.9.5.0 -L /usr/local/lib/libjpeg.a -L /usr/local/lib/libjpeg.la \
-L /usr/lib/x86_64-linux-gnu/libz.a
# 检查编译是否成功
if [ $? -eq 0 ]; then
    echo "编译成功"
    # 执行命令
    echo "运行demo..."
    start=$(date +%s)   # 记录开始时间
    ./demo
    # 检查命令是否执行成功
    if [ $? -eq 0 ]; then
        end=$(date +%s)     # 记录结束时间
        diff=$(( $end - $start ))  # 计算时间差
        echo "运行成功，累计花费时间： $diff 秒"
    else
        echo "命令执行失败"
        exit 1
    fi
else
    echo "编译失败"
    exit 1
fi
# 完成
echo "完成"