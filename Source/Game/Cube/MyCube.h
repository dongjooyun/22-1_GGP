#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"

class MyCube : public BaseCube
{
public:
    MyCube(const MyCube& other) = delete;
    MyCube(MyCube&& other) = delete;
    MyCube& operator=(const MyCube& other) = delete;
    MyCube& operator=(MyCube&& other) = delete;
    ~MyCube() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;
};