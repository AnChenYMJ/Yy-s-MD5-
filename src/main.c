#include <stdio.h>
#include <math.h>

#define ZU 64		//MD5中，每组必须64字节

//获取到的位置
long g_lDataBytes = 0;

//获取文件字节数
void GetLen(char* filename);
void GetData(char* filename);

//计算所需要的常数（工作常用到的知识）
	//一、4个缓冲器常数
unsigned char md5[16] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };
unsigned int* pMd5 = (unsigned int*)md5;	//pMd5[0] pMd5[1] pMd5[2] pMd5[3]	（unsigned int的类型刚好4字节为单位，用下标就能使用对应常数了）
	//二、64元素常数组
unsigned int CS_64[64];
void Init64(void);		//具体计算,
	//三、16元素常数组（转换例程函数）(固定的数字，如何来的不清楚，文档未说明，数学家弄得，我们先别管，知道怎么用就好)
#define CS1_1 7
#define CS1_2 12
#define CS1_3 17
#define CS1_4 22
#define CS2_1 5
#define CS2_2 9
#define CS2_3 14
#define CS2_4 20
#define CS3_1 4
#define CS3_2 11
#define CS3_3 16
#define CS3_4 23
#define CS4_1 6
#define CS4_2 10
#define CS4_3 15
#define CS4_4 21

//计算算法
	//函数指针(并且重命名) 这样就可以用函数的名字（指针）调用同样参数类型的函数了
typedef unsigned int (*pFun)(unsigned int, unsigned int, unsigned int);
	//四个缓冲器算法
