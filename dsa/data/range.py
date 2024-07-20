import matplotlib.pyplot as plt

# 读取in2.txt文件中的数据
in_file = "/mnt/data/in2.txt"
anchors = []
with open(in_file, "r") as f:
    for line in f.readlines()[1:]:  # 跳过第一行
        parts = line.strip().split()
        if len(parts) >= 4:
            ref_pos = int(parts[1])
            read_pos = int(parts[3])
            anchors.append((ref_pos, read_pos))

# 读取out2.txt文件中的数据
out_file = "/mnt/data/out2.txt"
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

# 画出链
for i, prev in chains:
    if prev != -1:
        ref_pos_i, read_pos_i = anchors[i]
        ref_pos_prev, read_pos_prev = anchors[prev]
        plt.plot([ref_pos_i, ref_pos_prev], [read_pos_i, read_pos_prev], c='red')

plt.xlabel('Reference Position')
plt.ylabel('Read Position')
plt.title('Anchor Distribution and Chains')
plt.legend()
plt.show()

