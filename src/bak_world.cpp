
//#include <opencv/cv.h>
//#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <GL/glut.h>
#include "world.h"
#include "teximage.h"
#include "fish.h"
#include "shark.h"
#include "target.h"
#include "shader.h"
#include "vec3d.h"


//X�EY�����ɂǂꂾ���������邩
const int PARTITION_X = 10;
const int PARTITION_Y = 10;

//���ʂ̉𑜓x
const int SURF_RESOLUTION_X = 100; //���_��
const int SURF_RESOLUTION_Y = 100; //���_��
								   //�o�b�t�@�[�T�C�Y
const int BUF_SIZE = 1024;


World::World(int w, int h) : width(w), height(h)
{
	partitons.resize(PARTITION_X*PARTITION_Y);
	for(int i = 0; i < 200; ++i)
	{
		//�ʒu�̓����_���A�������x�̓[��
		Fish* fish = new Fish(vec2d(100 + (double)rand() / RAND_MAX * 400,
			100 + (double)rand() / RAND_MAX * 400),
			vec2d());
		fishes.push_back(fish);

		//�ǂ��̗̈�ɂ��邩���v�Z�E�X�V
		int idx = fish->get_pos().x / (int)(width / PARTITION_X)
			+ (int)(fish->get_pos().y / (int)(height / PARTITION_Y))*PARTITION_X;
		partitons.at(idx).insert(std::make_pair(fish->get_id(), fish));
		fish->set_pidx(idx);
	}

	//�O�Cshark��ǉ�
	sharks.push_back(new Shark(vec2d(100, 100), vec2d(0, 0)));
	sharks.push_back(new Shark(vec2d(250, 250), vec2d(0, 0)));
	sharks.push_back(new Shark(vec2d(450, 450), vec2d(0, 0)));

	//�e�X�g�^�[�Q�b�g
	targets.push_back(new Target(vec2d(320, 200)));

	//�o�b�t�@
	buffer_data = new unsigned char[BUF_SIZE*BUF_SIZE * 3];

	//���ʃf�[�^������
	for(int i : {0, 1, 2})
	{
		surf_vertex_z[i].resize(SURF_RESOLUTION_X + 2); //�_�~�[2��
		for(int j = 0; j < SURF_RESOLUTION_X + 2; ++j)
		{
			surf_vertex_z[i][j].resize(SURF_RESOLUTION_Y + 2, 0.0);
		}
	}
	//���������i�e�X�g�j
	int px = SURF_RESOLUTION_X / 4;
	int py = SURF_RESOLUTION_Y / 4;
	constexpr int N = 1;
	for(int i = px - N; i <= px + N; ++i)
	{
		for(int j = py - N; j <= py + N; ++j)
		{
			surf_vertex_z[0][i][j] = sin(3.141592 / (2 * N));
		}
	}
	//2�X�e�b�v���v�Z
	constexpr double dt = 0.016;
	constexpr double c = 10;
	const double dx = (double)width / (SURF_RESOLUTION_X - 1);
	const double dy = (double)height / (SURF_RESOLUTION_Y - 1);
	for(int nx = 1; nx < SURF_RESOLUTION_X; ++nx)
	{
		for(int ny = 1; ny < SURF_RESOLUTION_Y; ++ny)
		{
			double x = nx * dx;
			double y = ny * dy;
			surf_vertex_z[1][nx][ny] = 2 * surf_vertex_z[0][nx][ny]
				+ (c * c * dt * dt / dx / dx)
				* (surf_vertex_z[0][nx - 1][ny] - 2 * surf_vertex_z[0][nx][ny] + surf_vertex_z[0][nx + 1][ny])
				+ (c * c * dt * dt / dy / dy)
				* (surf_vertex_z[0][nx][ny - 1] - 2 * surf_vertex_z[0][nx][ny] + surf_vertex_z[0][nx][ny + 1]);
		}
	}
	for(int nx = 1; nx < SURF_RESOLUTION_X; ++nx)
	{
		for(int ny = 1; ny < SURF_RESOLUTION_Y; ++ny)
		{
			double x = nx * dx;
			double y = ny * dy;
			surf_vertex_z[2][nx][ny] = 2 * surf_vertex_z[1][nx][ny] - surf_vertex_z[0][nx][ny]
				+ (c * c * dt * dt / dx / dx)
				* (surf_vertex_z[1][nx - 1][ny] - 2 * surf_vertex_z[1][nx][ny] + surf_vertex_z[1][nx + 1][ny])
				+ (c * c * dt * dt / dy / dy)
				* (surf_vertex_z[1][nx][ny - 1] - 2 * surf_vertex_z[1][nx][ny] + surf_vertex_z[1][nx][ny + 1]);
		}
	}

	fresnel_shader = std::unique_ptr<MyShader>(new MyShader("simple.vs", "simple.fs"));
}

