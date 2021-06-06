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

// cấu trúc cho một file ảnh bmp
struct BMP
{
	Header _header;
	DIB _dib;
	unsigned char *_dib_reserved = NULL;
	unsigned char *_pixels_data = NULL; // phần dữ liệu điểm ảnh
};

int readBMP(char *filename, BMP &bmp)
{
	ifstream f(filename, ios::binary);
	if (!f)
		return 0;
	f.seekg(0);
	f.read((char *)&bmp._header, sizeof(Header));
	// kiểm tra hai byte đầu của file có phải là "BM" hay không
	if (bmp._header._signature[0] != 'B' || bmp._header._signature[1] != 'M')
	{
		f.close();
		return 0;
	}
	// đọc phần dữ liệu DIB của file ảnh bmp (40 bytes tiếp theo sao Header)
	f.read((char *)&bmp._dib, sizeof(DIB));

	// kiểm tra ảnh có phải là ảnh 32 bits, và phần dib_size > 40 để đọc phần thông tin bổ sung của DIB
	if ((int)bmp._dib._bits_per_pixel == 32 && bmp._dib._size > 40)
	{
		int dibReservedSize = bmp._dib._size - 40;
		f.seekg(54);
		// đưa con trỏ file tới vị trí bắt đầu của phần dib reserved
		bmp._dib_reserved = new unsigned char[dibReservedSize];
		f.read((char *)bmp._dib_reserved, dibReservedSize);
	}

	f.seekg(bmp._header._offset);
	// đọc phần dữ liệu điểm ảnh
	// thao tác: chỉ đọc các byte thực, không đọc các file padding
	// đưa con trỏ file tới vị trí offset (vị trí bắt đầu pixels data)
	// số bytes trên mỗi pixel
	int bytes = (int)bmp._dib._bits_per_pixel / 8;
	// số bytes theo phương ngang
	int bytesInWidth = bmp._dib._width * bytes;
	// số lượng padding trên mỗi hàng
	int pad = (4 - bytesInWidth % 4) % 4;
	// kích thước thật của phần dữ liệu điểm ảnh, không bao gồm padding
	int size = bmp._dib._height * bytesInWidth;
	// khởi tạo mảng chứa các bytes dữ liệu
	bmp._pixels_data = new unsigned char[size];
	// thực hiện thao tác đọc phần dự liệu điểm ảnh
	for (int i = 0; i < bmp._dib._height; i++)
	{
		f.read((char *)(bmp._pixels_data + (i * bytesInWidth)), bytesInWidth);
		// đưa con trỏ file đến vị trí tiếp theo, bỏ qua padding.
		f.seekg((int)(f.tellg()) + pad);
	}

	// đóng file
	f.close();
	return 1;
}

void printInfoBMPToScreen(BMP bmp)
{
	cout << endl;
	cout << "Header __information__" << endl;
	cout << "Signature: " << bmp._header._signature[0] << bmp._header._signature[1] << endl;
	cout << "File size: " << bmp._header._size << endl;
	cout << "Reserve: " << bmp._header._reserved << endl;
	cout << "Offset: " << bmp._header._offset << endl;

	cout << endl;
	cout << "DIb __information__" << endl;
	cout << "DIB size: " << bmp._dib._size << endl;
	cout << "Width: " << bmp._dib._width << endl;
	cout << "Height: " << bmp._dib._height << endl;
	cout << "Planes: " << bmp._dib._planes << endl;
	cout << "Bits per pixel: " << bmp._dib._bits_per_pixel << endl;
	cout << "Compression: " << bmp._dib._compression << endl;
	cout << "Image size: " << bmp._dib._image_size << endl;
	cout << "X pixels per M: " << bmp._dib._x_pixels_per_m << endl;
	cout << "Y pixels per M: " << bmp._dib._y_pixels_per_m << endl;
	cout << "Color used: " << bmp._dib._colors_used << endl;
	cout << "Important colors: " << bmp._dib._important_colors << endl;
}

