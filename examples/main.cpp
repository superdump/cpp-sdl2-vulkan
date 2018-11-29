#include "project.h"

int main(int argc, char *argv[]) {
    auto project = project_init(1280, 720);
    project_loop(project);
    return 0;
}
