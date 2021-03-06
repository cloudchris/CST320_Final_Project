
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <io.h>
#include "resource.h"

using namespace std;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
	XMFLOAT3 Norm;
};


class ConstantBuffer
{
public:
	ConstantBuffer()
	{
		info = XMFLOAT4(1, 1, 1, 1);
	}
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
	XMFLOAT4 info;
	XMFLOAT4 CameraPos;
};


//********************************************
//********************************************
class StopWatchMicro_
{
private:
	LARGE_INTEGER last, frequency;
public:
	StopWatchMicro_()
	{
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&last);

	}
	long double elapse_micro()
	{
		LARGE_INTEGER now, dif;
		QueryPerformanceCounter(&now);
		dif.QuadPart = now.QuadPart - last.QuadPart;
		long double fdiff = (long double)dif.QuadPart;
		fdiff /= (long double)frequency.QuadPart;
		return fdiff*1000000.;
	}
	long double elapse_milli()
	{
		elapse_micro() / 1000.;
	}
	void start()
	{
		QueryPerformanceCounter(&last);
	}
};
//**********************************
class billboard
{
	float x, y, z, r;
public:
	billboard()
	{
		x = y = z = r = 0;
		position = XMFLOAT3(x, y, z);
		scale = 1;
		attacking = false;
		refill = false;
		life = 1.0;
		transparency = 1;
	}
	void setPosition(float xin, float yin, float zin)
	{
		x = xin;
		y = yin;
		z = zin;
		r = 0;
		position = XMFLOAT3(xin, yin, zin);

	}

	void setUsed() {
		used = true;
	}
	XMFLOAT3 position; //obvious
	float angle;
	bool attacking, refill;
	bool used = false;
	float life;
	float scale;		//in case it can grow
	float transparency; //for later use

						//begining the login
						//Enemy should follow user when within a radius
	void enemyanimation(int px, int py, int pz, float elapsed_microseconds)
	{
		float distance = sqrt(pow(position.x - px, 2) + pow(position.z - pz, 2));


		float enemysize = 2;
		if (
			(px >= (position.x - enemysize) && px <= (position.x + enemysize)) &&
			(pz >= (position.z - enemysize) && pz <= (position.z + enemysize))
			)
		{
			attacking = true;
		}
		else {
			attacking = false;
		}

	}

	void ammodropanimation(int px, int py, int pz, int dx, int dy, int dz)
	{
		float drop = 1;
		if (
			(px >= (dx - drop) && px <= (dx + drop)) &&
			(pz >= (dz - drop) && pz <= (dz + drop))
			)
		{
			refill = true;
		}
		else {
			refill = false;
		}

	}

	XMMATRIX get_matrix(XMMATRIX &ViewMatrix)
	{

		XMMATRIX view, R, T, S;
		view = ViewMatrix;
		//eliminate camera translation:
		view._41 = view._42 = view._43 = 0.0;
		XMVECTOR det;
		R = XMMatrixInverse(&det, view);//inverse rotation
		T = XMMatrixTranslation(position.x, position.y, position.z);
		S = XMMatrixScaling(scale, scale, scale);
		return S*R*T;
	}

	XMMATRIX get_matrix_y(XMMATRIX &ViewMatrix) //enemy-type
	{
		XMMATRIX view, R, T, S;
		view = ViewMatrix;
		//eliminate camera translation:
		view._41 = view._42 = view._43 = 0.0;
		XMVECTOR det;
		R = XMMatrixInverse(&det, view);//inverse rotation
		T = XMMatrixTranslation(position.x, position.y, position.z);
		S = XMMatrixScaling(scale, scale, scale);
		return S*R*T;
	}
};