int writeBMP(char* filename, BMP bmp)
{
	ofstream f(filename, ios::binary);
	if (!f)
		return 0;
	
	// ghi phần header của file ảnh
	f.write((char*)&bmp._header, sizeof(Header));
	// ghi phần thông tin DIB của file ảnh
	f.write((char*)&bmp._dib, sizeof(DIB));
	
	// ghi phần dib reserved đối với ảnh 32 bits, để trình đọc ảnh có thể hiển thị ảnh lên màn hình.
	if ((int)bmp._dib._bits_per_pixel == 32 && bmp._dib._size > 40)
	{
		f.seekp(54);
		int dibReservedSize = bmp._dib._size - 40;
		f.write((char*)bmp._dib_reserved, dibReservedSize);
	}

	// ghi phần colors table đối với ảnh 8 bits, đẻ trình đọc ảnh có thể hiển thị ảnh lên màn hình.
	if ((int)bmp._dib._bits_per_pixel == 8)
	{
		f.seekp(54);
		// khởi tạo phần colors table
		unsigned char colors_table[1024];
		int l = 0;
		for (int i = 0; i < 256; i++)
		{
			for (int j = i * 4; j < (i + 1) * 4; j++)
				colors_table[j] = l;
			l++;
		}
		f.write((char*)colors_table, 1024);
	}	

	f.seekp(bmp._header._offset);
	// ghi phần dữ liệu điểm ảnh vào file
	// do phần dữ liệu điểm ảnh của cấu trúc kho đọc vào đã lọc padding nên khi ghi vào cần tính đến tham số này

	int bytes = (int)bmp._dib._bits_per_pixel / 8;
	// số byte trên phương ngang
	int bytesInWidth = bmp._dib._width * bytes;
	int pad = (4 - bytesInWidth % 4) % 4;
	// do pad là một số bù của 4 nên giá trị của pad >= 0 và <= 3
	unsigned char padding[3] = {0, 0, 0};
	// // ghi phần dữ liệu điểm ảnh xen kẻ với chèn padding vào cuối mỗi hàng.
	for (int i = 0; i < bmp._dib._height; i++)
	{
		f.write((char*)(bmp._pixels_data + (i * bytesInWidth)), bytesInWidth);
		// ghi phần padding vào cuối mỗi hàng
		f.write((char*)padding, pad);
	}
	// đóng file
	f.close();
	return 1;
}

int convertTo8Bits(BMP srcImg, BMP& dstImg)
{
	// kiểm tra ảnh cần chuyển có phải là ảnh 24 hoặc 32 bits hay không.

	if (((int)srcImg._dib._bits_per_pixel != 24 && (int)srcImg._dib._bits_per_pixel != 32) || srcImg._pixels_data == NULL)
		return 0;
	// chuyển đổi phần dữ liệu điểm ảnh
	// số bytes trên mỗi pixel
	int bytes = (int)srcImg._dib._bits_per_pixel / 8;
	int bytesInWidth = srcImg._dib._width * bytes;
	// kích thước phần dữ liệu điểm của file ảnh 8 bits (không bao gồm padding trên mỗi cuối hàng)
	int size = srcImg._dib._width * srcImg._dib._height;
	
	// khởi tạo mảng chứa dữ liệu điểm ảnh mới
	unsigned char *destPixelsData = new unsigned char[size];
	
	// biến chạy của mảng destPixelsData
	int k = 0; 
	// bước nhảy của vị trí cần đọc 
	//(bytes == 3, step = 0 đọc tự 0, 1, 2, bytes == 4, step =  1đọc từ 1, 2 ,3)
	int step = bytes - 3; 
	for (int i = 0; i < srcImg._dib._height; i++)
		for (int j = 0; j < bytesInWidth; j += bytes)
			destPixelsData[k++] = srcImg._pixels_data[i * bytesInWidth + j + step] / (unsigned char)(3) + srcImg._pixels_data[i * bytesInWidth + j + 1 + step] / (unsigned char)(3) + srcImg._pixels_data[i * bytesInWidth + j + 2 + step] / (unsigned char)(3);
		
	// thay đổi thông tin của ảnh được chuyển
	dstImg = srcImg;
	delete[] dstImg._pixels_data;
	dstImg._pixels_data = destPixelsData;
	dstImg._header._size = (dstImg._dib._width + (4 - dstImg._dib._width % 4) % 4) * dstImg._dib._height + sizeof(Header) + sizeof(DIB) + 1024;
	// 1024 là kích thước của colors table
	dstImg._header._offset = sizeof(Header) + sizeof(DIB) + 1024;
	dstImg._dib._size = 40;
	dstImg._dib._bits_per_pixel = (unsigned short int)8;
	dstImg._dib._image_size = dstImg._header._size - 1078;
	// 1078 = sizeof(Header) + sizeof(DIB) + 1024
	dstImg._dib._x_pixels_per_m = 0;
	dstImg._dib._y_pixels_per_m = 0;
	dstImg._dib._colors_used = 256;
	return 1;
}

