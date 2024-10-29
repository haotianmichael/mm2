import matplotlib.pyplot as plt

# 读取in1.txt文件中的数据
in_file = "in2.txt"
anchors = []
with open(in_file, "r") as f:
    for line in f.readlines()[1:]:  # 跳过第一行
        parts = line.strip().split()
        if len(parts) >= 4:
            ref_pos = int(parts[1])
            read_pos = int(parts[3])
            anchors.append((ref_pos, read_pos))

# 读取out1.txt文件中的数据
out_file = "out2.txt"
chains = []
with open(out_file, "r") as f:
    for idx, line in enumerate(f.readlines()[2:]):  # 跳过前两行
        parts = line.strip().split()
        if len(parts) >= 2:
            prev_anchor = int(parts[1])
            chains.append((idx, prev_anchor))

# 画出anchors分布
ref_positions, read_positions = zip(*anchors)
plt.scatter(ref_positions, read_positions, c='blue', label='Anchors')

# 仅在参考序列维度上画出链
for i, prev in chains:
    if prev != -1:
        ref_pos_i, read_pos_i = anchors[i]
        ref_pos_prev, read_pos_prev = anchors[prev]
        # 画水平线（在ref维度上链接）
        #plt.plot([ref_pos_i, ref_pos_prev], [read_pos_i, read_pos_i], c='red')
        # 如果想要画垂直线（在read维度上链接），可以取消下面的注释
        plt.plot([ref_pos_i, ref_pos_i], [read_pos_i, read_pos_prev], c='red')

plt.xlabel('Reference Position')
plt.ylabel('Read Position')
plt.title('Anchor Distribution and Chains (Ref Dimension)')
plt.legend()
plt.show()

