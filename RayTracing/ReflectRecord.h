#pragma once

#include "Face.h"

//��¼�������
struct HalfReflectRecord
{
	const Face* face;
	Point dir, hitpoint;
	HalfReflectRecord(const Face* _face, const Point &_dir, const Point &_hitpoint) :
		face(_face), dir(_dir), hitpoint(_hitpoint)
	{}
	HalfReflectRecord() {}

	double queryInCos() const;
};

//��¼����������
class ReflectRecord
{
public:
	enum ReflectType
	{
		diffuse, specular, refractive, eye, light
	}type;
	const Face* face;
	Point indir, hitpoint, outdir;

	//����cos * ������� * �������͸���
	double randomProbability;

	//����cos * ���cos * BSDF
	Color luminiance;

private:
	//�������������
	void generateDiffuse();
	//���ո�����������������
	void makeDiffuse(const Point &_outdir);
	//���ո������������ӵ����
	void makeEye(const Point &_outdir);
	//���ո����������ɹ�Դ����
	void makeLight(const Point &_outdir);
	//���ɾ��淴��
	void generateSpecular();
	//��������
	void generateRefractive();


public:
	//������Ƭ����������ɷ���
	static ReflectRecord randomReflect(const Face* _face, const Point &_indir, const Point &_hitPoint);
	
	//�������䷽��
	//��������ʧ�� ʧ��ʱrandomProbability��Ϊ����
	static ReflectRecord adjustReflect(const Face* _face, const Point &_indir, const Point &_hitPoint, const ReflectRecord &tar);

	//���������
	static ReflectRecord randomDiffuse(const Face* _face, const Point &_indir, const Point &_hitPoint);
	//����������
	void adjustDiffuse(const Point &_outdir);

	//ͨ�����������ɳ����HalfReflect
	HalfReflectRecord makeHalfOut() const;

	//double queryOutCos() const;
	//��ѯ�����cos
	double queryInCos() const;

	//��ת
	void reverse();
};