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

void chuyenAnhThanh8Bits(unsigned char *&pixels_data, Header &header, DIB &dib)
{
	int width = dib._width;
	int bytes = (int)(dib._bits_per_pixel) / 8;
	int n = width * bytes;
	int size = width * dib._height;

	unsigned char *x = new unsigned char[1024 + size];
	if (bytes != 1)
	{
		int l = 0;
		int step = bytes - 3;
		for (int i = 0; i < dib._height; i++)
			for (int j = 0; j < n; j += bytes)
				x[l++] = pixels_data[i * n + j + step] / (unsigned char)(3) + pixels_data[i * n + j + 1 + step] / (unsigned char)(3) + pixels_data[i * n + j + 2 + step] / (unsigned char)(3);
		delete[] pixels_data;
		pixels_data = x;
	}

	header._size = (width + (4 - width % 4) % 4) * dib._height + 54 + 1024;
	header._offset = 54 + 1024;

	dib._size = 40;
	dib._bits_per_pixel = 8;
	dib._image_size = (width + (4 - width % 4) % 4) * dib._height;
	dib._x_pixels_per_m = 0;
	dib._y_pixels_per_m = 0;
	dib._colors_used = 256;
	dib._image_size = 256;
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

void thuNhoAnh(unsigned char *&pixels_data, Header &header, DIB &dib, int s)
{
	int bytes = (int)(dib._bits_per_pixel) / 8;
	int m = dib._height / s;
	int temp_m = 0;
	int n = dib._width / s;
	int temp_n = 0;
	int du_m = dib._height % s;
	int du_n = dib._width % s;
	unsigned char temp = 0;
	if (du_m != 0)
		temp_m = 1;
	if (du_n != 0)
		temp_n = 1;
	unsigned char *x = new unsigned char[(m + temp_m) * (n + temp_n) * bytes];
	int u = 0;
	int nn = bytes * dib._width;
	if (bytes == 1)
	{
		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < n; j++)
			{
				temp = 0;
				for (int k = i * s; k < (i + 1) * s; k++)
				{
					for (int l = j * s; l < (j + 1) * s; l++)
					{
						temp += pixels_data[k * nn + l] / (unsigned char)(s * s);
					}
				}
				x[u++] = temp;
			}
			temp = 0;
			if (du_n != 0)
				for (int k = i * s; k < (i + 1) * s; k++)
					for (int l = n * s; l < n * s + du_n; l++)
						temp += pixels_data[k * nn + l] / (unsigned char)(s * du_n);
			x[u++] = temp;
		}
		if (du_m != 0)
		{
			for (int j = 0; j < n; j++)
			{
				temp = 0;
				for (int k = m * s; k < m * s + du_m; k++)
					for (int l = j * s; l < (j + 1) * s; l++)
						temp += pixels_data[k * nn + l] / (unsigned char)(du_m * s);
				x[u++] = temp;
			}
			temp = 0;
			if (du_n != 0)
				for (int k = m * s; k < m * s + du_m; k++)
					for (int l = n * s; l < n * s + du_n; l++)
						temp += pixels_data[k * nn + l] / (unsigned char)(du_m * du_n);
			x[u++] = temp;
		}
	}

	if (bytes == 3)
	{
		unsigned char red = 0, green = 0, blue = 0;
		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < n; j++)
			{
				blue = 0;
				green = 0;
				red = 0;
				for (int k = i * s; k < (i + 1) * s; k++)
				{
					for (int l = j * s * bytes; l < (j + 1) * s * bytes; l++)
					{
						if (l % bytes == 0)
							blue += pixels_data[k * nn + l] / (unsigned char)(s * s);
						if (l % bytes == 1)
							green += pixels_data[k * nn + l] / (unsigned char)(s * s);
						if (l % bytes == 2)
							red += pixels_data[k * nn + l] / (unsigned char)(s * s);
					}
				}
				x[u++] = blue;
				x[u++] = green;
				x[u++] = red;
			}
			blue = 0;
			green = 0;
			red = 0;
			if (du_n != 0)
			{
				for (int k = i * s; k < (i + 1) * s; k++)
				{
					for (int l = n * s * bytes; l < (n * s + du_n) * bytes; l++)
					{
						if (l % bytes == 0)
							blue += pixels_data[k * nn + l] / (unsigned char)(s * du_n);
						if (l % bytes == 1)
							green += pixels_data[k * nn + l] / (unsigned char)(s * du_n);
						if (l % bytes == 2)
							red += pixels_data[k * nn + l] / (unsigned char)(s * du_n);
					}
				}
				x[u++] = blue;
				x[u++] = green;
				x[u++] = red;
			}
		}
		if (du_m != 0)
		{
			for (int j = 0; j < n; j++)
			{
				blue = 0;
				green = 0;
				red = 0;
				for (int k = m * s; k < (m * s + du_m); k++)
					for (int l = j * s * bytes; l < (j + 1) * s * bytes; l++)
					{
						if (l % bytes == 0)
							blue += pixels_data[k * nn + l] / (unsigned char)(s * du_m);
						if (l % bytes == 1)
							green += pixels_data[k * nn + l] / (unsigned char)(s * du_m);
						if (l % bytes == 2)
							red += pixels_data[k * nn + l] / (unsigned char)(s * du_m);
					}
				x[u++] = blue;
				x[u++] = green;
				x[u++] = red;
			}
			blue = 0;
			green = 0;
			red = 0;
			if (du_n != 0)
			{
				for (int k = m * s; k < m * s + du_m; k++)
					for (int l = n * s; l < n * s + du_n; l++)
					{
						if (l % bytes == 0)
							blue += pixels_data[k * nn + l] / (unsigned char)(du_m * du_n);
						if (l % bytes == 1)
							green += pixels_data[k * nn + l] / (unsigned char)(du_m * du_n);
						if (l % bytes == 2)
							red += pixels_data[k * nn + l] / (unsigned char)(du_m * du_n);
					}
				x[u++] = blue;
				x[u++] = green;
				x[u++] = red;
			}
		}
	}

	if (bytes == 4)
	{
		unsigned char red = 0, green = 0, blue = 0, alpha = 0;
		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < n; j++)
			{
				alpha = 0;
				blue = 0;
				green = 0;
				red = 0;
				for (int k = i * s; k < (i + 1) * s; k++)
				{
					for (int l = j * s * bytes; l < (j + 1) * s * bytes; l++)
					{
						if (l % bytes == 0)
							alpha += pixels_data[k * nn + l] / (unsigned char)(s * s);
						if (l % bytes == 1)
							blue += pixels_data[k * nn + l] / (unsigned char)(s * s);
						if (l % bytes == 2)
							green += pixels_data[k * nn + l] / (unsigned char)(s * s);
						if (l % bytes == 3)
							red += pixels_data[k * nn + l] / (unsigned char)(s * s);
					}
				}
				x[u++] = alpha;
				x[u++] = blue;
				x[u++] = green;
				x[u++] = red;
			}
			alpha = 0;
			blue = 0;
			green = 0;
			red = 0;
			if (du_n != 0)
			{
				for (int k = i * s; k < (i + 1) * s; k++)
				{
					for (int l = n * s * bytes; l < (n * s + du_n) * bytes; l++)
					{
						if (l % bytes == 0)
							alpha += pixels_data[k * nn + l] / (unsigned char)(s * du_n);
						if (l % bytes == 1)
							blue += pixels_data[k * nn + l] / (unsigned char)(s * du_n);
						if (l % bytes == 2)
							green += pixels_data[k * nn + l] / (unsigned char)(s * du_n);
						if (l % bytes == 3)
							red += pixels_data[k * nn + l] / (unsigned char)(s * du_n);
					}
				}
				x[u++] = alpha;
				x[u++] = blue;
				x[u++] = green;
				x[u++] = red;
			}
		}
		if (du_m != 0)
		{
			for (int j = 0; j < n; j++)
			{
				alpha = 0;
				blue = 0;
				green = 0;
				red = 0;
				for (int k = m * s; k < (m * s + du_m); k++)
					for (int l = j * s * bytes; l < (j + 1) * s * bytes; l++)
					{
						if (l % bytes == 0)
							alpha += pixels_data[k * nn + l] / (unsigned char)(s * du_m);
						if (l % bytes == 1)
							blue += pixels_data[k * nn + l] / (unsigned char)(s * du_m);
						if (l % bytes == 2)
							green += pixels_data[k * nn + l] / (unsigned char)(s * du_m);
						if (l % bytes == 3)
							red += pixels_data[k * nn + l] / (unsigned char)(s * du_m);
					}
				x[u++] = alpha;
				x[u++] = blue;
				x[u++] = green;
				x[u++] = red;
			}
			alpha = 0;
			blue = 0;
			green = 0;
			red = 0;
			if (du_n != 0)
			{
				for (int k = m * s; k < m * s + du_m; k++)
					for (int l = n * s; l < n * s + du_n; l++)
					{
						if (l % bytes == 0)
							alpha += pixels_data[k * nn + l] / (unsigned char)(du_m * du_n);
						if (l % bytes == 1)
							blue += pixels_data[k * nn + l] / (unsigned char)(du_m * du_n);
						if (l % bytes == 2)
							green += pixels_data[k * nn + l] / (unsigned char)(du_m * du_n);
						if (l % bytes == 3)
							red += pixels_data[k * nn + l] / (unsigned char)(du_m * du_n);
					}
				x[u++] = alpha;
				x[u++] = blue;
				x[u++] = green;
				x[u++] = red;
			}
		}
	}
	dib._image_size = (m + temp_m) * ((n + temp_n) * bytes + (4 - ((n + temp_n) * bytes) % 4) % 4);
	header._size = header._offset + dib._image_size;
	dib._width = n + temp_n;
	dib._height = m + temp_m;
	delete[] pixels_data;
	pixels_data = x;
}

