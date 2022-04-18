#include "Cube/MyCube.h"

MyCube::MyCube(const std::filesystem::path& textureFilePath)
	: BaseCube(textureFilePath)
{}

void MyCube::Update(_In_ FLOAT deltaTime)
{
	m_world = XMMatrixTranslation(0.0f, XMScalarSin(deltaTime * 6.0f), 2.0f) * XMMatrixRotationY(deltaTime * 2.0f);
}