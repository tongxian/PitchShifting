// wavread.cpp: 定义控制台应用程序的入口点
#include <stdafx.h>
#include <iostream>
#include <fstream>
#include <typeinfo.h>
#include "fftw3.h"
#include <math.h>
#define PI 3.1415926535897
using namespace std;

class wavfile
{
public:
unsigned short cahnnel;			// 声道数
unsigned int frequency;			// 采样频率
unsigned int byte_rate;			// 比特率，frequency * cahnnel * bit_per_sample /8
unsigned short bit_per_sample;	// 采样位数 8位或者16 24等
unsigned int file_size;			// 文件大小
unsigned long data_size;		// 实际数据文件大小（字节形式）
unsigned char *original_data;	// 实际存储的数据 （从字节形式进行转化)
double *norm_data;				// 归一化数据
double *data;					// default output is mono data
unsigned long len;				// 实际点数,data_size/(bit_per_sample/8) /channel
double duration;				// 持续时间(second)
bool is_open;
wavfile();
~wavfile();
int read(string file_neme);		// read the wav file
void show();					// show wav file message
int write(string file_name);	// write data to wav file
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
	for (unsigned long win_len = 0; win_len < this->data_size; win_len = win_len + 2) {
		long temp_value = 0;
		if ((this->original_data[win_len + 1] >> 7) == 1) {								// 判断正负数,x86系统中是小端模式，低位低地址，高位高地址，两字节
			temp_value = this->original_data[win_len] | (this->original_data[win_len + 1] << 8) - 65536;						
		}
		else {
			temp_value = this->original_data[win_len] | this->original_data[win_len + 1] << 8;		// 正数
		}
		this->norm_data[win_len / 2] = (double)temp_value / 32768.0;							// 归一化

	}

	// output data

	if (this->cahnnel > 1) {
		this->data = new double[this->data_size / 4];
		long count = 0;
		for (long win_len = 0; win_len < this->data_size / 4; win_len += 2) {
			this->data[count++] = (this->norm_data[win_len] + this->norm_data[win_len + 1]) / 2.0;
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

int wavfile::write(string file_name)
{	
	ofstream fp;
	fp.open(file_name, ios::out || ios::binary);
	//fp.write()
	return 0;
}

int main(void)
{
	// 换电脑时需要更新解决方案(清理+重定向项目SDK) 
	const string file_name = "D:\\Project\\effect_c\\audioData\\Long Live.wav";				// 需要使用绝对地址 work-pc
	//const string file_name = "D:\\WorkSpace\\Matlab\\GPE\\Audio Data\\Long Live.wav";		// 需要使用绝对地址 home-pc
	wavfile wav;
	wav.read(file_name);
	wav.show();
	// fftw test


	const int win_len = 256;								//win_len ,fft number
	fftw_complex *fft_in, *fft_out,*ifft_in,*ifft_out;
	fftw_plan fft,ifft;
	fft_in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*win_len);
	fft_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*win_len);
	ifft_in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*win_len);
	ifft_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*win_len);
	fft = fftw_plan_dft_1d(win_len, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);// FFT
	ifft = fftw_plan_dft_1d(win_len, ifft_in, ifft_out, FFTW_BACKWARD, FFTW_ESTIMATE);// IFFT


	double input[win_len];
	unsigned int ana_len = 64;					// analysis length
	unsigned int syn_len = 90;					// synthesis length
	unsigned int ana_count = ( wav.len - (win_len-ana_len) ) / ana_len;
	// fft 相关
	double real[win_len];
	double imag[win_len];
	double angle[win_len] = {0};
	double angle_pre[win_len];
	bool is_firstTime = true;
	double omega[win_len];
	double angle_syn[win_len];
	for (int n = 0; n < win_len; n++) {
		omega[n] = 2 * PI*double(n) / double(win_len);
	}
	// 分帧
	ana_count = 2;// test
	ofstream fo;// 输出值txt进行查看 test
	fo.open("mag.txt", ios::out | ios::app);// test



	for (int i=0,offset=0; i < ana_count; i++,offset++) {			//	以ana_len 为间隔向前移动，取出win_len点数据
		for (int n = 0; n < win_len; n++) {
			input[n] = wav.data[offset*ana_len +n];
			fft_in[n][0] = input[n];
			fft_in[n][1] = 0;
		}
		fftw_execute(fft);
		for (int n = 0; n < win_len; n++) {
			real[n] = fft_out[n][0];
			imag[n] = fft_out[n][0];
			angle_pre[n] = angle[n];
			angle[n] = atan2(real[n], imag[n]);// 求出相位
			// phase unwrap
			double delta = (angle - angle_pre) - omega[n] * ana_len; // phi
			double delta_phi = delta - round(delta / (2 * PI)) * (2 * PI);// phi 
			double y_unwrap = delta_phi/ana_len + omega[n];// w

			if (is_firstTime) {
				angle_syn[n] = angle[n];
				is_firstTime = false;
			}
			else {
				angle_syn[n] = angle_syn[n] + y_unwrap * syn_len;
			}
			// prepare for ifft
			fft_in[n][0] = real[n];
			fft_in[n][1] = angle_syn[n];
		}
		fo << endl;
	// 加窗
	}

	// 运算开始
	/*
	
	for (int count = 0; count < win_len; count++) {					// input data
		input[count] = wav.data[count];
	}
	for (int win_len = 0; win_len < win_len; win_len++) {
		in[win_len][0] = double(input[win_len]);					// 实部
		in[win_len][1] = 0;											// 虚部
	}
	fftw_execute(p);												// 调用fft
	fftw_destroy_plan(p);

	double *mag = new double[win_len];
	double *complex = new double[win_len];
	for (int win_len = 0; win_len < win_len; win_len++) {
		mag[win_len] = out[win_len][0];
		complex[win_len]= out[win_len][1];// 0：实数部分，1：复数部分
	}





	// 输出fft数据
	ofstream fo;
	fo.open("mag.txt", ios::out|ios::app);

	for (int win_len = 0; win_len < win_len; win_len++) {

		fo << mag[win_len] << endl;
	}
	
	*/
	// 释放资源
	fftw_free(fft_in);
	fftw_free(fft_out);
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