int zoom(BMP srcImg, BMP &dstImg, int s)
{
	int bytes = (int)srcImg._dib._bits_per_pixel / 8;
	if ((bytes != 1 && bytes != 3 && bytes != 4) || srcImg._pixels_data == NULL)
		return 0;
	
	// kích thước của ảnh mới, theo phương dọc (chưa bao gồm phần dư ở sau nếu có); tức là height / s có dư
	int m = srcImg._dib._height / s;
	// kích thước của ảnh mới, theo phương ngang (chưa bao gồm phần dư ở sau nếu có); tức là width / s có dư
	int n = srcImg._dib._width / s;
	// số bytes dư trên mỗi cột
	int du_m = srcImg._dib._height % s;
	// số bytes dư trên mỗi cột
	int du_n = srcImg._dib._width % s;
	// bytes bổ sung của m nếu du_m != 0;, mặc định = 0, nếu du_m == 0
	int temp_m = 0;
	if (du_m != 0)
		temp_m = 1;
	
	// bytes bổ sung của n nếu du_n != 0, mặc định = 0, nếu du_n == 0
	int temp_n = 0;
	if (du_n != 0)
		temp_n = 1;

	// mảng mới chứa dữ liệu điểm ảnh của ảnh cần chuyển, không bao gồm padding trên mỗi cuối hàng
	unsigned char* destPixelsData = new unsigned char [(m + temp_m) * (n + temp_n) * bytes];
	// biến chạy của mảng mới 
	int k = 0;
	// sô bytes theo phương ngang
	int bytesInWidth = bytes * srcImg._dib._width;
	// xét từng loại ảnh để thu nhả
	// nếu là ảnh 8 bits per pixel
	if (bytes == 1)
	{
		// do mỗi pixel chỉ chứ một byte chứa kênh màu xám
		unsigned char gray = 0;
		for (int i = 0; i < m; i++)
		{
			// xét trên từng hàng, mỗi block màu có kích thước s * s
			for (int j = 0; j < n; j++)
			{
				// reset biến trung bình màu sau khi tính mỗi block màu
				gray = 0;
				for (int k = i * s; k < (i + 1) * s; k++)
				{
					for (int l = j * s; l < (j + 1) * s; l++)
					{
						gray += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * s);
					}
				}
				// thêm tuần tự cac phần tử màu này vào mảng destPixelsData
				destPixelsData[k++] = gray;
			}
			// sau mỗi cuối hàng cần check xem có các phần tử dư khi chia width cho s
			gray = 0;
			if (du_n != 0)
			{
				for (int k = i * s; k < (i + 1) * s; k++)
					for (int l = n * s; l < n * s + du_n; l++)
						gray += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char) (s * du_n);
						// do phần dư cuối mỗi hàng là du_n nê tỉ lệ chia s * du_n để phù hơp với tỉ lệ thu nhỏ
				destPixelsData[k++] = gray;
			}
			// khi đi hết tuần tự các hàng, nếu phép chia height cho s có dư thì xét hàng cuối theo tỉ lệ s * du_m
		}
		if (du_m != 0)
		{
			for (int j = 0; j < n; j++)
			{
				gray = 0;
				for (int k = m * s; k < m * s + du_m; k++)
					for (int l = j * s; l < (j + 1) * s; l++)
						gray += srcImg._pixels_data[k * bytesInWidth + l];
				destPixelsData[k++] = gray;
			}
			gray = 0;
			// xét tiếp phần tử cuối của hàng
			if (du_n != 0)
			{
				for (int k = m * s; k < m * s + du_m; k++)
					for (int l = n * s; l < n * s + du_n; l++)
						gray += srcImg._pixels_data[k * bytesInWidth + l];
				destPixelsData[k++] = gray;
			}
		}

	}
	// tương tự xét cho ảnh 24/32 bits
	// đối với ảnh 24 thì 
	//vị trí kênh blue của mỗi block màu là vị trí trên mỗi hàng chó chỉ số chia hết cho bytes, (bytes = 3);
	// vị trí của kênh green của mỗi block màu là vị trí trên mỗi hàng có chỉ số chia cho bytes ,(bytes = 3) dư 1
	// vị trí của kênh red của mỗi block màu là vị trí trên mỗi hàng có chỉ số chia cho bytes ,(bytes = 3) dư 2
	if (bytes == 3)
	{
		unsigned char blue = 0, green = 0, red = 0;
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
							blue += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * s);
						if (l % bytes == 1)
							green += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * s);
						if (l % bytes == 2)
							red += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * s);
					}
				}
				destPixelsData[k++] = blue;
				destPixelsData[k++] = green;
				destPixelsData[k++] = red;
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
							blue += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_n);
						if (l % bytes == 1)
							green += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_n);
						if (l % bytes == 2)
							red += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_n);
					}
				}
				destPixelsData[k++] = blue;
				destPixelsData[k++] = green;
				destPixelsData[k++] = red;
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
							blue += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_m);
						if (l % bytes == 1)
							green += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_m);
						if (l % bytes == 2)
							red += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_m);
					}
				destPixelsData[k++] = blue;
				destPixelsData[k++] = green;
				destPixelsData[k++] = red;
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
							blue += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(du_m * du_n);
						if (l % bytes == 1)
							green += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(du_m * du_n);
						if (l % bytes == 2)
							red += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(du_m * du_n);
					}
				destPixelsData[k++] = blue;
				destPixelsData[k++] = green;
				destPixelsData[k++] = red;
			}
		}
	}
	// đối với ảnh 32 bits thì
	//vị trí kênh alpha của mỗi block màu là vị trí trên mỗi hàng chó chỉ số chia hết cho bytes, (bytes = 4);
	// vị trí của kênh blue của mỗi block màu là vị trí trên mỗi hàng có chỉ số chia cho bytes ,(bytes = 4) dư 1
	// vị trí của kênh green của mỗi block màu là vị trí trên mỗi hàng có chỉ số chia cho bytes ,(bytes = 4) dư 2
	// vị trí của kênh green của mỗi block màu là vị trí trên mỗi hàng có chỉ số chia cho bytes ,(bytes = 4) dư 3
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
							alpha += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * s);
						if (l % bytes == 1)
							blue += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * s);
						if (l % bytes == 2)
							green += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * s);
						if (l % bytes == 3)
							red += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * s);
					}
				}
				destPixelsData[k++] = alpha;
				destPixelsData[k++] = blue;
				destPixelsData[k++] = green;
				destPixelsData[k++] = red;
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
							alpha += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_n);
						if (l % bytes == 1)
							blue += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_n);
						if (l % bytes == 2)
							green += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_n);
						if (l % bytes == 3)
							red += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_n);
					}
				}
				destPixelsData[k++] = alpha;
				destPixelsData[k++] = blue;
				destPixelsData[k++] = green;
				destPixelsData[k++] = red;
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
							alpha += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_m);
						if (l % bytes == 1)
							blue += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_m);
						if (l % bytes == 2)
							green += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_m);
						if (l % bytes == 3)
							red += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(s * du_m);
					}
				destPixelsData[k++] = alpha;
				destPixelsData[k++] = blue;
				destPixelsData[k++] = green;
				destPixelsData[k++] = red;
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
							alpha += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(du_m * du_n);
						if (l % bytes == 1)
							blue += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(du_m * du_n);
						if (l % bytes == 2)
							green += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(du_m * du_n);
						if (l % bytes == 3)
							red += srcImg._pixels_data[k * bytesInWidth + l] / (unsigned char)(du_m * du_n);
					}
				destPixelsData[k++] = alpha;
				destPixelsData[k++] = blue;
				destPixelsData[k++] = green;
				destPixelsData[k++] = red;
			}
		}
	}
	// copy loạt thông tin mới của ảnh src sang ảnh dst
	dstImg = srcImg;
	// chỉnh sửa thông tin
	dstImg._dib._image_size = (m + temp_m) * ((n + temp_n) * bytes + (4 - ((n + temp_n) * bytes) % 4) % 4);
	dstImg._dib._width = n + temp_n;
	dstImg._dib._height = m + temp_m;
	if (dstImg._pixels_data != NULL)
		delete[] dstImg._pixels_data;
	dstImg._pixels_data = destPixelsData;
	dstImg._header._size = dstImg._dib._image_size + dstImg._header._offset;
	return 1;
}

