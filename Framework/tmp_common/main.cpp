#include <iostream>
#include "IApplication.hpp"

namespace kx {
extern IApplication* g_pApp;
}
using namespace kx;

int main(int argc, char** argv) {
    int ret;

    if ((ret = kx::g_pApp->Initialize()) != 0) {
        std::cout << "Application Initialize failed: " << ret << std::endl;
        return ret;
    }

    while (!kx::g_pApp->IsQuitting()) {
        kx::g_pApp->Tick();
    }

    kx::g_pApp->Shutdown();

    return 0;
}