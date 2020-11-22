// Include libraries
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <cstdlib>

#define byte unsigned char

using namespace std;

byte encodeByte(byte txt, byte bmp, int count, int offset){
	/*
		Записываем 'count' бит из байта 'txt' в байт 'bmp' 
		справа налево со смещением 'offset' из байта 'txt'
		т.е. из одного txt байта собирается N bmp байт.
	*/
	
	txt = txt >> offset;
	byte mask = 1;
	for(int i=0;  i<count; i++){
		if ((txt & mask) == 1){
			if ((bmp & mask) == 0){
				// txt = 1, bmp = 0
				bmp = bmp|mask; 
			}
		}
		else{
			if ((bmp & mask) == 1){
				// txt = 0, bmp = 1
				bmp = bmp&(~mask);
			}
		}
		mask = mask << 1;
	}
	return bmp;
}

byte decodeByte(byte txt, byte bmp, int count, int offset){
	/*
		Записываем 'count' бит из байта 'а' в байт 'b' 
		справа налево со смещением 'offset' из байта 'a'
		т.е. из N bmp-байт собирается txt байт.
	*/
	byte mask = 1;
	mask = mask << offset;
	bmp = bmp << offset;
	for(int i=0;  i<count; i++){
		if ((txt & mask) == 1){
			if ((bmp & mask) == 0){
				// txt = 1, bmp = 0
				txt = txt&(~mask);
			}
		}
		else{
			if ((bmp & mask) == 1){
				// txt = 0, bmp = 1
				txt = txt|mask;
			}
		}
		mask = mask << 1;
	}
	return txt;
}

class BMP{
	
	private:
	
	public:
		
		unsigned int fileSize, picBeg, dataLength, capacity, width, height;
		
		byte *fHeader, *bHeader, *data;
		
		BMP(char* path){
			open(path);			
		}
		
		BMP(){
			fileSize = 0;
			picBeg = 0;
			capacity = 0;
		}
		
		void write(char* path){
			fstream file(path, std::fstream::out | std::fstream::binary);
			if (file.is_open()){
				
				char *signedfHeader = new char[14];
				signedfHeader = reinterpret_cast<char*>(fHeader);
				file.write(signedfHeader, 14);
				
				char *signedbHeader = new char[40];
				signedbHeader = reinterpret_cast<char*>(bHeader);
				file.write(signedbHeader, 40);
				
				char *signeddata = new char[fileSize-picBeg];
				signeddata = reinterpret_cast<char*>(data);
				file.write(signeddata, fileSize-picBeg);
				
			}else{
				cout << "cannot open file " << path;
			}
		}
		
		void open(char* path){
			
			fstream file(path, std::fstream::in | std::fstream::binary);
			
			if (file.is_open()){
				
				fHeader = new byte[14];
				bHeader = new byte[40];
				
				char *signedfHeader = new char[14];
				file.read(signedfHeader, 14);
				fHeader = reinterpret_cast<unsigned char*>(signedfHeader);

				fileSize = fHeader[5] << 24 |  fHeader[4] << 16 |  fHeader[3] << 8 |  fHeader[2];

				picBeg = fHeader[13] << 24 |  fHeader[12] << 16 |  fHeader[11] << 8 |  fHeader[10];
				
				char *signedbHeader = new char[40];
				file.read(signedbHeader, 40);
				bHeader = reinterpret_cast<unsigned char*>(signedbHeader);
				
				width = bHeader[4] << 24 |  bHeader[5] << 16 |  bHeader[6] << 8 |  bHeader[7];
				height = bHeader[8] << 24 |  bHeader[9] << 16 |  bHeader[10] << 8 |  bHeader[11];
				
				data = new byte[fileSize-picBeg];
				char *signeddata = new char[fileSize-picBeg];
				file.read(signeddata, fileSize-picBeg);
				data = reinterpret_cast<unsigned char*>(signeddata);
				
				getDataLength();
				capacity = fileSize - picBeg - 12;
			}
		}
		
		void getDataLength(){
			byte b[4];
			int i = 0, j= 0;
			while(i < 4){
				b[i] = decodeByte(b[i], data[j], 3, 0);
				j++;
				b[i] = decodeByte(b[i], data[j], 3, 3);
				j++;
				b[i] = decodeByte(b[i], data[j], 2, 6);
				j++;
				i++;
			}
			dataLength = data[3] << 24 |  data[2] << 16 |  data[1] << 8 |  data[0];	
		}
		
