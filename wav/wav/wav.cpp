// wavread.cpp: 定义控制台应用程序的入口点
#include <stdafx.h>
#include <iostream>
#include <fstream>
#include <typeinfo.h>
//#include "AudioFile.h"
using namespace std;
struct wav_struct {
	unsigned short channel_num;		// 声道数
	unsigned int frequency;			// 采样频率
	unsigned int byte_rate;			// 比特率，frequency * channel_num * bit_per_sample /8
	unsigned short bit_per_sample;	// 采样位数 8位或者16 24等
	unsigned int file_size;			// 文件大小
	unsigned int data_size;			// 实际数据大小
	unsigned char *data;			// 实际存储的数据
};

int main(void)
{
	// 换电脑时需要更新解决方案(清理+重定向项目)
	const string file_name = "D:\\Project\\effect_c\\audioData\\Long Live.wav";		// 需要使用绝对地址 work-pc
	//const string file_name = "D:\\WorkSpace\\Matlab\\GPE\\Audio Data\\Long Live.wav";		// 需要使用绝对地址 home-pc


	ifstream fp;																	// 输入时 ifstream
	wav_struct wav;
	//AudioFile<double> audiofile;	//无法使用该库
	//audiofile.load(file_name);
	//audiofile.printSummary();

	unsigned char *p;
	fp.open(file_name, ios::in | ios::binary);										// 读取文件 try except
	if (!fp.is_open()) {
		cout << "Can not open it." << endl;
		//exit(0);
	}

	fp.seekg(0, ios::end);						// seek get
	wav.file_size =  fp.tellg();				// 从后往前，输出的是整个文件的大小(字节形式)
	fp.seekg(0x16);
	fp.read((char*)&wav.channel_num, 2);		// 声道
	fp.seekg(0x18);	// offset 24 is num channels
	fp.read((char *)&wav.frequency,4);			// 采样频率
	fp.read((char*)&wav.byte_rate, 4);			// 比特率
	fp.seekg(0x22);
	fp.read((char*)&wav.bit_per_sample, 2);		// 采样位数	
	fp.seekg(0x28);
	fp.read((char*)&wav.data_size, 4);

	wav.data = new unsigned  char[wav.data_size];// 开辟数据空间接收
	for (int c = 0; c < wav.data_size; c++) {
		wav.data[c] = '0';
	}

	fp.seekg(0x2c);								// 44开始是data域的东西
	cout << "cur1:" << fp.tellg() << endl;
	fp.read( (char*)wav.data, wav.data_size );
	cout << "cur2:" << fp.tellg() << endl;
	cout << "data_size(字节):" << wav.data_size << endl;
	cout << sizeof(wav.data) << endl;
	cout << "start here" << endl;

	for (int n = 0; n < 16; n=n+2) {
		unsigned long value = 0;
		long temp_value = 0;
		signed long flag = 1;
		//if ((wav.data[n + 1] & 0x80) == 0x01) {	// 这里的判断有误， 需要把0x01换成0x80 或者右移8位
		if ((wav.data[n + 1] >>7) == 1) {	// 这里的判断有误， 需要把0x01换成0x80 或者右移7位

			//value =   0x8000 | 0x7fff&( (~wav.data[n + 1]) << 8 ) | ~wav.data[n] | 0x01 ;// 2字节负数从补码求原码
			value = wav.data[n] | wav.data[n + 1] << 8;
			temp_value = value	- 65536;
			//cout << temp_value << "=" << endl;			// 没有识别出负数
			flag = -1;
			cout <<  "get" << endl;
		}
		else {
			value = wav.data[n] | wav.data[n + 1] << 8;
			temp_value = value;
		}
		//double true_value = double(temp_value)-65536;
		//test//printf("%x\n", wav.data[n + 1]>>7);
		printf("%x %x|| %f \n", wav.data[n + 1], wav.data[n],(double)temp_value*0.000030518);
		//unsigned long value2 = wav.data[n] + wav.data[n + 1]*256;
		//cout << value << "-" << value2 << endl;

		//printf("%f ", (double)value / (double)(32768.0));
		//cout << value << endl;
		////value = ((value & 0x8000) == 1) ? 0x8000| ~(0x7fff & value) | 0x01 : value;// 从补码还原成原码,注意char左右1字节，因此会被截断 需要重新设计
		//unsigned int true_value = value;
		////printf("%0x-%f\n", value,(double)value / (double)(32768.0));
		//cout << (double)true_value / (double)(32768.0) << endl;
	}
	/*
	for (unsigned long i = 0; i<wav.frequency; i = i + 2)
	{
		//右边为大端
		unsigned long data_low = wav.data[i];
		unsigned long data_high = wav.data[i + 1];
		double data_true = data_high * 256 + data_low;
		//printf("%d ",data_true);
		long data_complement = 0;
		//取大端的最高位（符号位）
		int my_sign = (int)(data_high / 128);
		//printf("%d ", my_sign);
		if (my_sign == 1)
		{
			data_complement = data_true - 65536;
		}
		else
		{
			data_complement = data_true;
		}
		//printf("%d ", data_complement);
		double float_data = (double)(data_complement / (double)32768);
		printf("%f ", float_data);

		//data_normalization[i] = (char)float_data;
		//printf("%f ", data_normalization[i]);     
		//bitset<8>lsc_high(data_high);
		//string high_binary = lsc_high.to_string();        
		//bitset<8> low_binary (low_data);            
	}
	*/
	
	// end here
	

	//cout <<"see charLen:"<< sizeof(char) << endl;
	//cout << "channel:"<<wav.channel_num << endl;
	//cout << "frequency:" << wav.frequency << endl;
	//cout << "byte_rate:" << wav.byte_rate << endl;
	//cout << "bit_per_sample:" << wav.bit_per_sample << endl;
	//cout << "data_size:" << wav.data_size << endl;


	//cout << sizeof(unsigned short) << endl;
	//cout << "cur:"<<fp.tellg() << endl;
	//fp.read((char*)&temp, sizeof(temp));			// 读取之后会自动将指针移动已读字节数
	//cout << "cur:" << fp.tellg() << endl;
	//fp.read((char*)&temp, 4);
	//fp.seekg(0);
	//fp.read(s, 4);
	//cout << "ChunkID-" << s[2] << endl;
	//cout << "Get:" << temp << endl;
	//
	//    fp.read(temp,sizeof(unsigned char));
	//    fp >> noskipws;
	//    while (fp.good()){
	//        fp >> c;
	//        cout << c <<"";
	//    }
	//cout << "see the id" << endl;
	//    cout<< typeid(WAV.frequency).name()<<endl;
	system("pause");
	return 0;
}