World::~World()
{
	for(int i = 0; i < fishes.size(); ++i)
	{
		Fish* fish = fishes.back();
		delete fish;
		fishes.pop_back();
	}
	for(int i = 0; i < sharks.size(); ++i)
	{
		Shark* shark = sharks.back();
		delete shark;
		sharks.pop_back();
	}
	for(int i = 0; i < targets.size(); ++i)
	{
		Target* target = targets.back();
		delete target;
		targets.pop_back();
	}

	delete buffer_data;
}

void World::update()
{
	for(auto& tar : targets)
	{
		tar->update(this);
	}
	for(int i = 0; i < fishes.size(); ++i)
	{
		//�efish���ǂ��̗̈�ɑ����邩���X�V
		Fish* fish = fishes.at(i);
		vec2d pos = fish->get_pos();
		int pre_idx = fish->get_pidx();
		int idx = pos.x / (int)(width / PARTITION_X)
			+ (int)(pos.y / (int)(height / PARTITION_Y))*PARTITION_X;
		if(pre_idx != idx)
		{
			partitons.at(pre_idx).erase(partitons.at(pre_idx).find(fish->get_id()));
			partitons.at(idx).insert(std::make_pair(fish->get_id(), fish));
			fish->set_pidx(idx);
		}

		fishes.at(i)->update(this);
	}
	for(int i = 0; i < sharks.size(); ++i)
	{
		int idx = sharks.at(i)->get_pos().x / (int)(width / PARTITION_X)
			+ (int)(sharks.at(i)->get_pos().y / (int)(height / PARTITION_Y))*PARTITION_X;
		sharks.at(i)->set_pidx(idx);

		sharks.at(i)->update(this);
	}

	//�g���������ɂ��v�Z
	/*  �����@�@
	(u[n+1] - 2u[n] + u[n-1])/(dt)^2 = c^2((ux-1[n] - 2u[n] + ux+1[n])/(dx)^2 + (uy-1[n] - 2u[n] + uy+1[n])/(dy)^2)
	*/
	constexpr double dt = 0.016;
	constexpr double c = 100;
	const double dx = (double)width / (SURF_RESOLUTION_X - 1);
	const double dy = (double)height / (SURF_RESOLUTION_Y - 1);
	//�X�e�b�v�X�V
	//	surf_vertex_z[0] = std::move(surf_vertex_z[1]);
	//	surf_vertex_z[1] = std::move(surf_vertex_z[2]);
	surf_vertex_z[0] = surf_vertex_z[1];
	surf_vertex_z[1] = surf_vertex_z[2];
	for(int nx = 1; nx <= SURF_RESOLUTION_X; ++nx)
	{
		for(int ny = 1; ny <= SURF_RESOLUTION_Y; ++ny)
		{
			surf_vertex_z[2][nx][ny] = 2 * surf_vertex_z[1][nx][ny] - surf_vertex_z[0][nx][ny]
				+ (c * c * dt * dt / dx / dx)
				* (surf_vertex_z[1][nx - 1][ny] - 2 * surf_vertex_z[1][nx][ny] + surf_vertex_z[1][nx + 1][ny])
				+ (c * c * dt * dt / dy / dy)
				* (surf_vertex_z[1][nx][ny - 1] - 2 * surf_vertex_z[1][nx][ny] + surf_vertex_z[1][nx][ny + 1]);
		}
	}
}


