/**
 * @file MeowKit.h
 * @brief Top-level application — owns hardware and launcher lifecycle
 */
#pragma once
#include "bsp/devices.h"
#include "app/launcher/launcher.h"
#include <memory>


class MeowKit
{
private:
    std::unique_ptr<DEVICES> _device;
    std::unique_ptr<Launcher> _launcher;

public:
    MeowKit() = default;
    ~MeowKit() = default;

    bool Setup();
    void Loop();
};