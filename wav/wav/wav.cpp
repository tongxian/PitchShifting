// wavread.cpp: 定义控制台应用程序的入口点
#include <stdafx.h>
#include <iostream>
#include <fstream>
#include <typeinfo.h>
using namespace std;

class wavfile
{
public:
unsigned short cahnnel;			// 声道数
unsigned int frequency;			// 采样频率
unsigned int byte_rate;			// 比特率，frequency * cahnnel * bit_per_sample /8
unsigned short bit_per_sample;	// 采样位数 8位或者16 24等
unsigned int file_size;			// 文件大小
unsigned int data_size;			// 实际数据文件大小（字节形式）
unsigned char *original_data;	// 实际存储的数据 （从字节形式进行转化)
double *norm_data;				// 归一化数据
double *data;					// default output is mono data
unsigned long len;				// 实际点数,data_size/(bit_per_sample/8) /channel
double duration;				// 持续时间(second)

bool is_open;
wavfile();
~wavfile();
int read(string file_neme);
void show();
private:

};

wavfile::wavfile()
{
	// 构造函数 相当于初始化

}

wavfile::~wavfile()
{
	// 析构函数 释放资源
	this->is_open = false;
}

int wavfile::read(string file_neme)
{	
	ifstream fp;																	// 输入时 ifstream
	fp.open(file_neme, ios::in | ios::binary);										// 读取文件 try except
	if (!fp.is_open()) {
		cout << "Can not open it." << endl;
		fp.close();
		return false;
	}
	fp.seekg(0, ios::end);						// seek get
	this->file_size = fp.tellg();					// 从后往前，输出的是整个文件的大小(字节形式)
	fp.seekg(0x16);
	fp.read((char*)&this->cahnnel, 2);		// 声道
	fp.seekg(0x18);								// offset 24 is num channels
	fp.read((char *)&this->frequency, 4);			// 采样频率
	fp.read((char*)&this->byte_rate, 4);			// 比特率
	fp.seekg(0x22);
	fp.read((char*)&this->bit_per_sample, 2);		// 采样位数	
	fp.seekg(0x28);
	fp.read((char*)&this->data_size, 4);

	this->original_data = new unsigned  char[this->data_size];// 开辟数据空间接收

	fp.seekg(0x2c);								// 44开始是data域的东西
	fp.read((char*)this->original_data, this->data_size);

	this->norm_data = new double[this->data_size / 2];
	for (int n = 0; n < this->data_size; n = n + 2) {
		long temp_value = 0;
		if ((this->original_data[n + 1] >> 7) == 1) {									// 这里的判断有误， 需要把0x01换成0x80 或者右移7位
																					//printf("%x", (0x8000 | 0x7fff & ((~wav.original_data[n + 1]) << 8) | ~wav.original_data[n] | 0x01) );
																					//value =   0x8000 | 0x7fff&( (~wav.original_data[n + 1]) << 8 ) | ~wav.original_data[n] | 0x01 ;// 2字节负数从补码求原码-求法错误
			temp_value = this->original_data[n] | (this->original_data[n + 1] << 8) - 65536;						// 负数
		}
		else {
			temp_value = this->original_data[n] | this->original_data[n + 1] << 8;								// 正数
		}
		this->norm_data[n / 2] = (double)temp_value / 32768.0;											// 归一化

	}

	// output data

	if (this->cahnnel > 1) {
		this->data = new double[this->data_size / 4];
		long count = 0;
		for (long n = 0; n < this->data_size / 4; n += 2) {
			this->data[count++] = (this->norm_data[n] + this->norm_data[n + 1]) / 2.0;
		}
	}
	else
		this->data = this->norm_data;
	fp.close();
	this->len = this->data_size / 4;
	this->duration = this->len / this->frequency;
	this->is_open = true;
	return true;
}

void wavfile::show()
{
	if (this->is_open) {
		cout << "The Wav file message:" << endl
			<< "Frequency	: " << this->frequency <<" Hz"<<endl
			<< "Bit_per_sample	: " << this->bit_per_sample <<" bps"<<endl
			<< "Size	: "<<this->len<<" samples"<<endl
			<< "Duration: " << this->duration << " seconds" << endl;
	}
	else
		cout << "Have not open any wav file." << endl;
}
int main(void)
{
	// 换电脑时需要更新解决方案(清理+重定向项目SDK)
	const string file_name = "D:\\Project\\effect_c\\audioData\\Long Live.wav";		// 需要使用绝对地址 work-pc
	//const string file_name = "D:\\WorkSpace\\Matlab\\GPE\\Audio Data\\Long Live.wav";		// 需要使用绝对地址 home-pc
	wavfile wav;
	wav.read(file_name);
	cout << wav.len << endl;
	cout << wav.duration/60.0<<"s" << endl;
	wav.show();
	//for (int n = 0; n < wav.len; n++) {
	//	cout << wav.data[n] << endl;
	//}
	cout << "end here" << endl;
	system("pause");
	return 0;
}
/*
TODO：
	-1. 封装成类，方便调用
	2. 优化双声道数据读取
	3. 使用向量的形式进行操作

	已知BUG：
		1. 所换算出来的值比matlab中的值(浮点)少一半，不知为何，需要重新校验matlab中的解码方式，或者
			自己编写的解码方式，就目前来说影响不大。
	
*/