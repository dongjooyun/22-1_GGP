#include "Cube/MyCube.h"

MyCube::MyCube(const std::filesystem::path& textureFilePath)
	: BaseCube(textureFilePath)
{}

void MyCube::Update(_In_ FLOAT deltaTime)
{
	/*XMMATRIX mSpin = XMMatrixRotationX(-deltaTime * 2.0f);
	XMMATRIX mOrbit = XMMatrixRotationZ(-deltaTime * 3.0f);
	XMMATRIX mTranslate = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
	XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

	m_world = mScale * mSpin * mTranslate * mOrbit;*/

	//static FLOAT s_totalTime = 0.0f;
	//s_totalTime += deltaTime;

	m_world = XMMatrixTranslation(0.0f, XMScalarSin(deltaTime * 3.0f), 3.0f) * XMMatrixRotationY(deltaTime);
}