int main(int argc, char* argv[])
{
	BMP srcImg;
	BMP dstImg;
	if (argc == 4 && strcmp(argv[1], "-conv") == 0)
	{
		int check_read = readBMP(argv[2], srcImg);
		if (check_read == 0)
		{
			cout << "Doc file khong thanh cong.";
			return 1;
		}
		cout << "Thong tin cua anh can chuyen thanh 8 bits." << endl;
		printInfoBMPToScreen(srcImg);
		int check_conv = convertTo8Bits(srcImg, dstImg);
		if (check_conv == 0)
		{
			cout << "Chuyen anh thanh 8 bits khong thanh cong." << endl;
			return 1;
		}
		cout << "Thong tin cua anh 8 bits moi." << endl;
		printInfoBMPToScreen(dstImg);
		int check_write = writeBMP(argv[3], dstImg);
		if (check_write == 0)
		{
			cout << "Ghi anh 8 bits khong thanh cong, loi tao file output" << endl;
			return 1;
		}
		cout << "Chuyen anh thanh 8 bits thanh cong." << endl;
		return 0;
	}
	else if (argc == 5 && strcmp(argv[1], "-zoom") == 0)
	{
		// chuyển tham số thứ 5 của dòng lệnh thành kiểu nguyên
		int s = atoi(argv[4]);
		int check_read = readBMP(argv[2], srcImg);
		if (check_read == 0)
		{
			cout << "Doc file khong thanh cong.";
			return 1;
		}
		cout << "Thong tin cua anh can chuyen thanh zoom." << endl;
		printInfoBMPToScreen(srcImg);
		int check_zoom = zoom(srcImg, dstImg, s);
		if (check_zoom == 0)
		{
			cout << "Thu nho anh khong thanh cong." << endl;
			return 1;
		}
		cout << "Thong tin cua anh thu nho moi." << endl;
		printInfoBMPToScreen(dstImg);
		int check_write = writeBMP(argv[3], dstImg);
		if (check_write == 0)
		{
			cout << "Ghi anh thu nho khong thanh cong, loi tao file output" << endl;
			return 1;
		}
		cout << "Thu nho anh thanh cong." << endl;
		return 0;
	}
	return 0;
}