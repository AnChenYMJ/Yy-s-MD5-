#include <stdio.h>
#include <math.h>

#define ZU 64		//MD5�У�ÿ�����64�ֽ�

//��ȡ����λ��
long g_lDataBytes = 0;

//��ȡ�ļ��ֽ���
void GetLen(char* filename);
void GetData(char* filename);

//��������Ҫ�ĳ������������õ���֪ʶ��
	//һ��4������������
unsigned char md5[16] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };
unsigned int* pMd5 = (unsigned int*)md5;	//pMd5[0] pMd5[1] pMd5[2] pMd5[3]	��unsigned int�����͸պ�4�ֽ�Ϊ��λ�����±����ʹ�ö�Ӧ�����ˣ�
	//����64Ԫ�س�����
unsigned int CS_64[64];
void Init64(void);		//�������,
	//����16Ԫ�س����飨ת�����̺�����(�̶������֣�������Ĳ�������ĵ�δ˵������ѧ��Ū�ã������ȱ�ܣ�֪����ô�þͺ�)
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

//�����㷨
	//����ָ��(����������) �����Ϳ����ú��������֣�ָ�룩����ͬ���������͵ĺ�����
typedef unsigned int (*pFun)(unsigned int, unsigned int, unsigned int);
	//�ĸ��������㷨