//*****************************************
class bitmap
{

public:
	BYTE *image;
	int array_size;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	bitmap()
	{
		image = NULL;
	}
	~bitmap()
	{
		if (image)
			delete[] image;
		array_size = 0;
	}
	bool read_image(char *filename)
	{
		ifstream bmpfile(filename, ios::in | ios::binary);
		if (!bmpfile.is_open()) return FALSE;	// Error opening file
		bmpfile.read((char*)&bmfh, sizeof(BITMAPFILEHEADER));
		bmpfile.read((char*)&bmih, sizeof(BITMAPINFOHEADER));
		bmpfile.seekg(bmfh.bfOffBits, ios::beg);
		//make the array
		if (image)delete[] image;
		int size = bmih.biWidth*bmih.biHeight * 3;
		image = new BYTE[size];//3 because red, green and blue, each one byte
		bmpfile.read((char*)image, size);
		array_size = size;
		bmpfile.close();
		check_save();
		return TRUE;
	}
	BYTE get_pixel(int x, int y, int color_offset) //color_offset = 0,1 or 2 for red, green and blue
	{
		int array_position = x * 3 + y* bmih.biWidth * 3 + color_offset;
		if (array_position >= array_size) return 0;
		if (array_position < 0) return 0;
		return image[array_position];
	}
	BYTE get_pixelBounded(int x, int y, int color_offset) //color_offset = 0,1 or 2 for red, green and blue
	{
		if (x <= 0 || x >= bmih.biWidth-1 || y <= 0 || y >= bmih.biHeight-1)
		{
			return 0;
		}
		int array_position = x * 3 + y* bmih.biWidth * 3 + color_offset;
		if (array_position >= array_size) return 0;
		if (array_position < 0) return 0;
		return image[array_position];
	}
	void check_save()
	{
		ofstream nbmpfile("newpic.bmp", ios::out | ios::binary);
		if (!nbmpfile.is_open()) return;
		nbmpfile.write((char*)&bmfh, sizeof(BITMAPFILEHEADER));
		nbmpfile.write((char*)&bmih, sizeof(BITMAPINFOHEADER));
		//offset:
		int rest = bmfh.bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER);
		if (rest > 0)
		{
			BYTE *r = new BYTE[rest];
			memset(r, 0, rest);
			nbmpfile.write((char*)&r, rest);
		}
		nbmpfile.write((char*)image, array_size);
		nbmpfile.close();

	}
};
////////////////////////////////////////////////////////////////////////////////
//lets assume a wall is 10/10 big!
#define FULLWALL 2
#define HALFWALL 1
class wall
{
public:
	XMFLOAT3 position;
	int texture_no;
	int rotation; //0,1,2,3,4,5 ... facing to z, x, -z, -x, y, -y
	wall()
	{
		texture_no = 0;
		rotation = 0;
		position = XMFLOAT3(0, 0, 0);
	}
	XMMATRIX get_matrix()
	{
		XMMATRIX R, T, T_offset;
		R = XMMatrixIdentity();
		T_offset = XMMatrixTranslation(0, 0, -HALFWALL);
		T = XMMatrixTranslation(position.x, position.y, position.z);
		switch (rotation)//0,1,2,3,4,5 ... facing to z, x, -z, -x, y, -y
		{
		default:
		case 0:	R = XMMatrixRotationY(XM_PI);		T_offset = XMMatrixTranslation(0, 0, HALFWALL); break;
		case 1: R = XMMatrixRotationY(XM_PIDIV2);	T_offset = XMMatrixTranslation(0, 0, HALFWALL); break;
		case 2:										T_offset = XMMatrixTranslation(0, 0, HALFWALL); break;
		case 3: R = XMMatrixRotationY(-XM_PIDIV2);	T_offset = XMMatrixTranslation(0, 0, HALFWALL); break;
		case 4: R = XMMatrixRotationX(XM_PIDIV2);	T_offset = XMMatrixTranslation(0, 0, -HALFWALL); break;
		case 5: R = XMMatrixRotationX(-XM_PIDIV2);	T_offset = XMMatrixTranslation(0, 0, -HALFWALL); break;
		}
		return T_offset * R * T;
	}
};
//********************************************************************************************
class level
{
private:
	bitmap leveldata;
	vector<wall*> walls;						//all wall positions
	vector<ID3D11ShaderResourceView*> textures;	//all wall textures
	void process_level()
	{
		//we have to get the level to the middle:
		int x_offset = (leveldata.bmih.biWidth / 2)*FULLWALL;

		//lets go over each pixel without the borders!, only the inner ones

		for (int yy = 1; yy < (leveldata.bmih.biHeight - 1); yy++)
			for (int xx = 1; xx < (leveldata.bmih.biWidth - 1); xx++)
			{
				//wall information is the interface between pixels:
				//blue to something not blue: wall. texture number = 255 - blue
				//green only: floor. texture number = 255 - green
				//red only: ceiling. texture number = 255 - red
				//green and red: floor and ceiling ............

				BYTE red, green, blue, initB, initG, initR;

				blue = leveldata.get_pixel(xx, yy, 0);
				green = leveldata.get_pixel(xx, yy, 1);
				red = leveldata.get_pixel(xx, yy, 2);

				initB = leveldata.get_pixel(0, 39, 0);
				initG = leveldata.get_pixel(0, 39, 1);
				initR = leveldata.get_pixel(0, 39, 2);

				BYTE left_red = leveldata.get_pixel(xx - 1, yy, 2);
				BYTE left_green = leveldata.get_pixel(xx - 1, yy, 1);
				BYTE right_red = leveldata.get_pixel(xx + 1, yy, 2);
				BYTE right_green = leveldata.get_pixel(xx + 1, yy, 1);
				BYTE top_red = leveldata.get_pixel(xx, yy + 1, 2);
				BYTE top_green = leveldata.get_pixel(xx, yy + 1, 1);
				BYTE bottom_red = leveldata.get_pixel(xx, yy - 1, 2);
				BYTE bottom_green = leveldata.get_pixel(xx, yy - 1, 1);

				if (initR == 255 && initB == 255) { // Top
					int texno = 0;
					init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 76 - 2, yy*FULLWALL), 4, texno);
				}

