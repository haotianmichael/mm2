import matplotlib.pyplot as plt
import numpy as np

def parse_chain(chain):
    """
    解析链数组，返回两个数字列表：ref 位置和 read 位置。
    """
    ref_positions = []
    read_positions = []
    for i in chain:
        if i == -1:
            break
        ref_positions.append(ref[i])
        read_positions.append(read[i])
    return ref_positions, read_positions

# 读取文件
with open("in2.txt", "r") as f:
    lines = f.readlines()
ref = [int(line.split()[1]) for line in lines]  # ref 位置
read = [int(line.split()[3]) for line in lines]  # read 位置

with open("out2.txt", "r") as f:
    lines = f.readlines()
chain_lines = f.readlines()  # 保存链信息

# 创建画布
plt.figure(figsize=(10, 10))

# 绘制 anchor
for i in range(len(ref)):
    plt.scatter(ref[i], read[i], color="blue")

# 绘制链
for i in range(1, len(chain_lines)):
    chain = list(map(int, chain_lines[i].split()[1].split(",")))
    if chain[0] == -1:
        continue
    ref_positions, read_positions = parse_chain(chain)
    plt.plot(ref_positions, read_positions, color="red")

# 设置坐标轴标签
plt.xlabel("Ref Position")
plt.ylabel("Read Position")

# 显示图形
plt.show()

