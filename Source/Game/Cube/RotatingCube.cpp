#include "Cube/RotatingCube.h"

RotatingCube::RotatingCube(const XMFLOAT4& outputColor)
    : BaseCube(outputColor)
{}

void RotatingCube::Update(_In_ FLOAT deltaTime)
{
    m_world *= XMMatrixRotationY(-2.0f * deltaTime);
}