unsigned int Md5_1(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_2(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_3(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_4(unsigned int x, unsigned int y, unsigned int z);
	//生成1个新的MD5值(核心算法函数)
unsigned int NewMD5(unsigned int md1, unsigned int md2, unsigned int md3, unsigned int md4, pFun Fun, unsigned int data, unsigned int cs64, unsigned int cs16);
	//获取MD5值
void GetMD5(char data[ZU]);

//将生成的新MD5值，存进文件中
void Save(char* filename);


//主函数参数，即用于程序接受数据之类，通用的只有前两个（个数、数据），第三个是VS编译器带的，存放一些环境配置信息（可打印查看）
int main(int argc, int argv[], int argd[])
{
	//传递参数的方式（指的是程序，即主函数）
	if (0)
	{
		//VS编译器特有的，可查看环境配置目录，其他编译器不支持不识别
		printf(argd[0]);
		putchar('\n');
		printf(argd[5]);
		putchar('\n');
		printf(argd[9]);
		putchar('\n'); putchar('\n');

		//参数1：数量  参数2：数据具体是什么
		printf("%d \n", argc);		//未传递的情况下默认是 1 个
		printf(argv[0]);	//默认是本程序的exe文件，打印的也就是本程序exe的路径
		putchar('\n'); putchar('\n');

		//向本程序传递参数的方式  运行传递：4种：1：系统cmd	2：右键--打开方式	  3：文件拖动  4：双击	（笔记中详细讲解）
		//调试传递：1种
		printf(argv[0]);			//这三行就相当于一个程序了，用本程序的exe，打印另一个文件的相关信息
		putchar('\n');
		printf(argv[1]);			//这俩都是地址，分别为 本程序exe、需要传递的文件 他俩的地址 
	}
	
	//64常数的初始化		必须在最开始，不然会出错，因为生成MD5要用到这个常数
	Init64();
	//获取文件字节数		用调试传递指定一个可编辑文件来测试
	GetLen(argv[1]);
	printf("文件的字节数：%d \n", g_lDataBytes);	//断点调试 来看  似乎中文占的字节数很高
	//获取文件内容进行MD5运算
	GetData(argv[1]);
	//将生成的新MD5值，存进文件中,并打印
	Save(argv[2]);

	system("pause > 0");
	return 0;
}

void GetLen(char* filename)
{
	

	//打开文件
	FILE* pFile = NULL;									//相当于文件指针，指向文件内的位置
	errno_t res = fopen_s(&pFile, filename, "r");		//参1：二级指针 参2：文件名 参3：打开的操作方式，r：只读  返回值：int类型的别名 返回0则打开正常，非0出现错误
	if (0 != res || NULL == pFile)						//错误处理
		return;
	//文件指针跳到文件结尾
	fseek(pFile, 0, SEEK_END);							//参1：文件指针 参2：精调位置 参3：相对位置 比如0, SEEK_END 就是跳到以文件结尾为头的下标0的位置，正好是末尾字符的下一位 就是eof的位置（文件结尾标识）
														//相对位置有3种：SEEK_SET 文件头，精调位置为6的话，就正好是文件中下标为6的字符所在位置
														//SEEK_CUR：当前位置 比如文件指针当前所指的位置，精调位置+ - 来操控指向哪里（文件指针也许此前被调过）
	//获取位置											//已指向文件结尾eof的位置，所以此时下标 数值上等于 文件字节数
	g_lDataBytes = ftell(pFile);						//MSDN上还有一种用法，此用法一般用于文件小于2G的情况（数字大约正负20亿） 另一种用于大于2G的情况 
														//_int64 _ftelli64 就是另一种用于大于2G的情况 （可自行算或查，40个4G左右）
	//关闭文件
	fclose(pFile);
}
void GetData(char* filename)
{
	//计算 满组的有多少个 文件末尾不足64字节的留有多少字节
	int man = g_lDataBytes / ZU;		//计算填装64字节，能填满的组
	int fanil = g_lDataBytes % ZU;		//计算文件末尾不足64字节的组

	//打开文件
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, filename, "r");
	if (0 != res || NULL == pFile)
		return;
	//满组的处理
	for (int i = 0; i < man; i++)
	{
		//依次读取文件中的数据，以64字节为单位读一次
		char str[ZU] = { 0 };
		fread(str, 1, ZU, pFile);		//文件指针pFile开始把数据读进数组str 中， 读多少：参2 参3 的乘积。所以这俩可以换位置，无所谓的，只需要乘积

		//MD5运算（用当前读到的那组数据）
		GetMD5(str);
	}
	//1. 末尾不足64的组 x > 0  && x <= 56 (x为改组的字节数)	因为要留8字节来存数据量，故64-8 == 56
	if (fanil > 0 && fanil < 56)
	{
		//依次读取文件中的数据
		char str[ZU] = { 0 };
		int count = fread(str, 1, ZU, pFile);	//返回值，记录读出来的字节数
		//最后留的8字节空间，赋值存储数据量（字节数）
		*(unsigned long long*)(str + 56) = g_lDataBytes * 8;	//str数组名字，就是该空间首地址， +56后就是末尾8字节空间的首地址了，
																//将末尾8空间看作整体强转成一个数据类型，存储数据量，
																//g_lDataBytes再上一个函数中，记录了位置（即元素个数） *8 后得到此文件的总位数
		//补充 10000....  末尾那组，尾元素与最后的8字节中间有空余的话，用10000....来填充（按位）
		if (count < 56)
		{
			str[count] = 0x80;	//count下标位置开始就是要填充的位置开始，一个字节8位，所以是1000 0000 十六进制为0x80 ，不能写1，因为这样该字节就不是1开头了，此后的字节填0
			for (int j = count + 1; j < 56; j++)	//注意边界，此处count当作下标，不能 =56
				str[j] = 0;
		}
		////打印测试一下		(末尾8位因为整合到了一起，按字节来打印，就不直观的知道具体数字，可以整体来打印8字节)
		//for (int n = 0; n < 64; n++)
		//	printf("%c  %d\n", str[n], str[n]);
		////测试打印末尾8字节
		//printf("%d \n", *(unsigned long long*)(str + 56));

		//把这组进行MD5运算
		GetMD5(str);
	}
	//2.末尾那组是满组时的情况
	else if (0 == fanil)
	{
		//重新申请一组空间(64字节)
		char str[ZU] = { 0 };
		//末尾8字节记录数据量
		*(unsigned long long*)(str + 56) = g_lDataBytes * 8;
		//从下标0开始到末尾8字节，按位填充 10000000...
		str[0] = 0x80;
		for (int j = 1; j < 56; j++)
			str[j] = 0;
		//可修改调试的txt文件内容，使其字节数为64的倍数，来测试本段代码（打印测试）

		//这一组进行MD5运算
		GetMD5(str);
	}
	//3.末尾那组 > 56 && < 64 字节时
	else if (fanil >= 56 && fanil < 64)
	{
		//读数据
		char str[ZU] = { 0 };
		int count = fread(str, 1, ZU, pFile);
		//将这组末尾按位填充10000000...
		str[count] = 0x80;
		for (int j = count + 1; j < 64; j++)
			str[j] = 0;
		//本组进行MD5算法
		GetMD5(str);

		//重新申请一组64字节空间
		char strNew[ZU] = { 0 };	//此步骤相当于每个字节都填充了0(上一组已经有1开头的了，此组的填充位全填0即可)
		//新申请的空间末尾8字节存储数据量
		*(unsigned long long*)(strNew + 56) = g_lDataBytes * 8;
		//本组进行MD5算法
		GetMD5(strNew);

	}

	//关闭文件（别忘了）
	fclose(pFile);
}
void Init64(void)
{
	//常数的计算方式：4294967296乘以abs(sin(i))后得到的值的整数部分（i是弧度，abs是函数（求绝对值），sin是正弦）（需要<math.h>）
	for (int i = 1; i <= 64; i++)
	{
		CS_64[i - 1] = (unsigned int)(4294967296 * fabs(sin(i)));	//(unsigned int)起到取整作用，sin F12进去看得知i是double类型，而abs只对对int整形有效
																	//而fabs则对浮点型double有效，故采用fabs。最后结果强转成整形，用来取得整数部分
		//打印测试
		//printf("%x ", CS_64[i - 1]);
	}
		
}
//四个缓冲器算法	(具体算法必须按照顺序对应函数)（GetMD5的函数来使用）
unsigned int Md5_1(unsigned int x, unsigned int y, unsigned int z)
{
	return ((x & y) | ((~x) & z));	//& 按位与   | 按位或   ~ 按位取反
}
unsigned int Md5_2(unsigned int x, unsigned int y, unsigned int z)
{
	return ((x & z) | ( y & (~z)));
}
unsigned int Md5_3(unsigned int x, unsigned int y, unsigned int z)
{
	return (x ^ y ^ z);
}
unsigned int Md5_4(unsigned int x, unsigned int y, unsigned int z)
{
	return (y ^ (x | (~z)));
}
//生成1个新的MD5值（详细参数介绍看笔记）
unsigned int NewMD5(unsigned int md1, unsigned int md2, unsigned int md3, unsigned int md4, pFun Fun, unsigned int data, unsigned int cs64, unsigned int cs16)
{
	//第1步
	unsigned int md5_New = md1 + Fun(md2, md3, md4) + data + cs64;	//写法固定的，我这里用了复制
	//第2步
	md5_New = (md5_New << cs16) | (md5_New >> (32 - cs16));
	//第3步
	md1 = md2 + md5_New;
	//返回计算结果
	return md1;		//传参时，会按照4个缓冲器常数，有顺序的轮换着传递给md1，即每次运算的md1是不同的
}
//获取MD5值
void GetMD5(char data[ZU])
{
	//将数据转换成4字节为一小段的数据类型,64字节就分成了16个
	unsigned int* pData = (unsigned int*)data;	

	//赋值记录那4个缓冲器常量，方便辨认顺序
	unsigned int A = pMd5[0];
	unsigned int B = pMd5[1];
	unsigned int C = pMd5[2];
	unsigned int D = pMd5[3];

	//第1轮16次（有固定顺序，不能出错,返回值必须是赋值给参1）
	A = NewMD5(A, B, C, D, Md5_1, pData[0], CS_64[0], CS1_1);
	D = NewMD5(D, A, B, C, Md5_1, pData[1], CS_64[1], CS1_2);
	C = NewMD5(C, D, A, B, Md5_1, pData[2], CS_64[2], CS1_3);
	B = NewMD5(B, C, D, A, Md5_1, pData[3], CS_64[3], CS1_4);

	A = NewMD5(A, B, C, D, Md5_1, pData[4], CS_64[4], CS1_1);
	D = NewMD5(D, A, B, C, Md5_1, pData[5], CS_64[5], CS1_2);
	C = NewMD5(C, D, A, B, Md5_1, pData[6], CS_64[6], CS1_3);
	B = NewMD5(B, C, D, A, Md5_1, pData[7], CS_64[7], CS1_4);

	A = NewMD5(A, B, C, D, Md5_1, pData[8], CS_64[8], CS1_1);
	D = NewMD5(D, A, B, C, Md5_1, pData[9], CS_64[9], CS1_2);
	C = NewMD5(C, D, A, B, Md5_1, pData[10], CS_64[10], CS1_3);
	B = NewMD5(B, C, D, A, Md5_1, pData[11], CS_64[11], CS1_4);

	A = NewMD5(A, B, C, D, Md5_1, pData[12], CS_64[12], CS1_1);
	D = NewMD5(D, A, B, C, Md5_1, pData[13], CS_64[13], CS1_2);
	C = NewMD5(C, D, A, B, Md5_1, pData[14], CS_64[14], CS1_3);
	B = NewMD5(B, C, D, A, Md5_1, pData[15], CS_64[15], CS1_4);

	//第2轮：	注意参考表里的参6 其实是实际函数的末尾参数，顺序有调整
	/*
	[ABCD 1 5 17] [DABC 6 9 18] [CDAB 11 14 19] [BCDA 0 20 20]
	[ABCD 5 5 21] [DABC 10 9 22] [CDAB 15 14 23] [BCDA 4 20 24]
	[ABCD 9 5 25] [DABC 14 9 26] [CDAB 3 14 27] [BCDA 8 20 28]
	[ABCD 13 5 29] [DABC 2 9 30] [CDAB 7 14 31] [BCDA 12 20 32]
	*/ 
	A = NewMD5(A, B, C, D, Md5_2, pData[1], CS_64[16], CS2_1);
	D = NewMD5(D, A, B, C, Md5_2, pData[6], CS_64[17], CS2_2);
	C = NewMD5(C, D, A, B, Md5_2, pData[11], CS_64[18], CS2_3);
	B = NewMD5(B, C, D, A, Md5_2, pData[0], CS_64[19], CS2_4);

	A = NewMD5(A, B, C, D, Md5_2, pData[5], CS_64[20], CS2_1);
	D = NewMD5(D, A, B, C, Md5_2, pData[10], CS_64[21], CS2_2);
	C = NewMD5(C, D, A, B, Md5_2, pData[15], CS_64[22], CS2_3);
	B = NewMD5(B, C, D, A, Md5_2, pData[4], CS_64[23], CS2_4);

	A = NewMD5(A, B, C, D, Md5_2, pData[9], CS_64[24], CS2_1);
	D = NewMD5(D, A, B, C, Md5_2, pData[14], CS_64[25], CS2_2);
	C = NewMD5(C, D, A, B, Md5_2, pData[3], CS_64[26], CS2_3);
	B = NewMD5(B, C, D, A, Md5_2, pData[8], CS_64[27], CS2_4);

	A = NewMD5(A, B, C, D, Md5_2, pData[13], CS_64[28], CS2_1);
	D = NewMD5(D, A, B, C, Md5_2, pData[2], CS_64[29], CS2_2);
	C = NewMD5(C, D, A, B, Md5_2, pData[7], CS_64[30], CS2_3);
	B = NewMD5(B, C, D, A, Md5_2, pData[12], CS_64[31], CS2_4);

	//第3轮：
	/*
	[ABCD 5 4 33] [DABC 8 11 34] [CDAB 11 16 35] [BCDA 14 23 36]
	[ABCD 1 4 37] [DABC 4 11 38] [CDAB 7 16 39] [BCDA 10 23 40]
	[ABCD 13 4 41] [DABC 0 11 42] [CDAB 3 16 43] [BCDA 6 23 44]
	[ABCD 9 4 45] [DABC 12 11 46] [CDAB 15 16 47] [BCDA 2 23 48]
	*/
	A = NewMD5(A, B, C, D, Md5_3, pData[5], CS_64[32], CS3_1);
	D = NewMD5(D, A, B, C, Md5_3, pData[8], CS_64[33], CS3_2);
	C = NewMD5(C, D, A, B, Md5_3, pData[11], CS_64[34], CS3_3);
	B = NewMD5(B, C, D, A, Md5_3, pData[14], CS_64[35], CS3_4);

	A = NewMD5(A, B, C, D, Md5_3, pData[1], CS_64[36], CS3_1);
	D = NewMD5(D, A, B, C, Md5_3, pData[4], CS_64[37], CS3_2);
	C = NewMD5(C, D, A, B, Md5_3, pData[7], CS_64[38], CS3_3);
	B = NewMD5(B, C, D, A, Md5_3, pData[10], CS_64[39], CS3_4);

	A = NewMD5(A, B, C, D, Md5_3, pData[13], CS_64[40], CS3_1);
	D = NewMD5(D, A, B, C, Md5_3, pData[0], CS_64[41], CS3_2);
	C = NewMD5(C, D, A, B, Md5_3, pData[3], CS_64[42], CS3_3);
	B = NewMD5(B, C, D, A, Md5_3, pData[6], CS_64[43], CS3_4);

	A = NewMD5(A, B, C, D, Md5_3, pData[9], CS_64[44], CS3_1);
	D = NewMD5(D, A, B, C, Md5_3, pData[12], CS_64[45], CS3_2);
	C = NewMD5(C, D, A, B, Md5_3, pData[15], CS_64[46], CS3_3);
	B = NewMD5(B, C, D, A, Md5_3, pData[2], CS_64[47], CS3_4);

	//第4轮：
	/*
		[ABCD 0 6 49] [DABC 7 10 50] [CDAB 14 15 51] [BCDA 5 21 52]
		[ABCD 12 6 53] [DABC 3 10 54] [CDAB 10 15 55] [BCDA 1 21 56]
		[ABCD 8 6 57] [DABC 15 10 58] [CDAB 6 15 59] [BCDA 13 21 60]
		[ABCD 4 6 61] [DABC 11 10 62] [CDAB 2 15 63] [BCDA 9 21 64]
	*/

	A = NewMD5(A, B, C, D, Md5_4, pData[0], CS_64[48], CS4_1);
	D = NewMD5(D, A, B, C, Md5_4, pData[7], CS_64[49], CS4_2);
	C = NewMD5(C, D, A, B, Md5_4, pData[14], CS_64[50], CS4_3);
	B = NewMD5(B, C, D, A, Md5_4, pData[5], CS_64[51], CS4_4);

	A = NewMD5(A, B, C, D, Md5_4, pData[12], CS_64[52], CS4_1);
	D = NewMD5(D, A, B, C, Md5_4, pData[3], CS_64[53], CS4_2);
	C = NewMD5(C, D, A, B, Md5_4, pData[10], CS_64[54], CS4_3);
	B = NewMD5(B, C, D, A, Md5_4, pData[1], CS_64[55], CS4_4);

	A = NewMD5(A, B, C, D, Md5_4, pData[8], CS_64[56], CS4_1);
	D = NewMD5(D, A, B, C, Md5_4, pData[15], CS_64[57], CS4_2);
	C = NewMD5(C, D, A, B, Md5_4, pData[6], CS_64[58], CS4_3);
	B = NewMD5(B, C, D, A, Md5_4, pData[13], CS_64[59], CS4_4);

	A = NewMD5(A, B, C, D, Md5_4, pData[4], CS_64[60], CS4_1);
	D = NewMD5(D, A, B, C, Md5_4, pData[11], CS_64[61], CS4_2);
	C = NewMD5(C, D, A, B, Md5_4, pData[2], CS_64[62], CS4_3);
	B = NewMD5(B, C, D, A, Md5_4, pData[9], CS_64[63], CS4_4);

	//最后要累加
	pMd5[0] += A;
	pMd5[1] += B;
	pMd5[2] += C;
	pMd5[3] += D;
}
//将生成的新MD5值，存进文件中
void Save(char* filename)
{
	//打印生成的MD5码
	for (int i = 0; i < 16; i++)
	{
		if(md5[i] <= 0x0f)
			printf("0%x", md5[i]);
		else
			printf("%x", md5[i]);
	}
		
	//打开文件
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, filename, "w");	//只写："w"
	if (0 != res || NULL == pFile)
		return;
	//写入文件
	for (int i = 0; i < 16; i++)
	{
		if(md5[i] <= 0x0f)
			fprintf(pFile, "0%x", md5[i]);	//让0也打印，就不会出现只打印 如 e 之类的 就会打印 0e 所有小于0f 的都是单位数，都加上
		else
			fprintf(pFile, "%x", md5[i]);	//fprintf的优点：格式化写入，控制写入格式，不会出现乱码问题
	}
		
	//关闭文件
	fclose(pFile);
}