		void putDataLength(){
			byte b[5];
			b[3] = dataLength >> 24;
			b[2] = dataLength >> 16;
			b[1] = dataLength >> 8;
			b[0] = dataLength >> 0;
			int i = 0, j= 0;
			while(i < 4){
				data[i] = encodeByte(b[i], data[j], 3, 0);
				j++;
				data[i] = encodeByte(b[i], data[j], 3, 3);
				j++;
				data[i] = encodeByte(b[i], data[j], 2, 6);
				j++;
				i++;
			}
		}
};

class TXT{
	
	private:
	
	public:
		
		int size;
		byte* data;
		
		TXT(){
		}
		
		void createBuffer(int dataLength){
			data = (byte*)malloc(dataLength);
		}
		
		void read(char* path){
			size = 0;
			fstream file(path, std::fstream::in | std::fstream::binary);
			if (file.is_open()){
				file.seekg (0, std::ios::end);
				size = file.tellg();
				data = new byte[size];
				file.seekg (0, std::ios::beg);
				
				char *signeddata = new char(size);
				file.read(signeddata, size);
				data = reinterpret_cast<unsigned char*>(signeddata);

				ifstream fin(path); 
				char c;
				int g = 0;
				while(!fin.eof()) 
				{ 
					fin.get(c); 
					if(!fin.eof()) 
					cout << g << " char is " << c << '\n';
					g++;
				}
			}
			else{
				cout << "Error, cannot open file " << path << '\n';
			}
		}
		
		void write(char* path){
			fstream file(path, std::fstream::out | std::fstream::binary);
			if (file.is_open()){
				char *signeddata = new char(size);
				signeddata = reinterpret_cast<char*>(data);
				file.write(signeddata, size);
				
				file.close();
			}
			else{
				cout << "Error, cannot create file " << path << '\n';
				cout << "File \"result.txt\" will be created." << '\n';
				write();
			}
		}
		
		void write(){
			fstream file("result.txt", std::fstream::out | std::fstream::binary);
			if (file.is_open()){
				char *signeddata = new char(size);
				signeddata = reinterpret_cast<char*>(data);
				file.write(signeddata, size);
				
				file.close();
			}
			else{
				cout << "Error, cannot create file." << '\n';
			}
		}
	
};

void encode(TXT &txt, BMP &pic){
	
	if(txt.size*3 < pic.capacity){
		int i = 0, j = 12;
		while(i < txt.size){
			pic.data[j] = encodeByte(txt.data[i], pic.data[j], 3, 0);
			j++;
			pic.data[j] = encodeByte(txt.data[i], pic.data[j], 3, 3);
			j++;
			pic.data[j] = encodeByte(txt.data[i], pic.data[j], 2, 6);
			j++;
			i++;
		}
		pic.dataLength = txt.size*3;
		pic.putDataLength();
		cout << "txt.size is " << hex << txt.size << '\n';
		cout << "dataLength is " << hex << pic.dataLength << '\n';
	}else{
		cout << "file is too big, choose another *.bmp";
	}
	
}

void decode(TXT &txt, BMP &pic){
	
	int multiplier = 3;
	
	if(pic.dataLength%3!=0)
		cout << "WARNING: File might be encoded by other type of encoding.";
	txt.size = pic.dataLength/multiplier;
	txt.createBuffer(txt.size);
	int i = 0, j = 12;

	while(i < txt.size){
		txt.data[i] = decodeByte(txt.data[i], pic.data[j], 3, 0);
		j++;
		txt.data[i] = decodeByte(txt.data[i], pic.data[j], 3, 3);
		j++;
		txt.data[i] = decodeByte(txt.data[i], pic.data[j], 2, 6);
		j++;
		i++;
	}		
}

int main(int argc, char *argv[]){
	setlocale(LC_ALL, "Russian");
	string S = argv[1];
	cout << argv[1] << '\n';
	if(S == "encode"){
		cout << "encoded" << '\n';
		TXT *txt = new TXT();
		txt->read(argv[2]);
		BMP *bmp = new BMP(argv[3]);
		encode(*txt, *bmp);
		bmp->write(argv[4]);
	}
	else if(S == "decode"){
		TXT *txt = new TXT();
		BMP *bmp = new BMP(argv[2]);
		txt->createBuffer(bmp->dataLength);
		decode(*txt, *bmp);
		txt->write(argv[3]);
	}
}
