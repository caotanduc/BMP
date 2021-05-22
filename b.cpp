#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

#pragma pack(push, 1)
struct Header
{
	unsigned char _signature[2];
	unsigned int _size;
	unsigned int _reserved;
	unsigned int _offset;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DIB
{
	unsigned int _size;
	unsigned int _width;
	unsigned int _height;
	unsigned short int _planes;
	unsigned short int _bits_per_pixel;
	unsigned int _compression;
	unsigned int _image_size;
	unsigned int _x_pixels_per_m;
	unsigned int _y_pixels_per_m;
	unsigned int _colors_used;
	unsigned int _important_colors;
};
#pragma pack(pop)

int docFile(char *filename, Header &header, DIB &dib)
{
	ifstream f(filename, ios::binary);
	if (!f)
		return 0;
	f.read((char *)&header, sizeof(Header));
	if (header._signature[0] != 'B' || header._signature[1] != 'M')
	{
		f.close();
		return 0;
	}
	f.read((char *)&dib, sizeof(DIB));
	f.close();
	return 1;
}

void inThongTin(Header header, DIB dib)
{
	cout << endl;
	cout << "Header __information__" << endl;
	cout << "Signature: " << header._signature[0] << header._signature[1] << endl;
	cout << "File size: " << header._size << endl;
	cout << "Reserve: " << header._reserved << endl;
	cout << "Offset: " << header._offset << endl;

	cout << endl;
	cout << "DIb __information__" << endl;
	cout << "DIB size: " << dib._size << endl;
	cout << "Width: " << dib._width << endl;
	cout << "Height: " << dib._height << endl;
	cout << "Planes: " << dib._planes << endl;
	cout << "Bits per pixel: " << dib._bits_per_pixel << endl;
	cout << "Compression: " << dib._compression << endl;
	cout << "Image size: " << dib._image_size << endl;
	cout << "X pixels per M: " << dib._x_pixels_per_m << endl;
	cout << "Y pixels per M: " << dib._y_pixels_per_m << endl;
	cout << "Color used: " << dib._colors_used << endl;
	cout << "Important colors: " << dib._important_colors << endl;
}

int docDuLieu(char *filename, unsigned char *&pixels_data, Header header, DIB dib)
{
	ifstream f(filename, ios::binary);
	if (!f || (int)(dib._bits_per_pixel) < 8 || (int)(dib._bits_per_pixel) == 16)
		return 0;

	f.seekg(header._offset);
	int width = dib._width;
	int bytes = (int)(dib._bits_per_pixel) / 8;
	int n = width * bytes;
	int pad = (4 - n % 4) % 4;
	int size = dib._height * n;

	pixels_data = new unsigned char[size];
	for (int i = 0; i < dib._height; i++)
	{
		f.read((char *)(pixels_data + (i * n)), n);
		f.seekg((int)(f.tellg()) + pad);
	}
	f.close();
	return 1;
}

unsigned char *docTempData(char *filename, Header header, DIB dib)
{
	int size = header._offset - 54;
	unsigned char *x = new unsigned char[size];
	ifstream f(filename, ios::binary);
	f.seekg(54);
	f.read((char *)x, size);
	f.close();
	return x;
}

int ghiAnh(char *filename, unsigned char *pixels_data, unsigned char* x, Header header, DIB dib)
{
	ofstream f(filename, ios::binary);
	if (!f)
		return 0;
	f.write((char *)&header, sizeof(Header));
	f.write((char *)&dib, sizeof(DIB));
	if ((int)(dib._bits_per_pixel) == 8)
	{
		unsigned char color_table[1024];
		int l = 0;
		for (int i = 0; i < 1024; i++)
		{
			if (i % 4 == 0)
				color_table[i] = (unsigned char)(l);
			if (i % 4 == 1)
				color_table[i] = (unsigned char)(l);
			if (i % 4 == 2)
				color_table[i] = (unsigned char)(l);
			if (i % 4 == 3)
				color_table[i] = (unsigned char)(l++);
		}
		f.write((char *)color_table, 1024);
	}
	if ((int)(dib._bits_per_pixel) == 32)
		f.write((char *)x, header._offset - 54);

	f.seekp(header._offset);
	int bytes = (int)(dib._bits_per_pixel) / 8;
	int pad = (4 - (dib._width * bytes) % 4) % 4;
	unsigned char padding[3] = {0, 0, 0};
	for (int i = 0; i < dib._height; i++)
	{
		f.write((char *)(pixels_data + (i * dib._width * bytes)), dib._width * bytes);
		f.write((char *)padding, pad);
	}
	return 1;
}

int main()
{
	char* filename = (char*)"photo/anh_nhom.bmp";
	Header header;
	DIB dib;
	docFile(filename, header, dib);
	inThongTin(header, dib);
	unsigned char* pixels_data = NULL;
	unsigned char* temp = docTempData(filename, header, dib);
	docDuLieu(filename, pixels_data, header, dib);
	ghiAnh((char*)"output.bmp", pixels_data, temp, header, dib);
	delete[] pixels_data;
	pixels_data = NULL;
	return 0;
}