				else if (initR == 255 && initG == 255) { // Left
					int texno = 2;
					init_wall(XMFLOAT3(FULLWALL - x_offset - 2, xx * 2 - 2, yy*FULLWALL), 1, texno);
				}
				else if (initR == 100 && initG == 100 && initB == 100) { // Back
					int texno = 3;
					init_wall(XMFLOAT3(xx*FULLWALL - x_offset, yy * 2 - 2, FULLWALL - 2), 2, texno);
				}
				else if (initB == 255) { // Right. FIX THIS
					int texno = 2;
					init_wall(XMFLOAT3(FULLWALL - x_offset + 76, xx * 2 - 2, yy*FULLWALL), 3, texno);


					if (red == 123 && green == 123) {
						int texno = 3;
						init_wall(XMFLOAT3(FULLWALL - x_offset + 76 - 2, xx * 2, yy*FULLWALL), 5, texno);


						if (left_red > 0 || left_green > 0) {
							init_wall(XMFLOAT3(FULLWALL - x_offset + 76 - 2, xx * 2 - 2, yy*FULLWALL), 3, texno);
							init_wall(XMFLOAT3(FULLWALL - x_offset + 76 - 2, xx * 2 - 2, yy*FULLWALL), 2, texno);
						}
						if (right_red > 0 || right_green > 0)// 
							init_wall(XMFLOAT3(FULLWALL - x_offset + 76 - 2, xx * 2, yy*FULLWALL), 0, texno);
						if (top_red > 0 || top_green > 0)// 
							init_wall(XMFLOAT3(FULLWALL - x_offset + 76 - 2, xx * 2 - 4, yy*FULLWALL), 4, texno);
						if (bottom_red > 0 || bottom_green > 0) {
							init_wall(XMFLOAT3(FULLWALL - x_offset + 76 - 2, xx * 2 - 2, yy*FULLWALL - 2), 3, texno);
							init_wall(XMFLOAT3(FULLWALL - x_offset + 76 - 2, xx * 2 - 2, yy*FULLWALL), 2, texno);
						}
					}
				}
				else if (initG == 255) { // Front
					int texno = 3;
					init_wall(XMFLOAT3(xx*FULLWALL - x_offset, yy * 2 - 2, FULLWALL + 76), 0, texno);


					if (red == 123 && green == 123) {
						int texno = 2;
						init_wall(XMFLOAT3(xx*FULLWALL - x_offset, yy * 2, FULLWALL + 74), 5, texno);

						if (left_red > 0 || left_green > 0)// left
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, yy * 2 - 2, FULLWALL + 74), 3, texno);
						if (right_red > 0 || right_green > 0)// bot
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, yy * 2 - 4, FULLWALL + 74), 4, texno);
						if (top_red > 0 || top_green > 0)// right
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, yy * 2 - 2, FULLWALL + 74), 1, texno);
						if (bottom_red > 0 || bottom_green > 0)// front
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, yy * 2 - 2, FULLWALL + 74), 0, texno);
					}

				}
				else if (initR == 255) { // Bottom
					int texno = 0;
					init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 5, texno);

					/*
					if (blue == 123)//wall possible
					{
					int texno = 255 - blue;
					BYTE left_red = leveldata.get_pixel(xx - 1, yy, 2);
					BYTE left_green = leveldata.get_pixel(xx - 1, yy, 1);
					BYTE right_red = leveldata.get_pixel(xx + 1, yy, 2);
					BYTE right_green = leveldata.get_pixel(xx + 1, yy, 1);
					BYTE top_red = leveldata.get_pixel(xx, yy + 1, 2);
					BYTE top_green = leveldata.get_pixel(xx, yy + 1, 1);
					BYTE bottom_red = leveldata.get_pixel(xx, yy - 1, 2);
					BYTE bottom_green = leveldata.get_pixel(xx, yy - 1, 1);

					if (left_red > 0 || left_green > 0)//to the left
					init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 3, texno);
					if (right_red > 0 || right_green > 0)//to the right
					init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 1, texno);
					if (top_red > 0 || top_green > 0)//to the top
					init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 2, texno);
					if (bottom_red > 0 || bottom_green > 0)//to the bottom
					init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 0, texno);
					}
					*/
					if (red == 123 && green == 123) {
						int texno = 2;

						init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 2, yy*FULLWALL), 5, texno);


						if (left_red > 0 || left_green > 0)//to the left
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 3, texno);
						if (right_red > 0 || right_green > 0)//to the right
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 1, texno);
						if (top_red > 0 || top_green > 0)//to the top
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 2, texno);
						if (bottom_red > 0 || bottom_green > 0)//to the bottom
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 0, texno);
					}
					/*
					else if (red == 123)//ceiling
					{
					int texno = 2;
					init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 5, texno);
					}
					else if (green == 123)//floor
					{
					int texno = 2;
					init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 4, texno);
					}
					*/
				}
			}
	}

	void init_wall(XMFLOAT3 pos, int rotation, int texture_no)
	{
		wall *w = new wall;
		walls.push_back(w);
		w->position = pos;
		w->rotation = rotation;
		w->texture_no = texture_no;
	}