unsigned int Md5_1(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_2(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_3(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_4(unsigned int x, unsigned int y, unsigned int z);
	//����1���µ�MD5ֵ(�����㷨����)
unsigned int NewMD5(unsigned int md1, unsigned int md2, unsigned int md3, unsigned int md4, pFun Fun, unsigned int data, unsigned int cs64, unsigned int cs16);
	//��ȡMD5ֵ
void GetMD5(char data[ZU]);

//�����ɵ���MD5ֵ������ļ���
void Save(char* filename);


//�����������������ڳ����������֮�࣬ͨ�õ�ֻ��ǰ���������������ݣ�����������VS���������ģ����һЩ����������Ϣ���ɴ�ӡ�鿴��
int main(int argc, int argv[], int argd[])
{
	//���ݲ����ķ�ʽ��ָ���ǳ��򣬼���������
	if (0)
	{
		//VS���������еģ��ɲ鿴��������Ŀ¼��������������֧�ֲ�ʶ��
		printf(argd[0]);
		putchar('\n');
		printf(argd[5]);
		putchar('\n');
		printf(argd[9]);
		putchar('\n'); putchar('\n');

		//����1������  ����2�����ݾ�����ʲô
		printf("%d \n", argc);		//δ���ݵ������Ĭ���� 1 ��
		printf(argv[0]);	//Ĭ���Ǳ������exe�ļ�����ӡ��Ҳ���Ǳ�����exe��·��
		putchar('\n'); putchar('\n');

		//�򱾳��򴫵ݲ����ķ�ʽ  ���д��ݣ�4�֣�1��ϵͳcmd	2���Ҽ�--�򿪷�ʽ	  3���ļ��϶�  4��˫��	���ʼ�����ϸ���⣩
		//���Դ��ݣ�1��
		printf(argv[0]);			//�����о��൱��һ�������ˣ��ñ������exe����ӡ��һ���ļ��������Ϣ
		putchar('\n');
		printf(argv[1]);			//�������ǵ�ַ���ֱ�Ϊ ������exe����Ҫ���ݵ��ļ� �����ĵ�ַ 
	}
	
	//64�����ĳ�ʼ��		�������ʼ����Ȼ�������Ϊ����MD5Ҫ�õ��������
	Init64();
	//��ȡ�ļ��ֽ���		�õ��Դ���ָ��һ���ɱ༭�ļ�������
	GetLen(argv[1]);
	printf("�ļ����ֽ�����%d \n", g_lDataBytes);	//�ϵ���� ����  �ƺ�����ռ���ֽ����ܸ�
	//��ȡ�ļ����ݽ���MD5����
	GetData(argv[1]);
	//�����ɵ���MD5ֵ������ļ���,����ӡ
	Save(argv[2]);

	system("pause > 0");
	return 0;
}

void GetLen(char* filename)
{
	

	//���ļ�
	FILE* pFile = NULL;									//�൱���ļ�ָ�룬ָ���ļ��ڵ�λ��
	errno_t res = fopen_s(&pFile, filename, "r");		//��1������ָ�� ��2���ļ��� ��3���򿪵Ĳ�����ʽ��r��ֻ��  ����ֵ��int���͵ı��� ����0�����������0���ִ���
	if (0 != res || NULL == pFile)						//������
		return;
	//�ļ�ָ�������ļ���β
	fseek(pFile, 0, SEEK_END);							//��1���ļ�ָ�� ��2������λ�� ��3�����λ�� ����0, SEEK_END �����������ļ���βΪͷ���±�0��λ�ã�������ĩβ�ַ�����һλ ����eof��λ�ã��ļ���β��ʶ��
														//���λ����3�֣�SEEK_SET �ļ�ͷ������λ��Ϊ6�Ļ������������ļ����±�Ϊ6���ַ�����λ��
														//SEEK_CUR����ǰλ�� �����ļ�ָ�뵱ǰ��ָ��λ�ã�����λ��+ - ���ٿ�ָ������ļ�ָ��Ҳ���ǰ��������
	//��ȡλ��											//��ָ���ļ���βeof��λ�ã����Դ�ʱ�±� ��ֵ�ϵ��� �ļ��ֽ���
	g_lDataBytes = ftell(pFile);						//MSDN�ϻ���һ���÷������÷�һ�������ļ�С��2G����������ִ�Լ����20�ڣ� ��һ�����ڴ���2G����� 
														//_int64 _ftelli64 ������һ�����ڴ���2G����� �����������飬40��4G���ң�
	//�ر��ļ�
	fclose(pFile);
}
void GetData(char* filename)
{
	//���� ������ж��ٸ� �ļ�ĩβ����64�ֽڵ����ж����ֽ�
	int man = g_lDataBytes / ZU;		//������װ64�ֽڣ�����������
	int fanil = g_lDataBytes % ZU;		//�����ļ�ĩβ����64�ֽڵ���

	//���ļ�
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, filename, "r");
	if (0 != res || NULL == pFile)
		return;
	//����Ĵ���
	for (int i = 0; i < man; i++)
	{
		//���ζ�ȡ�ļ��е����ݣ���64�ֽ�Ϊ��λ��һ��
		char str[ZU] = { 0 };
		fread(str, 1, ZU, pFile);		//�ļ�ָ��pFile��ʼ�����ݶ�������str �У� �����٣���2 ��3 �ĳ˻��������������Ի�λ�ã�����ν�ģ�ֻ��Ҫ�˻�

		//MD5���㣨�õ�ǰ�������������ݣ�
		GetMD5(str);
	}
	//1. ĩβ����64���� x > 0  && x <= 56 (xΪ������ֽ���)	��ΪҪ��8�ֽ���������������64-8 == 56
	if (fanil > 0 && fanil < 56)
	{
		//���ζ�ȡ�ļ��е�����
		char str[ZU] = { 0 };
		int count = fread(str, 1, ZU, pFile);	//����ֵ����¼���������ֽ���
		//�������8�ֽڿռ䣬��ֵ�洢���������ֽ�����
		*(unsigned long long*)(str + 56) = g_lDataBytes * 8;	//str�������֣����Ǹÿռ��׵�ַ�� +56�����ĩβ8�ֽڿռ���׵�ַ�ˣ�
																//��ĩβ8�ռ俴������ǿת��һ���������ͣ��洢��������
																//g_lDataBytes����һ�������У���¼��λ�ã���Ԫ�ظ����� *8 ��õ����ļ�����λ��
		//���� 10000....  ĩβ���飬βԪ��������8�ֽ��м��п���Ļ�����10000....����䣨��λ��
		if (count < 56)
		{
			str[count] = 0x80;	//count�±�λ�ÿ�ʼ����Ҫ����λ�ÿ�ʼ��һ���ֽ�8λ��������1000 0000 ʮ������Ϊ0x80 ������д1����Ϊ�������ֽھͲ���1��ͷ�ˣ��˺���ֽ���0
			for (int j = count + 1; j < 56; j++)	//ע��߽磬�˴�count�����±꣬���� =56
				str[j] = 0;
		}
		////��ӡ����һ��		(ĩβ8λ��Ϊ���ϵ���һ�𣬰��ֽ�����ӡ���Ͳ�ֱ�۵�֪���������֣�������������ӡ8�ֽ�)
		//for (int n = 0; n < 64; n++)
		//	printf("%c  %d\n", str[n], str[n]);
		////���Դ�ӡĩβ8�ֽ�
		//printf("%d \n", *(unsigned long long*)(str + 56));

		//���������MD5����
		GetMD5(str);
	}
	//2.ĩβ����������ʱ�����
	else if (0 == fanil)
	{
		//��������һ��ռ�(64�ֽ�)
		char str[ZU] = { 0 };
		//ĩβ8�ֽڼ�¼������
		*(unsigned long long*)(str + 56) = g_lDataBytes * 8;
		//���±�0��ʼ��ĩβ8�ֽڣ���λ��� 10000000...
		str[0] = 0x80;
		for (int j = 1; j < 56; j++)
			str[j] = 0;
		//���޸ĵ��Ե�txt�ļ����ݣ�ʹ���ֽ���Ϊ64�ı����������Ա��δ��루��ӡ���ԣ�

		//��һ�����MD5����
		GetMD5(str);
	}
	//3.ĩβ���� > 56 && < 64 �ֽ�ʱ
	else if (fanil >= 56 && fanil < 64)
	{
		//������
		char str[ZU] = { 0 };
		int count = fread(str, 1, ZU, pFile);
		//������ĩβ��λ���10000000...
		str[count] = 0x80;
		for (int j = count + 1; j < 64; j++)
			str[j] = 0;
		//�������MD5�㷨
		GetMD5(str);

		//��������һ��64�ֽڿռ�
		char strNew[ZU] = { 0 };	//�˲����൱��ÿ���ֽڶ������0(��һ���Ѿ���1��ͷ���ˣ���������λȫ��0����)
		//������Ŀռ�ĩβ8�ֽڴ洢������
		*(unsigned long long*)(strNew + 56) = g_lDataBytes * 8;
		//�������MD5�㷨
		GetMD5(strNew);

	}

	//�ر��ļ��������ˣ�
	fclose(pFile);
}
void Init64(void)
{
	//�����ļ��㷽ʽ��4294967296����abs(sin(i))��õ���ֵ���������֣�i�ǻ��ȣ�abs�Ǻ����������ֵ����sin�����ң�����Ҫ<math.h>��
	for (int i = 1; i <= 64; i++)
	{
		CS_64[i - 1] = (unsigned int)(4294967296 * fabs(sin(i)));	//(unsigned int)��ȡ�����ã�sin F12��ȥ����֪i��double���ͣ���absֻ�Զ�int������Ч
																	//��fabs��Ը�����double��Ч���ʲ���fabs�������ǿת�����Σ�����ȡ����������
		//��ӡ����
		//printf("%x ", CS_64[i - 1]);
	}
		
}
//�ĸ��������㷨	(�����㷨���밴��˳���Ӧ����)��GetMD5�ĺ�����ʹ�ã�
unsigned int Md5_1(unsigned int x, unsigned int y, unsigned int z)
{
	return ((x & y) | ((~x) & z));	//& ��λ��   | ��λ��   ~ ��λȡ��
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
//����1���µ�MD5ֵ����ϸ�������ܿ��ʼǣ�
unsigned int NewMD5(unsigned int md1, unsigned int md2, unsigned int md3, unsigned int md4, pFun Fun, unsigned int data, unsigned int cs64, unsigned int cs16)
{
	//��1��
	unsigned int md5_New = md1 + Fun(md2, md3, md4) + data + cs64;	//д���̶��ģ����������˸���
	//��2��
	md5_New = (md5_New << cs16) | (md5_New >> (32 - cs16));
	//��3��
	md1 = md2 + md5_New;
	//���ؼ�����
	return md1;		//����ʱ���ᰴ��4����������������˳����ֻ��Ŵ��ݸ�md1����ÿ�������md1�ǲ�ͬ��
}
//��ȡMD5ֵ
void GetMD5(char data[ZU])
{
	//������ת����4�ֽ�ΪһС�ε���������,64�ֽھͷֳ���16��
	unsigned int* pData = (unsigned int*)data;	

	//��ֵ��¼��4���������������������˳��
	unsigned int A = pMd5[0];
	unsigned int B = pMd5[1];
	unsigned int C = pMd5[2];
	unsigned int D = pMd5[3];

	//��1��16�Σ��й̶�˳�򣬲��ܳ���,����ֵ�����Ǹ�ֵ����1��
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

	//��2�֣�	ע��ο�����Ĳ�6 ��ʵ��ʵ�ʺ�����ĩβ������˳���е���
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

	//��3�֣�
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

	//��4�֣�
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

	//���Ҫ�ۼ�
	pMd5[0] += A;
	pMd5[1] += B;
	pMd5[2] += C;
	pMd5[3] += D;
}
//�����ɵ���MD5ֵ������ļ���
void Save(char* filename)
{
	//��ӡ���ɵ�MD5��
	for (int i = 0; i < 16; i++)
	{
		if(md5[i] <= 0x0f)
			printf("0%x", md5[i]);
		else
			printf("%x", md5[i]);
	}
		
	//���ļ�
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, filename, "w");	//ֻд��"w"
	if (0 != res || NULL == pFile)
		return;
	//д���ļ�
	for (int i = 0; i < 16; i++)
	{
		if(md5[i] <= 0x0f)
			fprintf(pFile, "0%x", md5[i]);	//��0Ҳ��ӡ���Ͳ������ֻ��ӡ �� e ֮��� �ͻ��ӡ 0e ����С��0f �Ķ��ǵ�λ����������
		else
			fprintf(pFile, "%x", md5[i]);	//fprintf���ŵ㣺��ʽ��д�룬����д���ʽ�����������������
	}
		
	//�ر��ļ�
	fclose(pFile);
}