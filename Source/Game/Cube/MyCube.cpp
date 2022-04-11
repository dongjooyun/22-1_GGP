#include "Cube/MyCube.h"

void MyCube::Update(_In_ FLOAT deltaTime)
{
	XMMATRIX mSpin = XMMatrixRotationX(-deltaTime * 2.0f);
	XMMATRIX mOrbit = XMMatrixRotationZ(-deltaTime * 3.0f);
	XMMATRIX mTranslate = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
	XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

	m_world = mScale * mSpin * mTranslate * mOrbit;
}