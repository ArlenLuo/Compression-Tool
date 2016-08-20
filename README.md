# Compression-Tool
一个实现压缩和解压缩文件的代码。

### 编译环境：

Ubuntu g++


### 用法：

<pre>
~$ make
~$ ./Compression input_file output_file //压缩文件
~$ ./Compression -d input_file output_file //解压文件
</pre>

### 代码思路：

利用两个有序队列构造huffman编码；
压缩后的文件分为记录信息的header和主文件body，将以下信息储存在header中：
      1.叶子数-1（一共可能256个叶子，叶子数减1乘2恰为树枝数）
      2.读码的余数 
      3.HF树形状（前序遍历走过的树枝，左为0右为1） 
      4.HF树叶子的信息


