//ParticleSystem.h
#pragma once

#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm\glm.hpp>

#define DEFAULT_PARTICLE_NUMBER 100000
#define DEFAULT_PARTICLE_LIFESPAN 10000

template<typename ParticleType>
class ParticleSystem {
public:
	//��Ĭ�ϻ��Զ�������������������������ڽ��г�ʼ��
	ParticleSystem(GLuint particleNumber = DEFAULT_PARTICLE_NUMBER,
		GLuint particleLifespan = DEFAULT_PARTICLE_LIFESPAN);
	//����
	virtual ~ParticleSystem();
	//��Ⱦ����ͨ����Ⱦ����������Ⱦÿһ�����ڵ�����
	virtual void Render();
	//�������ӵ���Ϣ�����麯������ͬ������ϵͳ�϶��в�ͬ��ʵ��
	virtual void Update(GLfloat deltaTime) = 0;

protected:
	//��һ�����ӽ�����Ⱦ�ĺ��������麯������Render�������ã���ͬ������ϵͳҲ�в�ͬ��ʵ��
	virtual void RenderParticle(const ParticleType& p) = 0;
	//�������������ӣ���Update��������
	void CreateParticle(const ParticleType& p);
	void DestroyParticle(GLint index);

protected:
	//�ط������ķ��䵥Ԫ��������������������Ϣ��freelist��Ϣ
	union PoolAllocUnit {
		ParticleType particle;
		struct Link {
			GLint mark;//�ж��Ƿ�Ϊlink�ı�־����Ϊ1
			GLint nextIdx;//ʹ�á�ջ�������ݽṹ�洢freelist������ʹ�õ���������
		}link;
		PoolAllocUnit() {}
	};
	//�ط������ĵ�ַ
	PoolAllocUnit* mParticlePool;

	GLuint mParticleNumber;
	GLuint mParticleLifespan;
private:
	//��ʾһ����freelist�е����������������freelistջ��ջ��Ԫ�أ�
	GLint mFreeIndex;
};

template<typename ParticleType>
inline ParticleSystem<ParticleType>::ParticleSystem(GLuint particleNumber, GLuint particleLifespan)
	:mParticleNumber(particleNumber), mParticleLifespan(particleLifespan)
{
	//��ʼ��ʱ���������ӵ��������ж�̬�ڴ����
	mParticlePool = new PoolAllocUnit[mParticleNumber];
	//��ʼ��freelist
	memset(mParticlePool, 0, sizeof(PoolAllocUnit)*mParticleNumber);
	mFreeIndex = 0;
	for (GLint i = 0; i < mParticleNumber; ++i) {
		mParticlePool[i].link.mark = 1;
		mParticlePool[i].link.nextIdx = i + 1;
	}
	mParticlePool[mParticleNumber - 1].link.nextIdx = -1;//-1��ǵ�ǰfreelistֻʣ�����һ��Ԫ��
}

template<typename ParticleType>
inline ParticleSystem<ParticleType>::~ParticleSystem()
{
	//�ͷŶ�̬�ڴ�
	delete[] mParticlePool;
}

template<typename ParticleType>
void ParticleSystem<ParticleType>::Render()
{
	//��Ⱦÿһ�������ڡ�������
	for (GLint i = 0; i < mParticleNumber; ++i) {
		if (mParticlePool[i].link.mark != 1) {
			RenderParticle(mParticlePool[i].particle);
		}
	}
}

template<typename ParticleType>
void ParticleSystem<ParticleType>::CreateParticle(const ParticleType& particle)
{
	GLint index = mParticlePool[mFreeIndex].link.nextIdx;
	if (index == -1)return;//�����ǰ���������ѳ����趨�������������������ֱ�ӷ���
	mParticlePool[mFreeIndex].particle = particle;
	mFreeIndex = index;
}

template<typename ParticleType>
void ParticleSystem<ParticleType>::DestroyParticle(GLint index)
{
	if (index < 0 || index >= mParticleNumber)return;//�������Ϸ�
	if (mParticlePool[index].link.mark == 1)return;//��ǰ������freelist��
	mParticlePool[index].link.mark = 1;//��ǰ������ӵ�freelist
	mParticlePool[index].link.nextIdx = mFreeIndex;
	mFreeIndex = index;
}

#endif