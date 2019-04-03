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
	//用默认或自定义的粒子数量和粒子生命周期进行初始化
	ParticleSystem(GLuint particleNumber = DEFAULT_PARTICLE_NUMBER,
		GLuint particleLifespan = DEFAULT_PARTICLE_LIFESPAN);
	//析构
	virtual ~ParticleSystem();
	//渲染，普通的渲染函数就是渲染每一粒存在的粒子
	virtual void Render();
	//更新粒子的信息，纯虚函数，不同的粒子系统肯定有不同的实现
	virtual void Update(GLfloat deltaTime) = 0;

protected:
	//对一颗粒子进行渲染的函数，纯虚函数，由Render函数调用，不同的粒子系统也有不同的实现
	virtual void RenderParticle(const ParticleType& p) = 0;
	//创建和销毁粒子，由Update函数调用
	void CreateParticle(const ParticleType& p);
	void DestroyParticle(GLint index);

protected:
	//池分配器的分配单元，可以用来保存粒子信息或freelist信息
	union PoolAllocUnit {
		ParticleType particle;
		struct Link {
			GLint mark;//判断是否为link的标志，设为1
			GLint nextIdx;//使用“栈”的数据结构存储freelist，所以使用单向链表即可
		}link;
		PoolAllocUnit() {}
	};
	//池分配器的地址
	PoolAllocUnit* mParticlePool;

	GLuint mParticleNumber;
	GLuint mParticleLifespan;
private:
	//表示一个在freelist中的粒子数组的索引（freelist栈的栈顶元素）
	GLint mFreeIndex;
};

template<typename ParticleType>
inline ParticleSystem<ParticleType>::ParticleSystem(GLuint particleNumber, GLuint particleLifespan)
	:mParticleNumber(particleNumber), mParticleLifespan(particleLifespan)
{
	//初始化时，根据粒子的数量进行动态内存分配
	mParticlePool = new PoolAllocUnit[mParticleNumber];
	//初始化freelist
	memset(mParticlePool, 0, sizeof(PoolAllocUnit)*mParticleNumber);
	mFreeIndex = 0;
	for (GLint i = 0; i < mParticleNumber; ++i) {
		mParticlePool[i].link.mark = 1;
		mParticlePool[i].link.nextIdx = i + 1;
	}
	mParticlePool[mParticleNumber - 1].link.nextIdx = -1;//-1标记当前freelist只剩最后这一个元素
}

template<typename ParticleType>
inline ParticleSystem<ParticleType>::~ParticleSystem()
{
	//释放动态内存
	delete[] mParticlePool;
}

template<typename ParticleType>
void ParticleSystem<ParticleType>::Render()
{
	//渲染每一个“存在”的粒子
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
	if (index == -1)return;//如果当前粒子数量已超过设定的最大粒子数量，则函数直接返回
	mParticlePool[mFreeIndex].particle = particle;
	mFreeIndex = index;
}

template<typename ParticleType>
void ParticleSystem<ParticleType>::DestroyParticle(GLint index)
{
	if (index < 0 || index >= mParticleNumber)return;//索引不合法
	if (mParticlePool[index].link.mark == 1)return;//当前索引在freelist中
	mParticlePool[index].link.mark = 1;//当前索引添加到freelist
	mParticlePool[index].link.nextIdx = mFreeIndex;
	mFreeIndex = index;
}

#endif