public:
	bitmap *get_bitmap()//get method *NEW*
	{
		return &leveldata;
	}
	level()
	{
	}
	void init(char *level_bitmap)
	{
		if (!leveldata.read_image(level_bitmap))return;
		process_level();
	}
	bool init_texture(ID3D11Device* pd3dDevice, LPCWSTR filename)
	{
		// Load the Texture
		ID3D11ShaderResourceView *texture;
		HRESULT hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, filename, NULL, NULL, &texture, NULL);
		if (FAILED(hr))
			return FALSE;
		textures.push_back(texture);
		return TRUE;
	}
	ID3D11ShaderResourceView *get_texture(int no)
	{
		if (no < 0 || no >= textures.size()) return NULL;
		return textures[no];
	}
	XMMATRIX get_wall_matrix(int no)
	{
		if (no < 0 || no >= walls.size()) return XMMatrixIdentity();
		return walls[no]->get_matrix();
	}
	int get_wall_count()
	{
		return walls.size();
	}
	void render_level(ID3D11DeviceContext* ImmediateContext, ID3D11Buffer *vertexbuffer_wall, XMMATRIX *view, XMMATRIX *projection, ID3D11Buffer* dx_cbuffer)
	{
		//set up everything for the waqlls/floors/ceilings:
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		ImmediateContext->IASetVertexBuffers(0, 1, &vertexbuffer_wall, &stride, &offset);
		ConstantBuffer constantbuffer;
		constantbuffer.View = XMMatrixTranspose(*view);
		constantbuffer.Projection = XMMatrixTranspose(*projection);
		XMMATRIX wall_matrix, S;
		ID3D11ShaderResourceView* tex;
		//S = XMMatrixScaling(FULLWALL, FULLWALL, FULLWALL);
		S = XMMatrixScaling(1, 1, 1);
		for (int ii = 0; ii < walls.size(); ii++)
		{
			wall_matrix = walls[ii]->get_matrix();
			int texno = walls[ii]->texture_no;
			if (texno >= textures.size())
				texno = 0;
			tex = textures[texno];
			wall_matrix = wall_matrix;// *S;

			constantbuffer.World = XMMatrixTranspose(wall_matrix);

			ImmediateContext->UpdateSubresource(dx_cbuffer, 0, NULL, &constantbuffer, 0, 0);
			ImmediateContext->VSSetConstantBuffers(0, 1, &dx_cbuffer);
			ImmediateContext->PSSetConstantBuffers(0, 1, &dx_cbuffer);
			ImmediateContext->PSSetShaderResources(0, 1, &tex);
			ImmediateContext->Draw(6, 0);
		}
	}
};



