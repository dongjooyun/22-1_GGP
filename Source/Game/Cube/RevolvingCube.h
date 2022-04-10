#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class RevolvingCube : public BaseCube
{
public:
	RevolvingCube() = default;
	~RevolvingCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;
};