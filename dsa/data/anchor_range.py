import matplotlib.pyplot as plt

# 读取文件数据
ref_positions = []
read_positions = []

with open('in1.txt', 'r') as file:
    next(file)  # 跳过第一行
    for line in file:
        columns = line.strip().split()
        if len(columns) >= 4:
            ref_position = int(columns[1])
            read_position = int(columns[3])
            ref_positions.append(ref_position)
            read_positions.append(read_position)

# 绘制散点图
plt.figure(figsize=(10, 6))
plt.scatter(ref_positions, read_positions, s=10, alpha=0.6)
plt.xlabel('Ref Position')
plt.ylabel('Read Position')
plt.title('Anchors Distribution on Ref and Read Positions')
plt.grid(True)
plt.show()

