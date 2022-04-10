#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class CenterCube : public BaseCube
{
public:
	CenterCube() = default;
	~CenterCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;
};