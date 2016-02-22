

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "shader.h"

MyShader::MyShader(const char * vtxShdName, const char * frgShdName)
{
	load_shader(vtxShdName, frgShdName);
}

int MyShader::load_shader(const char * vtxShdName, const char * frgShdName)
{
	GLuint vtxShader;
	GLuint frgShader;
	GLuint prog;
	GLint linked;

	/* �V�F�[�_�I�u�W�F�N�g�̍쐬 */
	vtxShader = glCreateShader(GL_VERTEX_SHADER);
	frgShader = glCreateShader(GL_FRAGMENT_SHADER);

	/* �o�[�e�b�N�X�V�F�[�_�̃��[�h�ƃR���p�C�� */
	if(load_and_compile(vtxShader, vtxShdName) < 0)
	{
		return -1;
	}

	/* �t���O�����g�V�F�[�_�̃��[�h�ƃR���p�C�� */
	if(load_and_compile(frgShader, frgShdName) < 0)
	{
		return -1;
	}

	/* �v���O�����I�u�W�F�N�g�̍쐬 */
	prog = glCreateProgram();

	/* �V�F�[�_�I�u�W�F�N�g�̃V�F�[�_�v���O�����ւ̓o�^ */
	glAttachShader(prog, vtxShader);
	glAttachShader(prog, frgShader);

	/* �V�F�[�_�I�u�W�F�N�g�̍폜 */
	glDeleteShader(vtxShader);
	glDeleteShader(frgShader);

	/* �V�F�[�_�v���O�����̃����N */
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	printProgramInfoLog(prog);
	if(linked == GL_FALSE)
	{
		fprintf(stderr, "Link error of %s & %s!!\n", vtxShdName, frgShdName);
		return -1;
	}

	lpProg = prog;

	return 0;
}

int MyShader::load_and_compile(GLuint shader, const char * name)
{
	errno_t err;
	FILE *fp;
	void *buf;
	int size;
	GLint compiled;

	if((err = fopen_s(&fp, name, "rb")) != 0)
	{
		fprintf(stderr, "%s is not found!!\n", name);
		return -1;
	}

	/* �t�@�C���T�C�Y�̎擾 */
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);

	/* �V�F�[�_�v���O�����̓ǂݍ��ݗ̈���m�� */
	if((buf = (void *)malloc(size)) == NULL)
	{
		fprintf(stderr, "Memory is not enough for %s\n", name);
		fclose(fp);
		return -1;
	}

	/* �t�@�C����擪����ǂݍ��� */
	fseek(fp, 0, SEEK_SET);
	fread(buf, 1, size, fp);

	/* �V�F�[�_�I�u�W�F�N�g�Ƀv���O�������Z�b�g */
	glShaderSource(shader, 1, (GLchar **)&buf, &size);

	/* �V�F�[�_�ǂݍ��ݗ̈�̉�� */
	free(buf);
	fclose(fp);

	/* �V�F�[�_�̃R���p�C�� */
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	printShaderInfoLog(shader);		/* �R���p�C�����O�̏o�� */
	if(compiled == GL_FALSE)
	{
		fprintf(stderr, "Compile error in %s!!\n", name);
		return -1;
	}

	return 0;
}

void MyShader::printProgramInfoLog(GLuint program)
{
	int logSize;
	int length;

	/* ���O�̒����́A�Ō��NULL�������܂� */
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);

	if(logSize > 1)
	{
		GLchar *infoLog = new GLchar(logSize);
		glGetProgramInfoLog(program, logSize, &length, infoLog);
		fprintf(stderr, "Program Info Log\n%s\n", infoLog);
		delete infoLog;
	}
}

void MyShader::printShaderInfoLog(GLuint shader)
{
	int logSize;
	int length;

	/* ���O�̒����́A�Ō��NULL�������܂� */
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

	if(logSize > 1)
	{
		GLchar *infoLog = new GLchar(logSize);
		glGetShaderInfoLog(shader, logSize, &length, infoLog);
		fprintf(stderr, "Shader Info Log\n%s\n", infoLog);
		delete infoLog;
	}
}