void World::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--���_��2D��--
	//�ˉe�s���ݒ�
	glMatrixMode(GL_PROJECTION);
	//�P�ʍs��ɏ�����
	glLoadIdentity();
	//��ʂ̍��W�����[���h���W�ɑΉ�
	glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
	//�r���[�|�[�g�ϊ�
	glViewport(0, 0, width, height);
	//--------------

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_ALPHA_TEST);

	for(auto& tar : targets)
	{
		tar->render(this);
	}
	for(int i = 0; i < fishes.size(); ++i)
	{
		fishes.at(i)->render(this);
	}
	for(int i = 0; i < sharks.size(); ++i)
	{
		sharks.at(i)->render(this);
	}

	glDisable(GL_ALPHA_TEST);

	//�o�b�t�@���ĕ`��
	glReadPixels(0, 0, BUF_SIZE, BUF_SIZE, GL_RGB, GL_UNSIGNED_BYTE, buffer_data);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//--���_��3D��---
	//�ˉe�s���ݒ�
	glMatrixMode(GL_PROJECTION);
	//�P�ʍs��ɏ�����
	glLoadIdentity();
	//���E�̐ݒ�
	gluPerspective(60.0, (double)width / height, 0.1, 1000.0);
	//���_
	///*
	gluLookAt(
		320.0, 0.0, 500.0, // ���_�̈ʒux,y,z;
		320.0, 320.0, 0.0,   // ���E�̒��S�ʒu�̎Q�Ɠ_���Wx,y,z
		0.0, 0.0, 1.0);  //���E�̏�����̃x�N�g��x,y,z
						 //*/
						 /*
						 gluLookAt(
						 100.0, -100.0, 50.0, // ���_�̈ʒux,y,z;
						 320.0, 320.0, 0.0,   // ���E�̒��S�ʒu�̎Q�Ɠ_���Wx,y,z
						 0.0, 0.0, 1.0);  //���E�̏�����̃x�N�g��x,y,z
						 */
						 //---------------
						 //�A�e��ON
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0); //����0�𗘗p

						 //�����̐ݒ�
	GLfloat tcolor[3] = {1.0, 1.0, 1.0}, scolor[3] = {0.1, 0.1, 0.1};
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tcolor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, scolor);
	//glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 30.0f);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BUF_SIZE, BUF_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer_data);
	glEnable(GL_TEXTURE_2D);
	//	glUseProgram(fresnel_shader->get_prog());
	const double dx = (double)width / (SURF_RESOLUTION_X - 1);
	const double dy = (double)height / (SURF_RESOLUTION_Y - 1);
	glBegin(GL_QUADS);
	for(int nx = 0; nx < SURF_RESOLUTION_X - 1; ++nx)
	{
		for(int ny = 0; ny < SURF_RESOLUTION_Y - 1; ++ny)
		{
			double x = nx * dx;
			double y = ny * dy;
			//�@���x�N�g���̐ݒ�
			auto set_norm = [&](int ix, int iy) {
				//				vec3d norm = cross_vec3d(
				//					vec3d(2 * dx, 2 * dy, surf_vertex_z[2][ix + 1][iy + 1] - surf_vertex_z[2][ix - 1][iy - 1]),
				//					vec3d(2 * dx, - 2 * dy, surf_vertex_z[2][ix + 1][iy - 1] - surf_vertex_z[2][ix - 1][iy + 1])).norm();
				//				glNormal3d(norm.x, norm.y, norm.z);
				vec3d norm = cross_vec3d(
					vec3d(1.0, 0.0, (surf_vertex_z[2][ix + 1][iy] - surf_vertex_z[2][ix - 1][iy]) / (2 * dx)),
					vec3d(0.0, 1.0, (surf_vertex_z[2][ix][iy + 1] - surf_vertex_z[2][ix][iy - 1]) / (2 * dy))).norm();
				glNormal3d(-norm.x, -norm.y, -norm.z);
			};

			set_norm(nx + 1, ny + 1);
			glTexCoord2d((x) / BUF_SIZE, (height - y) / BUF_SIZE);
			glVertex3d(x, y, surf_vertex_z[2][nx + 1][ny + 1]);

			set_norm(nx + 2, ny + 1);
			glTexCoord2d((x + dx) / BUF_SIZE, (height - y) / BUF_SIZE);
			glVertex3d(x + dx, y, surf_vertex_z[2][nx + 2][ny + 1]);

			set_norm(nx + 2, ny + 2);
			glTexCoord2d((x + dx) / BUF_SIZE, (height - y - dy) / BUF_SIZE);
			glVertex3d(x + dx, y + dy, surf_vertex_z[2][nx + 2][ny + 2]);

			set_norm(nx + 1, ny + 2);
			glTexCoord2d((x) / BUF_SIZE, (height - y - dy) / BUF_SIZE);
			glVertex3d(x, y + dy, surf_vertex_z[2][nx + 1][ny + 2]);
		}
	}
	glEnd();
	glUseProgram(0);
	glDisable(GL_TEXTURE_2D);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	glutSwapBuffers();
}


std::vector<Fish*> World::get_neighborfishes(int idx)
{
	//idx�̗̈�̎��ӂW�̗̈���܂߂āA������fish��Ԃ�
	std::vector<Fish*> nfishes;
	int x = idx%PARTITION_X;
	int y = idx / PARTITION_X;
	for(int i = x - 1; i <= x + 1; ++i)
	{
		for(int j = y - 1; j <= y + 1; ++j)
		{
			if(i < 0 || i >= PARTITION_X || j < 0 || j >= PARTITION_Y)
				continue;
			int nidx = i + j*PARTITION_X;
			for(std::map<int, Fish*>::iterator it = partitons.at(nidx).begin();
			it != partitons.at(nidx).end(); ++it)
			{
				nfishes.push_back(it->second);
			}
		}
	}
	return nfishes;
}
