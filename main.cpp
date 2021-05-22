#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

#pragma pack(push, 1)
struct Header
{
	char signature[2];
	int _size;
	int _reserved;
	int _offset;
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
	if (header.signature[0] != 'B' || header.signature[1] != 'M')
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
	cout << "Signature: " << header.signature[0] << header.signature[1] << endl;
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

unsigned char *getPixelsData_1D_Array(char *filename, Header header, DIB dib)
{
	ifstream f(filename, ios::binary);
	if (!f)
	{
		cout << "Duong dan file khong hop le." << endl;
		return NULL;
	}
	int bytes = (int)(dib._bits_per_pixel) / 8;
	int width = dib._width;
	int height = dib._height;
	int pad = (4 - (bytes * width) % 4) % 4;
	int n = width * bytes;
	int size = n * height;
	unsigned char *pixels_data = new unsigned char[size];
	f.seekg(header._offset);
	unsigned char padding[3];
	for (int i = 0; i < height; i++)
	{
		f.read((char *)(pixels_data + (i * n)), n);
		int step = (int)(f.tellg()) + pad;
		f.seekg(step);
	}
	f.close();
	return pixels_data;
}

int convert_8bits(char *filename, unsigned char *&pixels_data, Header header, DIB dib)
{
	unsigned char *p_1D = getPixelsData_1D_Array(filename, header, dib);
	if (p_1D == NULL)
		return 0;
	int bytes = (int)(dib._bits_per_pixel) / 8;
	int width = dib._width;
	int height = dib._height;
	int pad = (4 - width % 4) % 4;
	int n = width + pad;
	int size = n * height;

	if (bytes == 1)
	{
		memcpy(pixels_data, p_1D, size);
	}
	else
	{
		pixels_data = new unsigned char[size];

		int l = 0;
		int step = bytes - 3;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width * bytes; j += bytes)
			{
				pixels_data[l] = p_1D[i * width * bytes + j + step] / (unsigned char)(3) + p_1D[i * width * bytes + j + 1 + step] / (unsigned char)(3) + p_1D[i * width * bytes + j + 2 + step] / (unsigned char)(3);
				l += 1;
			}
		}

	}

	delete[] p_1D;
	p_1D = NULL;
	return 1;
}

Header header_8_bits(Header header, DIB dib)
{
	static Header n_header = header;
	int width = dib._width;
	int height = dib._height;
	int bytes = (int)(dib._bits_per_pixel) / 8;
	int pad = (4 - width % 4) % 4;
	int n = width + pad;
	int size = n * height;
	int file_size = 54 + 1024 + size;
	// 54: sizeof(Header) + sizeof(DIB), 1024: khi so sánh với các hình 8 bits bmp thì đều có phần mô tả color table từ vị trí byte 54.
	n_header._size = file_size;
	n_header._offset = 54 + 1024; // lý do tương tự.
	return n_header;
}

DIB dib_8_bits(DIB dib)
{
	static DIB n_dib = dib;
	int width = dib._width;
	int height = dib._height;
	int pad = (4 - width % 4) % 4;
	int n = width + pad;
	int size = n * height;

	n_dib._size = 40;
	n_dib._bits_per_pixel = (unsigned short int)(8);
	n_dib._compression = 0;
	n_dib._image_size = size;
	n_dib._x_pixels_per_m = 0;
	n_dib._y_pixels_per_m = 0;
	n_dib._colors_used = 256;
	n_dib._important_colors = 256;
	return n_dib;
}

int write_8_bits(char *filename_in, char *filename_out, Header header, DIB dib)
{
	Header n_header = header_8_bits(header, dib);
	DIB n_dib = dib_8_bits(dib);
	int width = dib._width;
	int height = dib._height;
	int pad = (4 - width % 4) % 4;
	unsigned char padding[3] = {0, 0, 0};
	unsigned char *pixels_data = NULL;
	int check = convert_8bits(filename_in, pixels_data, header, dib);
	if (check == 0)
	{
		cout << "Convert that bai." << endl;
		return 0;
	}

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
		// color_table[i] = (unsigned char)(i) * 256 / width;
	}
	l = 0;
	unsigned char **x = new unsigned char *[height];
	for (int i = 0; i < height; i++)
	{
		x[i] = new unsigned char[width];
		for (int j = 0; j < width; j++)
			x[i][j] = pixels_data[l++];
	}

	ofstream f(filename_out, ios::binary);

	f.write((char *)&n_header, sizeof(Header));
	f.write((char *)&n_dib, sizeof(DIB));
	f.write((char *)color_table, 1024);
	f.seekp(n_header._offset);
	for (int i = 0; i < height; i++)
	{
		f.write((char *)(x[i]), width);
		f.write((char *)padding, pad);
	}
	f.close();
	for (int i = 0; i < height; i++)
	{
		delete[] x[i];
		x[i] = NULL;
	}
	return 1;
}

int main(int argc, char *argv[])
{
	if (argc != 4 || strcmp(argv[1], "-conv") != 0)
	{
		cout << "Loi dong lenh." << endl;
		return 1;
	}
	Header header;
	DIB dib;
	int check = docFile(argv[2], header, dib);
	if (check == 0)
	{
		cout << "Khong co file can doc." << endl;
		return 1;
	}
	inThongTin(header, dib);
	check = write_8_bits(argv[2], argv[3], header, dib);
	if (check == 0)
	{
		cout << "Ghi 8 bits khong thanh cong." << endl;
		return 1;
	}
	cout << "Chuyen anh 24 bits thanh 8 bits thah cong." << endl;
	return 0;
}