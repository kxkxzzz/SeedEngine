// BasicApplication.cpp
#include <iostream>
#include "BasicApplication.hpp"

int kx::BasicApplication::Initialize() {
    m_quitting = false;

    return 0;
}

void kx::BasicApplication::Shutdown() {
}

void kx::BasicApplication::Tick() {
    std::cout << "BasicTick" << std::endl;
}

bool kx::BasicApplication::IsQuitting() {
    return m_quitting;
}