int ghiAnh(char *filename, unsigned char *pixels_data, unsigned char *temp, Header header, DIB dib)
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
		f.write((char *)temp, header._offset - 54);
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

int main(int argc, char *argv[])
{
	Header header;
	DIB dib;
	unsigned char *pixels_data = NULL;
	unsigned char *temp = NULL;
	if (argc == 4 && strcmp(argv[1], "-conv") == 0)
	{
		docFile(argv[2], header, dib);
		inThongTin(header, dib);
		cout << endl
			 << "Dang thuc hien chuc nang ghi lai thanh anh 8 bits." << endl;
		docDuLieu(argv[2], pixels_data, header, dib);
		chuyenAnhThanh8Bits(pixels_data, header, dib);
		ghiAnh(argv[3], pixels_data, temp, header, dib);
	}
	else if (argc == 5 && strcmp(argv[1], "-zoom") == 0)
	{
		docFile(argv[2], header, dib);
		inThongTin(header, dib);
		cout << endl
			 << "Dang thuc hien chuc nang thu nho anh." << endl;
		docDuLieu(argv[2], pixels_data, header, dib);
		if (dib._bits_per_pixel == 32)
			temp = docTempData(argv[2], header, dib);
		int s = atoi(argv[4]);
		// cout << 3 / s << endl;
		thuNhoAnh(pixels_data, header, dib, s);
		ghiAnh(argv[3], pixels_data, temp, header, dib);
	}
	return 0;
}