class camera
{
private:

public:
	int w, s, a, d, q, e, sprinting;
	XMFLOAT3 position, possible_position;
	XMFLOAT3 rotation;
	camera()
	{
		w = s = a = d = sprinting = 0;
		position = position = XMFLOAT3(0, 0, 0);
	}
	void animation(float elapsed_microseconds, bitmap *leveldata)
	{
		XMMATRIX Ry, Rx, T;
		Ry = XMMatrixRotationY(-rotation.y);
		Rx = XMMatrixRotationX(-rotation.x);

		XMFLOAT3 forward = XMFLOAT3(0, 0, 1);
		XMVECTOR f = XMLoadFloat3(&forward);
		f = XMVector3TransformCoord(f, Rx*Ry);
		XMStoreFloat3(&forward, f);
		XMFLOAT3 side = XMFLOAT3(1, 0, 0);
		XMVECTOR si = XMLoadFloat3(&side);
		si = XMVector3TransformCoord(si, Rx*Ry);
		XMStoreFloat3(&side, si);

		float speed = elapsed_microseconds / 100000.0;
		possible_position = position;

		if (w)
		{
			possible_position.x -= forward.x * speed;
			possible_position.z -= forward.z * speed;
			//possible_position.y -= forward.y * speed;
		}
		if (sprinting)
		{
			speed = speed * 2;
		}
		if (s)
		{
			possible_position.x += forward.x * speed;
			possible_position.z += forward.z * speed;
		}
		if (d)
		{
			possible_position.x -= side.x * speed;
			possible_position.z -= side.z * speed;
		}
		if (a)
		{
			possible_position.x += side.x * speed;
			possible_position.z += side.z * speed;
		}

		/*
		leveldata->get_pixel(x, z, 0); // scale and translate from camPos to bitmapPos first. bitap thinks (0, 0) is bottom left pixel, camera thinks its bottom middle?

		*/
		//*
		BYTE red, green, blue;
		float x = 0;
		float z = 0;

		x = (possible_position.x * -0.5) + 20.5;
		z = (possible_position.z * -0.5) + 0.5;


		blue = leveldata->get_pixel((int)x, (int)z, 0);
		green = leveldata->get_pixel((int)x, (int)z, 1);
		red = leveldata->get_pixelBounded((int)x, (int)z, 2);


		//if (position.x < 39 && position.x > -37) {
		if (red == 255) {
			position = possible_position;
		}
		else {

		}

		//*/


	}
	XMMATRIX get_matrix(XMMATRIX *view)
	{
		XMMATRIX Rx, Ry, T;
		Rx = XMMatrixRotationX(rotation.x);
		Ry = XMMatrixRotationY(rotation.y);
		T = XMMatrixTranslation(position.x, position.y, position.z);
		return T*(*view)*Ry*Rx;
	}
};






class bullet
{
public:
	XMFLOAT3 pos, imp;
	bullet()
	{
		pos = imp = XMFLOAT3(0, 0, 0);
	}
	XMMATRIX getmatrix(float elapsed, XMMATRIX &view)
	{

		pos.x = pos.x + imp.x *(elapsed / 100000.0);
		pos.y = pos.y + imp.y *(elapsed / 100000.0);
		pos.z = pos.z + imp.z *(elapsed / 100000.0);

		XMMATRIX R, T;
		R = view;
		R._41 = R._42 = R._43 = 0.0;
		XMVECTOR det;
		R = XMMatrixInverse(&det, R);
		T = XMMatrixTranslation(pos.x, pos.y, pos.z);

		return R * T;
	}
};



float Vec3Length(const XMFLOAT3 &v);
float Vec3Dot(XMFLOAT3 a, XMFLOAT3 b);
XMFLOAT3 Vec3Cross(XMFLOAT3 a, XMFLOAT3 b);
XMFLOAT3 Vec3Normalize(const  XMFLOAT3 &a);
XMFLOAT3 operator+(const XMFLOAT3 lhs, const XMFLOAT3 rhs);
XMFLOAT3 operator-(const XMFLOAT3 lhs, const XMFLOAT3 rhs);
bool Load3DS(char *filename, ID3D11Device* g_pd3dDevice, ID3D11Buffer **ppVertexBuffer, int *vertex_count);

