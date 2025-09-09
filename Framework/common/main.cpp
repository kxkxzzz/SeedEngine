#include <stdio.h>
#include "IApplication.hpp"

namespace kx {
extern IApplication* g_pApp;
}
using namespace kx;

int main(int argc, char** argv) {
    if (kx::g_pApp == nullptr) {
        printf("Application instance is null!\n");
        return -1;
    }

    if (kx::g_pApp->Initialize() != 0) {
        printf("Failed to initialize application!\n");
        return -1;
    }

    while (!kx::g_pApp->IsQuitting()) {
        kx::g_pApp->Tick();
    }

    kx::g_pApp->Shutdown();

    return 0;
}